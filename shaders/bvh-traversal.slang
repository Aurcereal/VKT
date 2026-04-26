module bvh_traversal;

import intersection_helper;
import math_helper;
import misc_structs;

public struct BVHNode {
    float3 lowExtent;
    uint32_t childStartIndex;
    float3 highExtent;
    uint32_t triangleCount;
};

public struct Intersection {
    public float t;
    public uint32_t triangleIndex;
};

public struct BVHMesh {
    public StructuredBuffer<Vertex> vertices;
    public StructuredBuffer<uint32_t> indices;

    public StructuredBuffer<BVHNode> nodes;
    public StructuredBuffer<uint32_t> triangleRedirection;
};

inline void GetTriangle(BVHMesh bvhMesh, uint32_t bvhTriIndex, out float3 v0, out float3 v1, out float3 v2) {
    uint32_t meshIndex = bvhMesh.triangleRedirection[bvhTriIndex];
    v0 = bvhMesh.vertices[bvhMesh.indices[meshIndex * 3 + 0]].pos;
    v1 = bvhMesh.vertices[bvhMesh.indices[meshIndex * 3 + 1]].pos;
    v2 = bvhMesh.vertices[bvhMesh.indices[meshIndex * 3 + 2]].pos;
}

// TODO: can try with an index traversal stack too, but sizeof(BVHNode) is so small rn
public Intersection RayIntersectBVH(float3 ro, float3 rd, BVHMesh bvh, out float checkCount) {
    BVHNode traversalStack[64];
    uint stackPtr = 0;
    BVHNode currNode = bvh.nodes[0];

    Intersection inter;
    inter.t = 1e4;
    inter.triangleIndex = 0;

    checkCount = 0.;

    bool finished = false;
    while (!finished) {
        if (currNode.triangleCount > 0) {
            // Is Leaf
            uint32_t start = currNode.childStartIndex; uint32_t end = start + currNode.triangleCount;
            checkCount += float(end - start);
            for (uint32_t i = start; i < end; ++i) {
                // Intersect triangles
                float3 v0, v1, v2;
                GetTriangle(bvh, i, v0, v1, v2);
                float tt = RayTriangleIntersection(ro, rd, v0, v1, v2);
                if (tt < inter.t) {
                    inter.t = tt;
                    inter.triangleIndex = bvh.triangleRedirection[i];
                }
            }
            if (stackPtr == 0) {
                finished = true;
                continue;
            } else {
                currNode = traversalStack[--stackPtr];
                continue;
            }
        }

        // Intersect children
        BVHNode leftChild = bvh.nodes[currNode.childStartIndex];
        BVHNode rightChild = bvh.nodes[currNode.childStartIndex + 1];

        float2 leftInter = RayBoxIntersection(ro, rd, leftChild.lowExtent, leftChild.highExtent);
        float2 rightInter = RayBoxIntersection(ro, rd, rightChild.lowExtent, rightChild.highExtent);

        if (leftInter.x > leftInter.y || leftInter.y < 0.) leftInter.x = 1e4;
        if (rightInter.x > rightInter.y || rightInter.y < 0.) rightInter.x = 1e4;

        // leftChild will be the closest
        if (rightInter.x < leftInter.x) {
            Swap(leftInter, rightInter);
            Swap(leftChild, rightChild);
        }

        // Add to stack
        if (leftInter.x >= 1e4) {
            if (stackPtr == 0) {
                finished = true;
                continue;
            } else {
                currNode = traversalStack[--stackPtr];
            }
        } else {
            currNode = leftChild;
            if (rightInter.x < 1e4) {
                traversalStack[stackPtr++] = rightChild;
            }
        }
    }

    return inter;
}

