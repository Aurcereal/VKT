#pragma once

#include "defines.h"
#include "bvh.h"
#include "scene/buffer.h"
#include "scene/mesh.h"
#include <vector>

using namespace std;

struct BVHGPU {
	WBuffer nodeBuffer;
	WBuffer triangleRedirectionBuffer;
};

class BVHManager {
public:
	uPtr<BVHGPU> BuildBVH(const VulkanReferences&, const Mesh&);
private:
	BVHBuilder builder;
};