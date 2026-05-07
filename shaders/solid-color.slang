import Raytracing;
import PBR;
import sh_sampling;
import probe_helper;
import misc_structs;

ConstantBuffer<UCamera> ubo;

struct UEntity {
    float4x4 transform;
};
ConstantBuffer<UEntity> uTransform;

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

struct FSInput {
    float4 position : SV_Position;
    float3 worldPos;
    float3 color;
    float2 uv;
    float3 norm;
    uint primID : SV_PrimitiveID;
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
    vOut.norm = mul(uTransform.transform, float4(vIn.norm, 0.0)).xyz; // make do opposite scale (inv trans)
    return vOut;
}

struct PushConstants {
    float4 col;
}

[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants;

[shader("fragment")]
float4 fragMain(FSInput o) : SV_Target
{
    return pushConstants.col;
}