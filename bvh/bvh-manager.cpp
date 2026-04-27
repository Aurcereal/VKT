#include "bvh-manager.h"

uPtr<BVHGPU> BVHManager::BuildBVH(const VulkanReferences& ref, const Mesh& mesh) {
	vector<vec3> positions;
	mesh.GetPositions(&positions);
	const vector<uint32_t>& indices = mesh.GetIndices();

	auto bvh = builder.BuildBVH(positions, indices);
	uPtr<BVHGPU> bvhGpu = mkU<BVHGPU>();

	// The array of structs copy only works since sizeof(BVHNode) == gpuSizeof(BVHNode) but wouldn't be the case if the size wasn't a multiple of 16, the highest alignment of a member var
	bvhGpu->nodeBuffer.CreateDeviceLocalFromData(ref, sizeof(BVHNode) * bvh->bvhNodes.size(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, bvh->bvhNodes.data());
	bvhGpu->triangleRedirectionBuffer.CreateDeviceLocalFromData(ref, sizeof(uint32_t) * bvh->triangleRedirection.size(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, bvh->triangleRedirection.data());

	return std::move(bvhGpu);
}