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

// TODO: these are combined image samplers w/ image, but they all use same sampler so i can just separate explicitly Texture2D and SamplerState have 1 sampler and lots of textures (need to change descriptor pool oc)
struct PushConstants {
    float4 albedoMult;
    // bool hasAlbedoTexture;
    // bool hasMetallicRoughnessTexture;
    // bool hasAOTexture;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants;
Sampler2D albedoSampler;
Sampler2D metallicRoughnessSampler;
Sampler2D aoSampler;

SamplerCube skybox;
StructuredBuffer<float> shCoefficients;
StructuredBuffer<float> skySh;
[[vk::image_format("r32f")]] Sampler2D<float> depthTexture;
ConstantBuffer<UProbeVolumeLayout> probeLayout;

[shader("fragment")]
float4 fragMain(FSInput o) : SV_Target
{
    float4x4 viewTranspose = transpose(ubo.view);
    float3 camPos = -mul(float3x3(viewTranspose), viewTranspose[3].xyz);
    float3 norm = normalize(o.norm);//normalize(o.worldPos); // for sphere for now, but get normals and test them soon

    const float3 albedo = float3(1.) * textureGammaCorrect(albedoSampler.Sample(o.uv).rgb); // float3(1.);// // float3(1.0,0.,0.);
    // TODO: all meshes should have push constants for this shader and check to see if should use these vals
    const float metallic = metallicRoughnessSampler.Sample(o.uv).g;            // 1.0; // 0.5
    const float roughness = metallicRoughnessSampler.Sample(o.uv).b;//roughnessSampler.Sample(o.uv).r;                // 0.1;
    const float ambientOcclusion = aoSampler.Sample(o.uv).r;

    float3 Lo = float3(0.);
    Lo += 0.03 * ambientOcclusion * albedo; // Ambient Light

    float3 wo = normalize(camPos - o.worldPos);

    float3 col = gammaCorrect(reinhardOp(SamplePBREnv(wo, o.worldPos+norm*0.01, norm, albedo, metallic, roughness, ambientOcclusion, shCoefficients, depthTexture, probeLayout)));

    return float4(col, 1.);

}