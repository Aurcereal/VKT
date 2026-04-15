import probe_helper;
module ProbeVisibility;

public float2 DirectionToUV(float3 dir) {
    dir = normalize(dir);
    float3 adir = abs(dir);
    dir = dir / (adir.x + adir.y + adir.z);

    float2 uv = dir.xy;
    if (dir.z <= 0.) {
        uv = sign(uv) * (1. - abs(uv.yx));
    }
    uv = uv * 0.5 + 0.5;

    return uv;
}

// Assuming 16x16
public uint2 DirectionToTexel(float3 dir) {
    float2 uv = DirectionToUV(dir);
    return uint2(15.99999 * uv);
}

public float3 TexelToDirection(uint2 texel) {
    float2 uv = (float2(texel)+0.5) / 16.0;
    uv = uv * 2. - 1.;

    float h = 1. - abs(uv.x) - abs(uv.y);
    if (h <= 0.) {
        uv = sign(uv) * (1. - abs(uv.yx));
    }

    return normalize(float3(uv, h));
}

public uint TexelToBufferIndices(uint probeID, uint2 texel) {
    uint texelID = texel.y * 16 + texel.x;
    return probeID * 256 * 3 + texelID * 3; // 3 Indices: (depth, depth^2, weight)
}

public float2 GetNormalizedDepthAndSqr(uint probeID, uint2 texel, StructuredBuffer<float> depthBuffer) {
    uint ind = TexelToBufferIndices(probeID, texel);
    return float2(depthBuffer[ind + 0], depthBuffer[ind+1]) / max(1e-5, depthBuffer[ind + 2]);
}

public float2 GetInterpolatedDepthAndSqr(uint3 probeCounts, uint3 probeID, float2 uv, Sampler2D<float> depthTexture) {
    float4 uvs = localUVToAtlasUVs(probeCounts, probeID, uv);
    return float2(depthTexture.Sample(uvs.xy), depthTexture.Sample(uvs.zw));
}

