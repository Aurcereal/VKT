
struct VSInput {
    float2 position;
    float3 color;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOutput vertMain(VSInput vIn) { //uint vid : SV_VertexID) {
    VSOutput vOut;
    vOut.position = float4(vIn.position, 0.0, 1.0);
    vOut.color = vIn.color;
    return vOut;
}

[shader("fragment")]
float4 fragMain(VSOutput o) : SV_Target
{
    float3 color = o.color;
    return float4(color, 1.0);
}