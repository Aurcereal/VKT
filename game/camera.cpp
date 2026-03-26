#include "camera.h"

Camera::Camera(vec3 pos, float aspect, float fovYRadians, vec2 nearFar) :
	pos(pos), nearFar(nearFar),
	aspect(aspect), fovYRadians(fovYRadians),
	ri(1, 0, 0), up(0, 1, 0), fo(0, 0, -1),
	lookSens(4), moveSens(10) {
}

void Camera::Update(const InputManager& input, float dt) {
	vec3 moveInput = vec3(
		input.d - input.a,
		input.space - input.ctrl,
		input.w - input.s
	);
	moveInput *= dt * moveSens;

	pos +=
		ri * moveInput.x +
		vec3(0, 1, 0) * moveInput.y +
		fo * moveInput.z;

	RotateAboutLocalX(input.lockMouseDelta.y * lookSens);
	RotateAboutGlobalY(-input.lockMouseDelta.x * lookSens);
}

void Camera::RotateAboutLocalX(float amt) {
	up = vec3(glm::rotate(mat4(1.0f), amt, ri) * vec4(up, 0));
	fo = vec3(glm::rotate(mat4(1.0f), amt, ri) * vec4(fo, 0));
}

void Camera::RotateAboutGlobalY(float amt) {
	up = vec3(glm::rotate(mat4(1.0f), amt, vec3(0, 1, 0)) * vec4(up, 0));
	fo = vec3(glm::rotate(mat4(1.0f), amt, vec3(0, 1, 0)) * vec4(fo, 0));
	ri = vec3(glm::rotate(mat4(1.0f), amt, vec3(0, 1, 0)) * vec4(ri, 0));
}

mat4 Camera::GetProjectionMatrix() {
	return glm::perspective(fovYRadians, aspect, nearFar.x, nearFar.y);
}

mat4 Camera::GetViewMatrix() {
	return
		glm::transpose(mat4(vec4(ri, 0.0f), vec4(up, 0.0f), vec4(-fo, 0.0f), vec4(0, 0, 0, 1))) *
		glm::translate(mat4(1.0f), -pos);
}