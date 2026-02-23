#pragma once

#include "defines.h"

// Don't duplicate CPU data. Every frame, extract and duplicate CPU data to simple GPU data to draw with

class ComputeDispatcher {
public:
	void Create(const VulkanReferences&);
	void StartRecord(const VulkanReferences&);
	void FinishRecordSubmit(const VulkanReferences&, bool waitForFinished = true);
	
	vk::raii::CommandBuffer cmd = nullptr;
	vk::raii::Semaphore computeFinishedSemaphore = nullptr; // TODO: FOR NOW UNUSED, timeline semaphores better
private:
	void WaitForFinish(const VulkanReferences&);

	vk::raii::Fence fence = nullptr;
};