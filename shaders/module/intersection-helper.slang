module intersection_helper;

public float IntersectAABB(const float3 ro, const float3 rd, const float3 bmin, const float3 bmax, const float currT) // curr T is important so we dont trace boxes past triangles we already hit
{
    float tx1 = (bmin.x - ro.x) / rd.x, tx2 = (bmax.x - ro.x) / rd.x;
    float tmin = min(tx1, tx2); float tmax = max(tx1, tx2);
    float ty1 = (bmin.y - ro.y) / rd.y, ty2 = (bmax.y - ro.y) / rd.y;
    tmin = max(tmin, min(ty1, ty2)); tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bmin.z - ro.z) / rd.z, tz2 = (bmax.z - ro.z) / rd.z;
    tmin = max(tmin, min(tz1, tz2)); tmax = min(tmax, max(tz1, tz2));
    if (tmax >= tmin && tmin < currT && tmax > 0) return tmin; else return 1e4;
}

public float2 RayBoxIntersection(float3 ro, float3 rd, float3 minBound, float3 maxBound) {
    const float3 invRd = 1. / rd;

    const float3 center = 0.5 * (minBound + maxBound);
    ro -= center;

    const float3 halfDim = 0.5 * (maxBound - minBound);

    const float3 t1 = (-halfDim - ro) * invRd;
    const float3 t2 = (halfDim - ro) * invRd;

    const float tEnter = max(max(min(t1.x, t2.x), min(t1.y, t2.y)), min(t1.z, t2.z));
    const float tExit = min(min(max(t1.x, t2.x), max(t1.y, t2.y)), max(t1.z, t2.z));

    return float2(tEnter, tExit);
}

// Taken from jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/ todo: derive w cramer's rule
public float RayTriangleIntersection(float3 ro, float3 rd, float3 v0, float3 v1, float3 v2)
{
    const float3 edge1 = v1 - v0;
    const float3 edge2 = v2 - v0;
    const float3 h = cross(rd, edge2);
    const float a = dot(edge1, h);
    if (a > -0.0001f && a < 0.0001f) return 1e4; // ray parallel to triangle
    const float f = 1 / a;
    const float3 s = ro - v0;
    const float u = f * dot(s, h);
    if (u < 0 || u > 1) return 1e4;
    const float3 q = cross(s, edge1);
    const float v = f * dot(rd, q);
    if (v < 0 || u + v > 1) return 1e4;
    const float t = f * dot(edge2, q);
    if (t > 0.0001f) return t;
    return 1e4;
}
