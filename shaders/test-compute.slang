struct UniformData {
    uint3 invocationCount;
};
ConstantBuffer<UniformData> ubo;

StructuredBuffer<int> inBuffer;
RWStructuredBuffer<int> outBuffer;

// 64 is good for groupshared stuff
[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    if (id.x > ubo.invocationCount.x || id.y > ubo.invocationCount.y || id.z > ubo.invocationCount.z) {
        return;
    }

    outBuffer[id.x] = inBuffer[id.x] * 2;
}