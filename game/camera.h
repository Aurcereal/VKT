#pragma once
#include "input-manager.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
	Camera(vec3 pos, float aspect, float fovYRadians, vec2 nearFar = vec2(0.01f, 1000.0f));
	void Update(const InputManager&, float dt);

	mat4 GetProjectionMatrix();
	mat4 GetViewMatrix();
private:
	void RotateAboutLocalX(float amt);
	void RotateAboutGlobalY(float amt);

	vec3 pos;
	vec2 nearFar;
	vec3 ri, up, fo;
	float fovYRadians;
	float aspect;

	float lookSens;
	float moveSens;
};

