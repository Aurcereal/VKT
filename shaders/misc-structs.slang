module misc_structs;

public struct UCamera {
    public float4x4 view;
    public float4x4 proj;
};

public struct Vertex {
    public float3 pos;
    public float3 color;
    public float2 uv;
    public float3 norm;
};

public struct UPBRInfo {
    public float4 albedoMult;
    public bool hasAlbedoTexture; // these unused for multi
    public bool hasMetallicRoughnessTexture;
    public bool hasAOTexture;
};

public struct UBoxLight {
    public float4x4 model;
    public float4x4 invModel;
    public float3 emission;
};