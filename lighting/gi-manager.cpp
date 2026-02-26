#include "gi-manager.h"

#include "scene/compute-dispatcher.h"
#include "scene/shader-parameter.h"

#include "scene/material.h"

void GIManager::Test(WTexture* skybox) {
	GenerateSHCoefficients(skybox);
}

void GIManager::GenerateSHCoefficients(WTexture* skybox) {
	shCoefficients.Create(*ref, sizeof(float) * 3 * 9, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vector sParams = {
		ShaderParameter::SParameter{.type = ShaderParameter::Type::SAMPLER, .visibility = vk::ShaderStageFlagBits::eCompute},
		ShaderParameter::SParameter{.type = ShaderParameter::Type::BUFFER, .visibility = vk::ShaderStageFlagBits::eCompute},
	};

	vector mParams = {
		ShaderParameter::MParameter(ShaderParameter::USampler{.texture = skybox}),
		ShaderParameter::MParameter(ShaderParameter::UBuffer{.buffer = &shCoefficients}),
	};

	ComputePipeline generateShader; // need better shader name like sh generator lol
	generateShader.Create(*ref, "shaders/spherical-harmonics.spv", sParams, mParams, uvec3(8, 8, 1));

	ComputeDispatcher dispatcher;
	dispatcher.Create(*ref);

	dispatcher.StartRecord(*ref);
	generateShader.EnqueueDispatch(&dispatcher, uvec3(32, 32, 1));
	dispatcher.FinishRecordSubmit(*ref);
}