#pragma once

#include "defines.h"
#include "imgui/imgui.h"

using namespace glm;

class TestGame {
public:
	void DrawUI();
	void Update(float time, float dt);
	mat4 roomTransform, ballTransform;
	vec3 ballOffset;
private:
	
};