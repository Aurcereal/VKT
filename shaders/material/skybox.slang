import Raytracing;
import misc_structs;

ConstantBuffer<UCamera> ubo;

struct VSInput {
    float3 position;
    // TODO: below is unnecessary but consistent is easy
    float3 color;
    float2 uv;
    float3 norm;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 worldPos;
    float dummy;
};

[shader("vertex")]
VSOutput vertMain(VSInput vIn) { //uint vid : SV_VertexID) {
    VSOutput vOut;

    vOut.worldPos = vIn.position;
    vOut.dummy =  0.1 * (vIn.color.x + vIn.uv.x + vIn.norm.x);
    vOut.position = mul(ubo.proj, 
        float4(mul(ubo.view, float4(vOut.worldPos, 0.)).xyz, 1.)
    );

    return vOut;
}

SamplerCube skybox;

[shader("fragment")]
float4 fragMain(VSOutput o) : SV_Target
{
    return float4(SampleSkybox(normalize(o.worldPos), skybox), 1.);
}