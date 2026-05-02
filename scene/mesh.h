#pragma once

#include "defines.h"
#include "scene/buffer.h"
#include "scene/texture.h"
#include "scene/shader-parameter.h"
#include "scene/pipeline.h"
#include "scene/material.h"

using namespace glm;
using namespace std;

struct Vertex {
    alignas(16) vec3 pos;
    alignas(16) vec3 color;
    alignas(8) vec2 uv;
    alignas(16) vec3 norm;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; // binding index, stride, load data per vertex
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        // Location, Binding Index
        return {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)),
            vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, norm))
        };
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && uv == other.uv && norm == other.norm;
    }
};

// Hash Function For Vertex
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.uv) << 1) ^
                (hash<glm::vec3>()(vertex.norm) << 2);
        }
    };
}

struct MultiPrimitivePBRInfo {
    vec4 baseColorMult;
    vector<WTexture> baseColorTexs;
    vector<WTexture> metallicRoughnessTexs;
    // normal
    vector<WTexture> aoTexs;
    vector<uint32_t> triToMaterialIndex;
    WBuffer triToMaterialIndexBuffer;
};

struct SinglePrimitivePBRInfo {
    vec4 baseColorMult;
    WTexture baseColorTex;
    WTexture metallicRoughnessTex;
    WTexture aoTex;
    Material mat;
    vector<WBuffer> uPbrInfo;
};

namespace tinygltf {
    class Model;
    class Primitive;
}

// Staging allows us to use high performance memory for loading vertex data
// In practice, not good to do a separate allocation for every object, better to do one big one and split it up (VulkanMemoryAllocator library)
// You should even go a step further, allocate a single vertex and index buffer for lots of things and use offsets to bindvertexbuffers to store lots of 3D objects
class Mesh {
public:
    void CreateFromOBJFile(const VulkanReferences&, const std::string& path, bool isStorageBuffer = false);
    void CreateFromGLTFFile(const VulkanReferences&, const std::string& path, bool isStorageBuffer = false);
    void CreateFromArrays(const VulkanReferences& ref, const vector<vec3>& positions, const vector<vec3>& colors, const vector<vec3>& normals, const vector<uint32_t>& indices, bool isStorageBuffer = false);

    static uPtr<vector<Mesh>> CreatePrimitiveMeshesFromGLTFFile(const VulkanReferences&, const std::string& path, ShaderPipeline* pbrShader, const vector<ShaderParameter::MParameter>& pbrMParams, int textureParamStartIndex, bool isStorageBuffer = false);

    WBuffer vertexBuffer;
    WBuffer indexBuffer;
    uint32_t indexCount;

    void GetPositions(vector<vec3>*) const;
    const vector<uint32_t>& GetIndices() const;

    uPtr<MultiPrimitivePBRInfo> multiPrimitivePBR;
    uPtr<SinglePrimitivePBRInfo> singlePrimitivePBR;
private:
    friend class WRenderPass;

    void CreateBuffers(const VulkanReferences&);
    void LoadOBJModel(const std::string& path);

    void CreateFromGLTFPrimitive(const VulkanReferences&, ShaderPipeline* pbrShader, const vector<ShaderParameter::MParameter>& pbrMParams, int textureParamStartIndex, const tinygltf::Model& model, const tinygltf::Primitive& prim, bool isStorageBuffer = false);
    void LoadGLTFModelAndTextures(const VulkanReferences& ref, const std::string& path);
    void LoadGLTFPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& prim, bool accountForMultiplePrimitives);


    vector<Vertex> vertices;
    vector<uint32_t> indices;

    bool isStorageBuffer;
};

// Should be uniform
struct UPBRInfo {
    vec4 albedoMult;
    bool hasAlbedoTex;
    bool hasMetallicRoughnessTex;
    bool hasAOTex;
};