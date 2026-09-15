import Raytracing;
import PBR;
import sh_sampling;
import probe_helper;
import misc_structs;
import bvh_traversal;

struct VSInput {
    float3 position;
    float3 color;
    float2 uv;
    float3 norm;
};
struct VSOutput {
    float4 position : SV_Position;
    float2 uv;
};

struct CameraInformation {
    float4 position;
    float4 right;
    float4 up;
    float4 forward;
    float fovY;
    float aspect;
};

ConstantBuffer<CameraInformation> uCamera;

ConstantBuffer<URaytracedScene> uRaytracedScene;
ConstantBuffer<UBoxLight> uBoxLight;
SamplerCube skybox;
StructuredBuffer<Vertex> worldVertices;
StructuredBuffer<uint32_t> worldIndices;
StructuredBuffer<uint32_t> triangleRedirection;
StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<uint32_t> triToPrimIndex;
StructuredBuffer<PrimitiveData> primData;

Sampler2D texs[];

[shader("vertex")]
VSOutput vertMain(VSInput vIn) { //uint vid : SV_VertexID) {
    VSOutput vOut;
    vOut.position = float4(vIn.position, 1.0);
    float2 uv = vIn.position.xy * 0.5 + 0.5;
    uv.y = 1. - uv.y;
    vOut.uv = uv;
    return vOut;
}

float3 Raycast(float2 uv) {
    float2 p = uv * 2. - 1.;
    float3 rdLocal = normalize(float3(p * tan(uCamera.fovY * 0.5) * float2(uCamera.aspect, 1.0), 1.0));
    float3 rdWorld = uCamera.right.xyz * rdLocal.x + uCamera.up.xyz * rdLocal.y + uCamera.forward.xyz * rdLocal.z;

    return rdWorld;
}

[shader("fragment")]
float4 fragMain(VSOutput o) : SV_Target
{
    BVHMesh bvh;
    bvh.indices = worldIndices;
    bvh.vertices = worldVertices;
    bvh.triangleRedirection = triangleRedirection;
    bvh.nodes = bvhNodes;

    float3 ro = uCamera.position.xyz;
    float3 rd = Raycast(o.uv);

    return float4(SampleEnvironmentBVHAlbedo(
        ro, rd, 
        uRaytracedScene.invModel, float3x3(uRaytracedScene.invTransposeModel), 
        bvh, 
        texs, triToPrimIndex, primData, 
        skybox, uBoxLight).rgb, 1.);
}