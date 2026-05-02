#pragma once

#include "defines.h"

#include "scene/material.h"
#include "scene/render-target.h"
#include "scene/mesh.h"
#include "scene/texture.h"
#include "scene/pipeline.h"

class WRenderPass {
public:
	void Create(const VulkanReferences&);

	void Start(RenderTarget*, vk::raii::CommandBuffer* cmd, bool waitForPrevFinish = true);
	void EnqueueSetMaterial(Material&, int setIndex, vector<uint32_t> dynamicOffsets = {});
	void EnqueueDraw(const Mesh&);
	void FinishExecute(bool waitForFinish = true, vk::ImageLayout targetColorLayout = vk::ImageLayout::eUndefined, vk::raii::Semaphore* waitSem = nullptr, vk::raii::Semaphore* signalSem = nullptr);

	void WaitForFinish();
private:
	const VulkanReferences* ref = nullptr;
	const RenderTarget* target = nullptr;

	vk::raii::CommandBuffer* currCmd = nullptr;
	vk::raii::Fence drawFence = nullptr;
};