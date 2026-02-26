import Raytracing;

// Per work-group variable
// All threads in workgroup parallely write to this
// Then after, we sync and add all coefficients to the big main buffer
// We do 1/64 times less atomic adds than every thread using an atomic add
groupshared float3[64][9] sharedCoefficients;

SamplerCube skybox;
RWStructuredBuffer<float> shCoefficients;

#define PI 3.141592

float hash11(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return fract(float(x) * 0.01);
}

float2 hash12(uint x) {
    return float2(hash11(x), hash11(x * 14 + 52));
}

float3 uniformSphere(float2 xi) {
    float theta = xi.x * 2. * PI;

    float y = xi.y * 2. - 1.;
    float r = 1. - y * y;

    float3 v = float3(r * float2(cos(theta), sin(theta)), y).xzy;

    return normalize(v); // Should already be normalized but just in case
}

// later will add ro component when considering multiple probes
// need to decouple compute pipeline and compute material lol, should be ez tho
// cuz like i wanna vary ro but keep pipeline same
[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 id: SV_DispatchThreadID, uint threadIDWithinGroup : SV_GroupThreadID) { // GroupTHreadID is id within group flattened to 1D
    float3 rd = uniformSphere(hash12(id.x+id.y*1024)); // 1024 temp make it the baked size
    float3 li = SampleSkybox(rd, skybox);

    // All of these basis functions are normalized so no need to divide by their function dot product
    sharedCoefficients[threadIDWithinGroup][0] = 0.28209479 * li;

    sharedCoefficients[threadIDWithinGroup][1] = 0.4886025119029199 * rd.y * li;
    sharedCoefficients[threadIDWithinGroup][2] = 0.4886025119029199 * rd.z * li;
    sharedCoefficients[threadIDWithinGroup][3] = 0.4886025119029199 * rd.x * li;

    sharedCoefficients[threadIDWithinGroup][4] = 1.0925484305920792 * rd.x * rd.y * li;
    sharedCoefficients[threadIDWithinGroup][5] = 1.0925484305920792 * rd.y * rd.z * li;
    sharedCoefficients[threadIDWithinGroup][6] = 0.31539156525252005 * (3 * rd.z * rd.z - 1) * li;
    sharedCoefficients[threadIDWithinGroup][7] = 1.0925484305920792 * rd.x * rd.z * li;
    sharedCoefficients[threadIDWithinGroup][8] = 0.5462742152960396 * (rd.x * rd.x - rd.y * rd.y) * li;

    // Wait til all threads in group are done
    GroupMemoryBarrierWithGroupSync();

    // First thread will be accumulator
    if (threadIDWithinGroup == 0) {
        for (int basis = 0; basis < 9; basis++) {
            float3 sum = 0.;
            for (int thread = 0; thread < 64; thread++) {
                sum += sharedCoefficients[thread][basis];
            }
            InterlockedAdd(shCoefficients[basis*3 + 0], sum.x);
            InterlockedAdd(shCoefficients[basis*3 + 1], sum.y);
            InterlockedAdd(shCoefficients[basis*3 + 2], sum.z);
        }
    }
}