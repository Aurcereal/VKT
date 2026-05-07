#pragma once

#include "defines.h"
#include "imgui/imgui.h"

using namespace glm;

class TestGame {
public:
	void DrawUI();
	void Update(float time, float dt);

	mat4 raytraceSceneTransform, ballTransform, boxLightTransform;

	vec3 ballOffset;

	vec3 boxLightPosition;
	vec3 boxLightColor;
private:
	
};