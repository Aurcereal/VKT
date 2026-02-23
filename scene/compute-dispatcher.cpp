#include "compute-dispatcher.h"

void ComputeDispatcher::Create(const VulkanReferences& ref) {
	fence = vk::raii::Fence(ref.device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	computeFinishedSemaphore = vk::raii::Semaphore(ref.device, vk::SemaphoreCreateInfo());

	vk::CommandBufferAllocateInfo computeAllocateInfo = {
			.commandPool = ref.commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
	};
	cmd = std::move(vk::raii::CommandBuffers(ref.device, computeAllocateInfo).front());
}

void ComputeDispatcher::WaitForFinish(const VulkanReferences& ref) {
	while (ref.device.waitForFences(*fence, vk::True, UINT64_MAX) == vk::Result::eTimeout) {}
}

void ComputeDispatcher::StartRecord(const VulkanReferences& ref) {
	// Wait for previous run if any
	WaitForFinish(ref);
	ref.device.resetFences(*fence);
	cmd.reset();

	cmd.begin({});
}

/// <summary>
/// If waitForFinish is on, we'll simply CPU wait for Compute Shader to finish.  If not, Semaphore will be signaled and MUST be caught by someone who uses compute shader.
/// For this reason, always waitForFinish for now until we have timeline semaphore so we can have multiple signals a 1 wait and 1 signal and multiple waits.
/// </summary>
/// <param name="ref"></param>
/// <param name="waitForFinish"></param>
void ComputeDispatcher::FinishRecordSubmit(const VulkanReferences& ref, bool waitForFinish) {
	cmd.end();

	const vk::SubmitInfo submitInfo = {
			.commandBufferCount = 1,
			.pCommandBuffers = &*cmd, // Submit this cmd buffer

			.signalSemaphoreCount = (waitForFinish ? 0u : 1),
			.pSignalSemaphores = waitForFinish ? nullptr : &*computeFinishedSemaphore
	};
	ref.computeQueue.submit(submitInfo, *fence);
	
	if (waitForFinish) {
		WaitForFinish(ref);
	}
}