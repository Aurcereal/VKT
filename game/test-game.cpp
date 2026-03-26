#include "test-game.h"
#include <iostream>

void TestGame::Update(float time, float dt) {
	roomTransform = glm::rotate(mat4(1.0f), 0 * time, vec3(0.0f, 1.0f, 0.0f)) * glm::scale(mat4(1.0f), vec3(0.014f + 0.5));
	ballTransform = glm::translate(mat4(1.0f), ballOffset) * glm::scale(mat4(1.0f), vec3(0.5f));
}

void TestGame::DrawUI() {
	ImGui::Begin("Ball Transform");

	float posArr[3] = { ballOffset.x, ballOffset.y, ballOffset.z };
	if (ImGui::SliderFloat3("Position", posArr, -8.0f, 8.0f)) {
		ballOffset = vec3(posArr[0], posArr[1], posArr[2]);
	}

	ImGui::End();
}