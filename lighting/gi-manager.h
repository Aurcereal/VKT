#pragma once

#include "defines.h"

#include "scene/buffer.h"
#include "scene/pipeline.h"
#include "scene/texture.h"

class GIManager {
public:
	inline GIManager(VulkanReferences* r) : ref(r) {}

	void Test(WTexture*);
	WBuffer shCoefficients; // (r1, g1, b2, r2, ... r9, g9, b9) - Number is basis id
private:
	VulkanReferences* ref;

	void GenerateSHCoefficients(WTexture*);
};