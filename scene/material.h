#pragma once

#include "defines.h"
// #include "pipeline.h"

#include "scene/buffer.h"
#include "scene/texture.h"
#include "scene/shader-parameter.h"

class WPipeline;

using namespace glm;

class Material {
public:
	void Create(WPipeline*, const VulkanReferences&, const vector<ShaderParameter::MParameter>& parameters, int duplicationCount = MAX_FRAMES_IN_FLIGHT);// WTexture&, WTexture&, WTexture&, WTexture&, array<WBuffer*, 2>& meshBuffers);
	WPipeline* pipeline = nullptr;
private:
	friend class ShaderPipeline;
	friend class ComputePipeline;

	vector<ShaderParameter::MParameter> params;
	void CreateDescriptorSet(const VulkanReferences&, const vector<ShaderParameter::MParameter>& parameters, bool pingPongSelection = false);// WTexture&, WTexture&, WTexture&, WTexture&, array<WBuffer*, 2>& meshBuffers);

	vector<vk::raii::DescriptorSet> descriptorSets;

	bool usePingPong;
	vector<vk::raii::DescriptorSet> alternateDescriptorSets;

	int duplicationCount = -1;
};

//struct UniformBufferObject {
//	alignas(4) float off;
//	alignas(16) mat4 raytraceSceneModel; // TODO: remove
//	alignas(16) mat4 view;
//	alignas(16) mat4 proj;
//};

struct UCamera {
	mat4 view;
	mat4 proj;
};

struct UEntity {
	mat4 transform;
};