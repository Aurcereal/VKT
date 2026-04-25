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

	void UpdateNodeBounds(BVHNode*);
	void ChooseSplitPlane(const BVHNode*, int* pAxis, float* pPos);
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

void BVHBuilder::ChooseSplitPlane(const BVHNode* node, int* pAxis, float* pPos) {
	assert(node->triangleCount > 0);

	int currAxis = -1;
	float highestLength = 0.0f;
	for (int a = 0; a < 3; a++) {
		if (node->highExtent[a] - node->lowExtent[a] > highestLength) {
			currAxis = a;
			highestLength = node->highExtent[a] - node->lowExtent[a];
		}
	}

	float avgPos = 0.0f;
	uint32_t start = node->childStartIndex; uint32_t end = node->childStartIndex + node->triangleCount;
	for (uint32_t i = start; i < end; i++) {
		avgPos += GetTriangle(i).centroid[currAxis];
	}
	avgPos /= static_cast<float>(node->triangleCount);

	*pPos = avgPos;// node->lowExtent[currAxis] + highestLength * 0.5f;
	*pAxis = currAxis;
}

void BVHBuilder::Subdivide(BVHNode* node) {
	// Shouldn't split?
	if (node->triangleCount <= 2)
		return;

	// Get split plane
	int splitAxis;
	float splitPos;
	ChooseSplitPlane(node, &splitAxis, &splitPos);

	int32_t i = node->childStartIndex; int32_t j = node->childStartIndex + node->triangleCount - 1;

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

	if (i == node->childStartIndex || i == node->childStartIndex + node->triangleCount) return;

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
	if (node->triangleCount == 0) {
		node->lowExtent = vec3(-1e-5);
		node->highExtent = vec3(1e-5);
		return;
	}
	node->lowExtent = vec3(std::numeric_limits<float>::max());
	node->highExtent = vec3(std::numeric_limits<float>::min());
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
	bvh->bvhNodes.reserve(static_cast<uint32_t>(glm::ceil(glm::log2(static_cast<float>(indices.size())))) * indices.size() / 3);
	
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
		bvh->triangleRedirection.push_back(i);
	}

	// Build Root
	BVHNode* root = NewNode();
	root->childStartIndex = 0; root->triangleCount = triangles.size();
	UpdateNodeBounds(root);
	Subdivide(root);

	return std::move(bvh);
}

// Triangle Count = 0 => Interior Node, Child Start Index is Node Index
// Triangle Count > 0 => Leaf Node, Child Start Index is Triangle Index