struct UniformBuffer {
    float off;
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

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
    if (objPos.z > 80.) {
        objPos.z += 15000.;
    }
    float4 worldPos = mul(ubo.model, float4(objPos, 1.0));
    vOut.worldPos = worldPos.xyz;
    vOut.position = mul(ubo.proj, mul(ubo.view, worldPos));
    vOut.color = vIn.color;
    vOut.uv = vIn.uv;
    vOut.norm = mul(ubo.model, float4(vIn.norm, 0.0)).xyz; // make do opposite scale (inv trans)
    return vOut;
}

Sampler2D texSampler;
Sampler2D metallicSampler;
Sampler2D roughnessSampler;
Sampler2D aoSampler;

struct Vertex {
    float3 pos;
    float3 color;
    float2 uv;
    float3 norm;
};

StructuredBuffer<Vertex> vertices;
StructuredBuffer<uint32_t> indices;

SamplerCube skybox;

StructuredBuffer<float> shCoefficients;
float3 sampleSh(float3 rd) {
    // TODO: put in module andor generate into a cubemap so we dont have to do math after caching
    float3 c1 = float3(shCoefficients[0], shCoefficients[1], shCoefficients[2]);
    float3 c2 = float3(shCoefficients[3], shCoefficients[4], shCoefficients[5]);
    float3 c3 = float3(shCoefficients[6], shCoefficients[7], shCoefficients[8]);
    float3 c4 = float3(shCoefficients[9], shCoefficients[10], shCoefficients[11]);
    float3 c5 = float3(shCoefficients[12], shCoefficients[13], shCoefficients[14]);
    float3 c6 = float3(shCoefficients[15], shCoefficients[16], shCoefficients[17]);
    float3 c7 = float3(shCoefficients[18], shCoefficients[19], shCoefficients[20]);
    float3 c8 = float3(shCoefficients[21], shCoefficients[22], shCoefficients[23]);
    float3 c9 = float3(shCoefficients[24], shCoefficients[25], shCoefficients[26]);

    return float3(0.); // TODO: complete and remember we have to get the cosine term too
    // convolution of f and g multiplied is equal to frequency version of f multiplied by frequency version of g (find frequency ver of cos online)
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

float3 reinhardOp(float3 energy) {
    return energy / (1.0 + energy);
}

float3 gammaCorrect(float3 col) {
    return pow(col, float3(1. / 2.2));
}

float3 textureGammaCorrect(float3 texColor) {
    return pow(texColor, float3(2.2));
}

[shader("fragment")]
float4 fragMain(FSInput o) : SV_Target
{
    float4x4 viewTranspose = transpose(ubo.view);
    float3 camPos = -mul(float3x3(viewTranspose), viewTranspose[3].xyz);
    float3 norm = normalize(o.norm);//normalize(o.worldPos); // for sphere for now, but get normals and test them soon

    float3 sta = o.worldPos + norm * 0.01;
    float3 reflRay = normalize(reflect(o.worldPos - camPos, norm));
    float3 skyCol = skybox.Sample(reflRay).rgb;
    float minDist = 10000.;
    float2 uv = o.uv;
    for (uint32_t i = 0; i < 7440; i += 3) {
        Vertex vs[3] = Vertex[](vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]]);
        float3 vp[3] = float3[](
            mul(ubo.model, float4(vs[0].pos, 1.)).xyz,
            mul(ubo.model, float4(vs[1].pos, 1.)).xyz,
            mul(ubo.model, float4(vs[2].pos, 1.)).xyz
        );
        // TODO: PUT VERTICES IN WORLD SPACE FOR NOW ( later should localize query to run bvh)
        float3 cl = cross(vp[2] - vp[0], vp[1] - vp[0]);
        float area = 0.5 * length(cl);
        float3 oNorm = normalize(cl);
        
        // Plane
        float dist = -dot(oNorm, sta - vp[0]) / dot(oNorm, reflRay);
        if (dist > 0.01 && dist < minDist) {
            float3 hitPnt = sta + reflRay * dist;
            float b2 = 0.5*length(cross(hitPnt - vp[0], hitPnt - vp[1]));
            float b0 = 0.5*length(cross(hitPnt - vp[1], hitPnt - vp[2]));
            float b1 = 0.5*length(cross(hitPnt - vp[2], hitPnt - vp[0]));
            if (b0 + b1 + b2 <= area) {
                uv = (b0 * vs[0].uv + b1 * vs[1].uv + b2 * vs[2].uv)/area;
                minDist = dist;
            }
        }
    }

    const float3 albedo = textureGammaCorrect(texSampler.Sample(uv).rgb); // float3(1.0,0.,0.);
    const float metallic = metallicSampler.Sample(uv).r;                  // 1.0; // 0.5
    const float roughness = roughnessSampler.Sample(uv).r;                // 0.1;
    const float ambientOcclusion = 0.;//aoSampler.Sample(o.uv).r;//0.;

    float3 Lo = float3(0.);
    Lo += 0.03 * ambientOcclusion * albedo; // Ambient Light

    float3 wo = normalize(camPos - o.worldPos);
    for (int i = 0; i < 4; i++) {
        float3 wi = normalize(light_pos[i] - o.worldPos);
        float3 wh = normalize(wo + wi);

        float3 fragToLight = light_pos[i] - o.worldPos;
        float attenuation = 1.0 / dot(fragToLight, fragToLight);
        float3 Li = light_col[i] * attenuation;

        //
        float3 fresnelCol = lerp(float3(0.04), albedo, metallic);
        float3 fresnel = fresnelShlick(max(dot(wh, wo), 0.0), fresnelCol);

        float distrib = distribFunc(norm, wh, roughness);

        float geom = geomOcclusion(wo, wi, norm, roughness);

        //
        float lambert = max(0., dot(wi, norm));

        //
        float3 fCookTorrance = fresnel * distrib * geom / (4.0 * dot(norm, wo) * dot(norm, wi));
        float3 fLambert = albedo * INVPI;

        float3 ks = fresnel;
        float3 kd = float3(1.0) - ks;
        kd *= 1.0 - metallic; // More metal less diffuse

        float3 fComposite = kd * fLambert + fCookTorrance;
        Lo += fComposite * Li * lambert;
    }

    float3 col = gammaCorrect(reinhardOp(Lo));

    return float4(skyCol, 1.);

}