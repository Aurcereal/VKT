#include "mesh.h"
#include "tiny_gltf.h"
#include <iostream>
#include "tiny_obj_loader.h"
#include <glm/gtc/type_ptr.hpp>

void Mesh::CreateFromOBJFile(const VulkanReferences& ref, const std::string& path, bool isStorageBuffer) {
    vertices.clear();
    indices.clear();
    this->isStorageBuffer = isStorageBuffer;

    LoadOBJModel(path);
    CreateBuffers(ref);
}
void Mesh::CreateFromGLTFFile(const VulkanReferences& ref, const std::string& path, ShaderPipeline* pbrShader, const vector<ShaderParameter::MParameter>& pbrMParams, int textureParamsStartIndex, bool isStorageBuffer) {
    vertices.clear();
    indices.clear();
    this->isStorageBuffer = isStorageBuffer;

    LoadGLTFModel(ref, path);
    CreateBuffers(ref);

    vector newMParams = pbrMParams;
    if(baseColor) newMParams[textureParamsStartIndex + 0] = ShaderParameter::MParameter(ShaderParameter::USampler{ .texture = baseColor.get() });
    if(metallicRoughness) newMParams[textureParamsStartIndex + 1] = ShaderParameter::MParameter(ShaderParameter::USampler{ .texture = metallicRoughness.get() });
    if(aoTexture) newMParams[textureParamsStartIndex + 2] = ShaderParameter::MParameter(ShaderParameter::USampler{ .texture = aoTexture.get()});
    mat.Create(pbrShader, ref, newMParams);
}
void Mesh::CreateFromArrays(const VulkanReferences& ref, const vector<vec3>& positions, const vector<vec3>& colors, const vector<vec3>& normals, const vector<uint32_t>& indices, bool isStorageBuffer) {
    this->vertices.clear();
    this->indices = indices;
    this->indexCount = this->indices.size();
    this->isStorageBuffer = isStorageBuffer;

    for (int i = 0; i < positions.size(); ++i) {
        this->vertices.push_back(
            Vertex{
                .pos = positions[i],
                .color = colors[i],
                .uv = vec2(0),
                .norm = normals[i]
            }
        );
    }

    CreateBuffers(ref);
}

void Mesh::GetPositions(vector<vec3>* pPositions) const {   
    pPositions->clear();
    for (const auto& v : vertices) {
        pPositions->push_back(v.pos);
    }
}

const vector<uint32_t>& Mesh::GetIndices() const {
    return indices;
}

