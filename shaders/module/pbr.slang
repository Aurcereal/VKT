module PBR;
import sh_sampling;
import probe_helper;
import misc_structs;
import bvh_traversal;

public float3 SampleSkybox(float3 rd, SamplerCube skybox, bool normalize = false) {
    return (normalize ? 1.0 : 25.) * skybox.Sample(rd).rgb;//float3(0.4, 0.5, 1.) * 40.;//55. * float3(0.4, 0.5, 1.0) * pow(max(0., dot(rd, float3(0., 1., 1.) / sqrt(2.))), 64.); ;////10. * skybox.Sample(rd).rgb; // pow(skybox.Sample(rd).rgb,float3(2.2));  ;/
}

static float3 light_pos[4] = float3[](float3(-10, 10, 10),
                                      float3(10, 10, 10),
                                      float3(-10, -10, 10),
                                      float3(10, -10, 10));

static float3 light_col[4] = float3[](float3(300.f, 300.f, 900.f),
                                      float3(300.f, 900.f, 300.f),
                                      float3(900.f, 300.f, 300.f),
                                      float3(300.f, 300.f, 900.f));

static float PI = 3.14159f;
static float INVPI = 0.31830988618;

float3 fresnelShlick(float cosAngleFromMicroNormal, float3 baseCol) {
    return baseCol + (float3(1.0) - baseCol) * pow(1.0 - cosAngleFromMicroNormal, 5.);
}

float distribFunc(float3 macroNormal, float3 microNormal, float roughness) {
    float a = roughness * roughness;
    return a * a * INVPI * 1.0
            / pow(
                pow(dot(macroNormal, microNormal), 2.)
                * (a * a - 1.) + 1.,
                    2.);
}

float gShlick(float cosW, float k) {
    return cosW / (cosW * (1.0 - k) + k);
}

float geomOcclusion(float3 wo, float3 wi, float3 macroNormal, float roughness) {
    float k = 1.0; // pow(roughness+1, 2.) / 8.;
    return gShlick(dot(wo, macroNormal), k) * gShlick(dot(wi, macroNormal), k);
}

public float3 reinhardOp(float3 energy) {
    return energy / (1.0 + energy);
}

public float3 gammaCorrect(float3 col) {
    return pow(col, float3(1. / 2.2));
}

public float3 textureGammaCorrect(float3 texColor) {
    return pow(texColor, float3(2.2));
}

float3 SamplePBR(float3 wo, float3 worldPos, float3 norm, float3 albedo, float metallic, float roughness, float ao, float3 irradiance) {
    // float3 Lo = float3(0.);
    // Lo += 0.03 * ao * albedo; // Ambient Light

    // float att = 0.;

    // for (int i = 0; i < 4; i++) {
    //     float3 wi = normalize(light_pos[i] - worldPos);
    //     float3 wh = normalize(wo + wi);

    //     float3 fragToLight = light_pos[i] - worldPos;
    //     float attenuation = 1.0 / dot(fragToLight, fragToLight); att += attenuation;
    //     float3 Li = light_col[i] * attenuation;

    //     //
    //     float3 fresnelCol = lerp(float3(0.04), albedo, metallic);
    //     float3 fresnel = fresnelShlick(max(dot(wh, wo), 0.0), fresnelCol);

    //     float distrib = distribFunc(norm, wh, roughness);

    //     float geom = geomOcclusion(wo, wi, norm, roughness);

    //     //
    //     float lambert = max(0., dot(wi, norm));

    //     //
    //     float3 fCookTorrance = fresnel * distrib * geom / (4.0 * dot(norm, wo) * dot(norm, wi));
    //     float3 fLambert = albedo * INVPI;

    //     float3 ks = fresnel;
    //     float3 kd = float3(1.0) - ks;
    //     kd *= 1.0 - metallic; // More metal less diffuse

    //     Lo += fCookTorrance * Li * lambert; // Specular
    //     Lo += kd * fLambert * Li * lambert; // Diffuse
    // }
    return irradiance * INVPI * albedo;
}

public float3 SamplePBRSky(float3 wo, float3 worldPos, float3 norm, float3 albedo, float metallic, float roughness, float ao, StructuredBuffer<float> skyboxSh) {
    return SamplePBR(wo, worldPos, norm, albedo, metallic, roughness, ao, SampleShIrradiance(norm, skyboxSh, 0));//  10. * float3(0.4, 0.5, 1.0) * 3.141592);//
}

public float3 SamplePBREnv(float3 wo, float3 worldPos, float3 norm, float3 albedo, float metallic, float roughness, float ao, StructuredBuffer<float> sh, Sampler2D<float> depthTexture, ConstantBuffer<UProbeVolumeLayout> probeLayout,
                           float4x4 invModel, SamplerCube skybox, in BVHMesh bvh,
                           int ignoredProbe = -1) {

    float3 lightDir = float3(0.0, 1.0, 1.0) / sqrt(2.0);

    float3 lro = mul(invModel, float4(worldPos, 1.)).xyz;
    float3 lLightDir = mul(invModel, float4(lightDir, 0.)).xyz;
    Intersection shadowInter = RayIntersectBVH(lro, lLightDir, bvh);

    float3 indirect = SampleInterpolatedShIrradiance(norm, worldPos, sh, depthTexture, probeLayout, ignoredProbe);
    float3 direct = SampleSkybox(lightDir, skybox) * step(1e4, shadowInter.t) * max(0., dot(norm, lightDir));
    return SamplePBR(wo, worldPos, norm, albedo, metallic, roughness, ao, indirect + direct);
}

// public float3 SamplePBRSkyRadiance(float3 wo, float3 worldPos, float3 norm, float3 albedo, float metallic, float roughness, float ao, StructuredBuffer<float> skyboxSh) {
//     return SamplePBR(wo, worldPos, norm, albedo, metallic, roughness, ao, SampleSh(norm, skyboxSh, 0)); //  10. * float3(0.4, 0.5, 1.0) * 3.141592);//
// }

// public float3 SamplePBREnvRadiance(float3 wo, float3 worldPos, float3 norm, float3 albedo, float metallic, float roughness, float ao, StructuredBuffer<float> sh, Sampler2D<float> depthTexture, ConstantBuffer<UProbeVolumeLayout> probeLayout, int ignoredProbe = -1) {
//     return SamplePBR(wo, worldPos, norm, albedo, metallic, roughness, ao, SampleInterpolatedSh(norm, worldPos, sh, depthTexture, probeLayout, ignoredProbe));
// }