#pragma once

#include "defines.h"
#include <vector>
#include <array>

using namespace glm;
using namespace std;

struct BVHNode {
	vec3 lowExtent;
	vec3 highExtent;
	uint32_t childStartIndex; uint32_t triangleCount;
};

class BVHBuilder {
public:
	void BuildBVH(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices, std::vector<BVHNode>* pBvh);
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

	void UpdateNodeBounds(BVHNode*);
	void ChooseSplitPlane(int* pAxis, float* pPos);
	void Subdivide(BVHNode*);
};

void BVHBuilder::UpdateNodeBounds(BVHNode* node) {
	assert(node->triangleCount != 0);
	node->lowExtent = vec3(std::numeric_limits<float>::min());
	node->highExtent = vec3(std::numeric_limits<float>::max());
	uint32_t start = node->childStartIndex; uint32_t end = node->childStartIndex + node->triangleCount;
	for (uint32_t i = start; i<end; i++) {
		const Triangle& t = triangles[i];
		for (int v = 0; v < 3; v++) {
			node->lowExtent = min(node->lowExtent, t.pts[v]);
			node->highExtent = max(node->highExtent, t.pts[v]);
		}
	}
}

// TODO: reserve for pointers to work
void BVHBuilder::BuildBVH(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices, std::vector<BVHNode>* pBvh) {
	// Build Triangles
	for (int i = 0; i < indices.size(); i += 3) {
		triangles.emplace_back(
			vertices[indices[i+0]],
			vertices[indices[i+1]],
			vertices[indices[i+2]]
		);
	}

	// Build Root
	pBvh->push_back({});
	BVHNode* root = &(*pBvh)[pBvh->size() - 1];
	root->childStartIndex = 0; root->triangleCount = triangles.size();
	UpdateNodeBounds(root);
}

// Triangle Count = 0 => Interior Node, Child Start Index is Node Index
// Triangle Count > 0 => Leaf Node, Child Start Index is Triangle Index