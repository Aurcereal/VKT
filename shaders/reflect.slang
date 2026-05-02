import Raytracing;
import PBR;
import sh_sampling;
import bvh_traversal; // TODO: dedicated struct modules
import misc_structs;

ConstantBuffer<UCamera> ubo;
struct UEntity {
    float4x4 transform;
};
ConstantBuffer<UEntity> uTransform;
ConstantBuffer<URaytracedScene> uRaytracedScene;

SamplerCube skybox;

StructuredBuffer<Vertex> worldVertices;
StructuredBuffer<uint32_t> worldIndices;
StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<uint32_t> triangleRedirection;
Sampler2D worldTex;
Sampler2D worldMetal;
Sampler2D worldAO;

StructuredBuffer<float> skyboxSh;

struct VSInput {
    float3 position;
    float3 color;
    float2 uv;
    float3 norm;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 worldPos;
    float3 color;
    float2 uv;
    float3 norm;
};

[shader("vertex")]
VSOutput vertMain(VSInput vIn) { //uint vid : SV_VertexID) {
    VSOutput vOut;
    float3 objPos = vIn.position;
    float4 worldPos = mul(uTransform.transform, float4(objPos, 1.0));
    vOut.worldPos = worldPos.xyz;
    vOut.position = mul(ubo.proj, mul(ubo.view, worldPos));
    vOut.color = vIn.color;
    vOut.uv = vIn.uv;
    vOut.norm = mul(uTransform.transform, float4(vIn.norm, 0.0)).xyz; // TODO: make do opposite scale (inv trans)
    return vOut;
}

static float PI = 3.14159f;
static float INVPI = 0.31830988618;

[shader("fragment")]
float4 fragMain(VSOutput o) : SV_Target
{
    float4x4 viewTranspose = transpose(ubo.view);
    float3 camPos = -mul(float3x3(viewTranspose), viewTranspose[3].xyz);
    float3 norm = normalize(o.norm);

    float3 sta = o.worldPos + norm * 0.01;
    float3 reflRay = normalize(reflect(o.worldPos - camPos, norm));

    // float3 Lo = SampleEnvironment(sta, reflRay, uRaytracedScene.model, worldVertices, worldIndices, worldTex, worldMetal, worldRoughness, worldAO, skyboxSh, skybox).xyz;
    BVHMesh bvh;
    bvh.indices = worldIndices;
    bvh.vertices = worldVertices;
    bvh.triangleRedirection = triangleRedirection;
    bvh.nodes = bvhNodes;
    
    float3 Lo = SampleEnvironmentBVH(sta, reflRay, uRaytracedScene.invModel, float3x3(uRaytracedScene.invTransposeModel), bvh, worldTex, worldMetal, worldAO, skyboxSh, skybox).xyz;
    float3 col = gammaCorrect(reinhardOp(Lo));

    return float4(col, 1.0);//float4(SampleSh(norm, skyboxSh, 0), 1.);//float4(col, 1.);

}