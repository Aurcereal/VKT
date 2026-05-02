module Raytracing;
import PBR;
import bvh_traversal;
import misc_structs;

public struct URaytracedScene {
    public float4x4 model;
    public float4x4 invModel;
    public float4x4 invTransposeModel;
}

public float3 SampleSkybox(float3 rd, SamplerCube skybox) {
    return 25. * float3(0.4, 0.5, 1.0) * pow(max(0., dot(rd, float3(1., 1., 0.)/sqrt(2.))), 64.);//10. * skybox.Sample(rd).rgb;//10. * skybox.Sample(rd).rgb; // pow(skybox.Sample(rd).rgb,float3(2.2));  ;/
}

public float4 SampleEnvironment(float3 ro, float3 rd,
        float4x4 model, // todo: inv transpose
        StructuredBuffer<Vertex> worldVertices,
        StructuredBuffer<uint32_t> worldIndices,
        Sampler2D worldTex,
        Sampler2D worldMetal,
        Sampler2D worldRoughness,
        Sampler2D worldAO,
        StructuredBuffer<float> skyboxSh,
        SamplerCube skybox
    ) {
    float minDist = 1e4;
    float2 hitUv;
    float3 hitPos, hitNorm;
    for (uint32_t i = 0; i < 4596; i += 3) { // TODO: dont hardcode
        Vertex vs[3] = Vertex[](worldVertices[worldIndices[i]], worldVertices[worldIndices[i + 1]], worldVertices[worldIndices[i + 2]]);
        float3 vp[3] = float3[](
            mul(model, float4(vs[0].pos, 1.)).xyz,
            mul(model, float4(vs[1].pos, 1.)).xyz,
            mul(model, float4(vs[2].pos, 1.)).xyz
        );
        // TODO: ( later should localize query to run bvh)
        float3 cl = cross(vp[2] - vp[0], vp[1] - vp[0]);
        if (length(cl) < 0.001) continue;
        float area = 0.5 * length(cl);
        float3 oNorm = normalize(cl);

        // Plane
        float dist = -dot(oNorm, ro - vp[0]) / dot(oNorm, rd);
        if (dist > 0.01 && dist < minDist) {
            float3 hitPnt = ro + rd * dist;
            float b2 = 0.5 * length(cross(hitPnt - vp[0], hitPnt - vp[1]));
            float b0 = 0.5 * length(cross(hitPnt - vp[1], hitPnt - vp[2]));
            float b1 = 0.5 * length(cross(hitPnt - vp[2], hitPnt - vp[0]));
            if (b0 + b1 + b2 <= area + 0.001) {
                minDist = dist;

                hitUv = (b0 * vs[0].uv + b1 * vs[1].uv + b2 * vs[2].uv) / area;
                hitPos = (b0 * vp[0] + b1 * vp[1] + b2 * vp[2]) / area;
                hitNorm = normalize(mul(model, float4(b0 * vs[0].norm + b1 * vs[1].norm + b2 * vs[2].norm, 0.)).xyz); // TODO: make inverse transpose
            }
        }
    }

    if (minDist < 1e4) {
        float3 albedo = textureGammaCorrect(worldTex.Sample(hitUv).rgb);
        float metallic = 0.;   // worldMetal.Sample(hitUv).r;
        float roughness = 0.2; // worldRoughness.Sample(hitUv).r;
        float ao = 0.;         // worldAO.Sample(hitUv).r;
        return float4(SamplePBRSky(normalize(ro - hitPos), hitPos, hitNorm, albedo, metallic, roughness, ao, skyboxSh), minDist);
    } else {
        return float4(SampleSkybox(rd, skybox), 1e4);
    }
}

public float4 SampleEnvironmentBVH(float3 ro, float3 rd,
                                   float4x4 invModel,
                                   float3x3 invTransposeModel,
                                   BVHMesh bvhMesh,
                                   Sampler2D worldTex,
                                   Sampler2D worldMetalRoughness,
                                   Sampler2D worldAO,
                                   StructuredBuffer<float> skyboxSh,
                                   SamplerCube skybox
    ) {
    float3 lro = mul(invModel, float4(ro, 1.)).xyz;
    float3 lrd = mul(invModel, float4(rd, 0.)).xyz;
    Intersection inter = RayIntersectBVH(lro, lrd, bvhMesh);
    if (inter.t < 1e4) {
        float3 lHitPnt = lro + lrd * inter.t;

        Vertex vs[3] = Vertex[3](
            bvhMesh.vertices[bvhMesh.indices[inter.triangleIndex * 3 + 0]],
            bvhMesh.vertices[bvhMesh.indices[inter.triangleIndex * 3 + 1]],
            bvhMesh.vertices[bvhMesh.indices[inter.triangleIndex * 3 + 2]]
        );

        float3 cl = cross(vs[2].pos - vs[0].pos, vs[1].pos - vs[0].pos);
        // if (length(cl) < 0.001) continue;
        float area = 0.5 * length(cl);

        float b2 = 0.5 * length(cross(lHitPnt - vs[0].pos, lHitPnt - vs[1].pos));
        float b0 = 0.5 * length(cross(lHitPnt - vs[1].pos, lHitPnt - vs[2].pos));
        float b1 = 0.5 * length(cross(lHitPnt - vs[2].pos, lHitPnt - vs[0].pos));
        float2 hitUv = (b0 * vs[0].uv + b1 * vs[1].uv + b2 * vs[2].uv) / area;
        // float3 lHitPos = (b0 * vs[0].pos + b1 * vs[1].pos + b2 * vs[2].pos) / area;
        float3 lHitNorm = b0 * vs[0].norm + b1 * vs[1].norm + b2 * vs[2].norm;

        float3 hitNorm = normalize(mul(invTransposeModel, lHitNorm));
        float3 hitPos = ro + rd * inter.t;

        float3 albedo = textureGammaCorrect(worldTex.Sample(hitUv).rgb);
        float metallic = 0.;   // worldMetal.Sample(hitUv).r;
        float roughness = 0.2; // worldRoughness.Sample(hitUv).r;
        float ao = 0.;         // worldAO.Sample(hitUv).r;
        return float4(SamplePBRSky(normalize(ro - hitPos), hitPos, hitNorm, albedo, metallic, roughness, ao, skyboxSh).rgb, inter.t);
    } else {
        return float4(SampleSkybox(rd, skybox), 1e4);
    }
}