void Mesh::CreateBuffers(const VulkanReferences& ref) {
    assert(indexCount > 0);

    vk::DeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();

    WBuffer vertexStagingBuffer;
    vertexStagingBuffer.Create(ref, vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc, // Can be the SOURCE of a transfer
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    vertexStagingBuffer.MapMemory();
    memcpy(vertexStagingBuffer.mappedMemory, vertices.data(), vertexBufferSize);
    vertexStagingBuffer.UnmapMemory();

    vertexBuffer.Create(ref, vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst |
        (isStorageBuffer ? vk::BufferUsageFlagBits::eStorageBuffer : static_cast<vk::BufferUsageFlagBits>(0)),
        vk::MemoryPropertyFlagBits::eDeviceLocal // Device local, can't map memory directly
    );
    vertexBuffer.CopyFrom(ref, vertexStagingBuffer);


    vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    // std::cout << "\t" << indices.size() << std::endl;

    WBuffer indexStagingBuffer;
    indexStagingBuffer.Create(ref, indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    indexStagingBuffer.MapMemory();
    memcpy(indexStagingBuffer.mappedMemory, indices.data(), indexBufferSize);
    indexStagingBuffer.UnmapMemory();

    indexBuffer.Create(ref, indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer |
        (isStorageBuffer ? vk::BufferUsageFlagBits::eStorageBuffer : static_cast<vk::BufferUsageFlagBits>(0)),
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    indexBuffer.CopyFrom(ref, indexStagingBuffer);
}

void GetGLTFAttributes(const tinygltf::Model& model, const tinygltf::Primitive& prim, std::string attribName, vector<float>* pAttribs, int* pComponentCount, size_t* pAttribCount) {
    int accessorIndex = prim.attributes.at(attribName);

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[view.buffer];
    int numComponents = tinygltf::GetNumComponentsInType(accessor.type);

    const uint8_t* dataStartByte = buffer.data.data() + view.byteOffset + accessor.byteOffset;
    size_t stride = view.byteStride;
    if (stride == 0) {
        // Tightly packed
        stride = numComponents * sizeof(float);
    }

    pAttribs->resize(accessor.count * numComponents);
    for (int i = 0; i < accessor.count; i++) {
        const float* elem = reinterpret_cast<const float*>(dataStartByte + stride * i);

        for (int c = 0; c < numComponents; c++) {
            (*pAttribs)[i * numComponents + c] = elem[c];
        }
    }

    if(pComponentCount) *pComponentCount = numComponents;
    if(pAttribCount) *pAttribCount = accessor.count;
}

void GetGLTFIndices(const tinygltf::Model& model, const tinygltf::Primitive& prim, vector<uint32_t>* pIndices) {
    int accessorIndex = prim.indices;

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[view.buffer];

    const uint8_t* dataStartByte = buffer.data.data() + view.byteOffset + accessor.byteOffset;
    size_t stride = view.byteStride;
    if (stride == 0) {
        // Tightly packed
        stride =
            accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ? sizeof(uint8_t) :
            TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? sizeof(uint16_t) :
            sizeof(uint32_t);
    }

    pIndices->resize(accessor.count);
    for (int i = 0; i < accessor.count; i++) {
        const uint8_t* currByte = dataStartByte + stride * i;
        switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                (*pIndices)[i] = *currByte; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                (*pIndices)[i] = *reinterpret_cast<const uint16_t*>(currByte); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                (*pIndices)[i] = *reinterpret_cast<const uint32_t*>(currByte); break;
        }
    }
}

void Mesh::LoadGLTFModel(const VulkanReferences& ref, const std::string& path) {
    assert(vertices.size() == 0 && indices.size() == 0);

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
   
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cerr << "Warn: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Err: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to parse gltf of name " << path << " - remember gltf needs LoadASCII glb needs LoadBinary " << std::endl;
    }

    // Assume 1 for now
    const tinygltf::Primitive& prim = model.meshes[0].primitives[0];

    vector<float> positions;
    size_t positionCount = 0;
    if (prim.attributes.count("POSITION")) {
        int cc; 
        GetGLTFAttributes(model, prim, "POSITION", &positions, &cc, &positionCount);
        assert(cc == 3);
        assert(positionCount == positions.size() / 3);
    }
    vector<float> normals;
    if (prim.attributes.count("NORMAL"))
        GetGLTFAttributes(model, prim, "NORMAL", &normals, nullptr, nullptr);
    vector<float> uvs;
    if (prim.attributes.count("TEXCOORD_0"))
        GetGLTFAttributes(model, prim, "TEXCOORD_0", &uvs, nullptr, nullptr);

    assert(positions.size() / 3 == uvs.size() / 2);
    assert(uvs.size() / 2 == normals.size() / 3);

    for (size_t i = 0; i < positionCount; i++) {
        vertices.push_back(
            Vertex {
                .pos = vec3(
                    positions[i * 3 + 0],
                    positions[i * 3 + 1],
                    positions[i * 3 + 2]
                    ),
                .color = vec3(1.0f),
                .uv = vec2(
                    uvs[i * 2 + 0],
                    uvs[i * 2 + 1]
                    ),
                .norm = vec3(
                    normals[i * 3 + 0],
                    normals[i * 3 + 1],
                    normals[i * 3 + 2]
                )
            }
        );

        assert(prim.indices >= 0);
    }

    // Indices
    GetGLTFIndices(model, prim, &indices);

    // Materials
    const tinygltf::Material& mat = model.materials[prim.material];
    const auto& pbr = mat.pbrMetallicRoughness;
    
    baseColorMult = pbr.baseColorFactor.empty() ? vec4(1.0f) : vec4(glm::make_vec4(pbr.baseColorFactor.data()));
    
    if (pbr.baseColorTexture.index >= 0) {
        const auto& img = model.images[model.textures[pbr.baseColorTexture.index].source];
        baseColor = mkU<WTexture>();
        baseColor->CreateFromPixels(ref, img.image.data(), img.width, img.height, vk::Format::eR8G8B8A8Srgb);
    }
    if (pbr.metallicRoughnessTexture.index >= 0) {
        const auto& img = model.images[model.textures[pbr.metallicRoughnessTexture.index].source];
        metallicRoughness = mkU<WTexture>();
        metallicRoughness->CreateFromPixels(ref, img.image.data(), img.width, img.height, vk::Format::eR8G8B8A8Srgb);
    }
    if (mat.occlusionTexture.index >= 0) {
        const auto& img = model.images[model.textures[mat.occlusionTexture.index].source];
        aoTexture = mkU<WTexture>();
        aoTexture->CreateFromPixels(ref, img.image.data(), img.width, img.height, vk::Format::eR8G8B8A8Srgb);
    }

    indexCount = indices.size();
}

void Mesh::LoadOBJModel(const std::string& path) {
    // Shouldn't be loading when model already loaded
    assert(vertices.size() == 0 && indices.size() == 0);

    tinyobj::attrib_t attrib; // all vert attribs
    vector<tinyobj::shape_t> shapes; // all separate objects and their faces
    vector<tinyobj::material_t> materials; // material/texture per face
    string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str())) {
        throw std::runtime_error(err);
    }
    // LoadObj will auto triangulate n-gons into triangles by default

    // uniqueVerts[v] = Index of v in vertex buffer if exists
    std::unordered_map<Vertex, uint32_t> uniqueVerts;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {
                .pos = vec3(
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                ),
                .color = vec3(1.0f),
                .uv = vec2(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                ),
                .norm = vec3(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                ),
            };

            // Create vertex if doesn't exist
            if (uniqueVerts.count(vertex) == 0) {
                vertices.push_back(vertex);
                uniqueVerts[vertex] = vertices.size() - 1;
            }

            indices.push_back(uniqueVerts[vertex]);
        }
    }

    indexCount = indices.size();
}
