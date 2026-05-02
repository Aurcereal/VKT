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

struct UPBRInfo {
    float4 albedoMult;
    bool hasAlbedoTexture;
    bool hasMetallicRoughnessTexture;
    bool hasAOTexture;
};
Sampler2D albedoSampler; // TODO: these are combined image samplers w/ image, but they all use same sampler so i can just separate explicitly Texture2D and SamplerState have 1 sampler and lots of textures (need to change descriptor pool oc)
Sampler2D metallicRoughnessSampler;
Sampler2D aoSampler;
ConstantBuffer<UPBRInfo> pbrInfo;

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

    // todo: Shouldn't be dynamically checking this.. shader features for better engine
    const float3 albedo = pbrInfo.albedoMult.rgb * (pbrInfo.hasAlbedoTexture ? textureGammaCorrect(albedoSampler.Sample(o.uv).rgb) : float3(1.));

    const float metallic = pbrInfo.hasMetallicRoughnessTexture ? metallicRoughnessSampler.Sample(o.uv).g : 0.;
    const float roughness = pbrInfo.hasMetallicRoughnessTexture ? metallicRoughnessSampler.Sample(o.uv).b : 1.;
    const float ambientOcclusion = pbrInfo.hasAOTexture ? aoSampler.Sample(o.uv).r : 0.;

    float3 Lo = float3(0.);
    Lo += 0.03 * ambientOcclusion * albedo; // Ambient Light (shouldnt it be the OPPOSITE? todo)

    float3 wo = normalize(camPos - o.worldPos);

    float3 col = gammaCorrect(reinhardOp(SamplePBREnv(wo, o.worldPos+norm*0.01, norm, albedo, metallic, roughness, ambientOcclusion, shCoefficients, depthTexture, probeLayout)));

    return float4(albedo, 1.);

}