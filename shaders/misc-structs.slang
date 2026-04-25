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