#pragma once

#include "defines.h"
#include <vector>
#include <array>

using namespace glm;
using namespace std;

struct BVHNode {
	vec3 lowExtent;
	uint32_t childStartIndex;
	vec3 highExtent;
	uint32_t triangleCount;
};

struct BVH {
	vector<BVHNode> bvhNodes;
	vector<uint32_t> triangleRedirection;
};

class BVHBuilder {
public:
	uPtr<BVH> BuildBVH(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices);
private:
	struct Triangle {
		std::array<vec3, 3> pts;
		vec3 centroid;
		inline Triangle(vec3 v1, vec3 v2, vec3 v3) {
			pts = { v1, v2, v3 };
			centroid = (v1 + v2 + v3) * 0.3333333f;
		}
	};
	vector<Triangle> triangles;

	uPtr<BVH> bvh;
	inline BVHNode* NewNode() {
		bvh->bvhNodes.push_back({});
		return &bvh->bvhNodes[bvh->bvhNodes.size() - 1];
	}

	Triangle& GetTriangle(uint32_t);
	void SwapTriangles(uint32_t, uint32_t);

	float EvaluateSAH(const BVHNode*, int splitAxis, float splitPos);

	void UpdateNodeBounds(BVHNode*);
	void ChooseSplitPlane(const BVHNode*, int* pAxis, float* pPos);
	float ChooseSplitPlaneSAH(const BVHNode*, int* pAxis, float* pPos); // -> cost
	void Subdivide(BVHNode*);
};