#pragma once

#include "defines.h"
#include "scene/buffer.h"
#include "scene/texture.h"
#include <vector>

using namespace std;

namespace ShaderParameter {
	enum class Type {
		UNIFORM,
		DYNAMIC_UNIFORM,
		COMBINED_SAMPLER,
		COMBINED_SAMPLER_ARRAY,
		BUFFER,
		STORAGE_TEXTURE
	};

	struct UUniform {
		// We should have a notion of uniforms and whether they're frame in flight duplicated like a uniform.h...
		const vector<WBuffer>* uniformBuffers;
	};

	struct UDynamicUniform {
		const vector<WBuffer>* buffers;
		vk::DeviceSize singleObjectSize;
	};

	struct UCombinedSampler {
		WTexture* texture;
	};

	struct UCombinedSamplerArray {
		vector<WTexture>* textures;
	};

	struct UBuffer {
		const WBuffer* buffer;
	};

	struct UStorageTexture {
		WTexture* texture;
	};

	struct MParameter {
		Type type;
		union {
			UUniform uniform;
			UDynamicUniform dynamicUniform;
			UCombinedSampler sampler;
			UCombinedSamplerArray samplers;
			UBuffer buffer;
			UStorageTexture storageTexture;
		};

		inline MParameter(UUniform u) : type(Type::UNIFORM), uniform(u) {}
		inline MParameter(UDynamicUniform u) : type(Type::DYNAMIC_UNIFORM), dynamicUniform(u) {}
		inline MParameter(UCombinedSampler s) : type(Type::COMBINED_SAMPLER), sampler(s) {}
		inline MParameter(UCombinedSamplerArray s) : type(Type::COMBINED_SAMPLER_ARRAY), samplers(s) {}
		inline MParameter(UBuffer b) : type(Type::BUFFER), buffer(b) {}
		inline MParameter(UStorageTexture s) : type(Type::STORAGE_TEXTURE), storageTexture(s) {}

		// inline ~MParameter() {}
	};

	struct SParameter {
		Type type;
		vk::ShaderStageFlagBits visibility;
		// For now Shader Parameters don't need any actual data besides type
	};
};