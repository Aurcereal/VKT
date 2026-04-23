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
		pBvh->push_back({});
		return &(*pBvh)[pBvh->size() - 1];
	}

	Triangle& GetTriangle(uint32_t);
	void SwapTriangles(uint32_t, uint32_t);

	void UpdateNodeBounds(BVHNode*);
	void ChooseSplitPlane(int* pAxis, float* pPos);
	void Subdivide(BVHNode*);
};

BVHBuilder::Triangle& BVHBuilder::GetTriangle(uint32_t i) {
	return triangles[bvh->triangleRedirection[i]];
}

void BVHBuilder::SwapTriangles(uint32_t i, uint32_t j) {
	auto temp = bvh->triangleRedirection[i];
	bvh->triangleRedirection[i] = bvh->triangleRedirection[j];
	bvh->triangleRedirection[j] = temp;
}

void BVHBuilder::Subdivide(BVHNode* node) {
	// Shouldn't split?
	if (node->triangleCount <= 2)
		return;

	// Get split plane
	int splitAxis, float splitPos;
	ChooseSplitPlane(&splitAxis, &splitPos);

	uint32_t i = node->childStartIndex; uint32_t j = node->childStartIndex + node->triangleCount;

	// Sort Triangles
	while (i <= j) {
		if (GetTriangle(i).centroid[splitAxis] < splitPos) {
			// In left child
			++i;
		} else {
			// In right child
			SwapTriangles(i, j);
			--j;
		}
	}

	// Create children
	BVHNode* leftNode = NewNode();
	BVHNode* rightNode = NewNode();

	leftNode->childStartIndex = node->childStartIndex;
	leftNode->triangleCount = i - node->childStartIndex;

	rightNode->childStartIndex = i;
	rightNode->triangleCount = node->childStartIndex + node->triangleCount - i;

	UpdateNodeBounds(leftNode);
	UpdateNodeBounds(rightNode);

	// Make parent hold children
	node->triangleCount = 0;
	node->childStartIndex = bvh->bvhNodes.size() - 2;

	// Recurse
	Subdivide(leftNode);
	Subdivide(rightNode);
}

void BVHBuilder::UpdateNodeBounds(BVHNode* node) {
	assert(node->triangleCount != 0);
	node->lowExtent = vec3(std::numeric_limits<float>::min());
	node->highExtent = vec3(std::numeric_limits<float>::max());
	uint32_t start = node->childStartIndex; uint32_t end = node->childStartIndex + node->triangleCount;
	for (uint32_t i = start; i<end; i++) {
		const Triangle& t = GetTriangle(i);
		for (int v = 0; v < 3; v++) {
			node->lowExtent = min(node->lowExtent, t.pts[v]);
			node->highExtent = max(node->highExtent, t.pts[v]);
		}
	}
}

// TODO: reserve for pointers to work
uPtr<BVH> BVHBuilder::BuildBVH(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices) {
	bvh = mkU<BVH>();
	bvh->bvhNodes.reserve(indices.size() / 3);
	
	// Build Triangles
	for (int i = 0; i < indices.size(); i += 3) {
		triangles.emplace_back(
			vertices[indices[i+0]],
			vertices[indices[i+1]],
			vertices[indices[i+2]]
		);
	}
	bvh->triangleRedirection.reserve(triangles.size());
	for (uint32_t i = 0; i < triangles.size(); i++) {
		bvh->triangleRedirection[i] = i;
	}

	// Build Root
	BVHNode* root = NewNode();
	root->childStartIndex = 0; root->triangleCount = triangles.size();
	UpdateNodeBounds(root);

	return std::move(bvh);
}

// Triangle Count = 0 => Interior Node, Child Start Index is Node Index
// Triangle Count > 0 => Leaf Node, Child Start Index is Triangle Index