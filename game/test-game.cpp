#include "test-game.h"
#include <iostream>

void TestGame::Update(float time, float dt) {
#if SCENE == 0
	raytraceSceneTransform = glm::scale(mat4(1.0f), vec3(0.04f)) * glm::rotate(mat4(1.0f), glm::radians(0.0f), vec3(0, 1, 0)) * glm::rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * glm::scale(mat4(1.0f), vec3(0.014f + 0.5));
#elif SCENE == 1
	raytraceSceneTransform = glm::scale(mat4(1.0f), vec3(1.4f));
#elif SCENE == 2
	raytraceSceneTransform = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f)) * glm::scale(mat4(1.0f), vec3(1.4f));
#endif
	ballTransform = glm::rotate(mat4(1.0f), glm::radians(0.0f), vec3(0,1,0)) * glm::translate(mat4(1.0f), ballOffset) * glm::scale(mat4(1.0f), vec3(0.5f));
	boxLightTransform = glm::translate(mat4(1.0f), boxLightPosition) * glm::scale(mat4(1.0f), vec3(0.5f));
}

void TestGame::DrawUI() {
	ImGui::Begin("Ball Transform");

	float posArr[3] = { ballOffset.x, ballOffset.y, ballOffset.z };
	if (ImGui::SliderFloat3("Position", posArr, -20.0f, 20.0f)) {
		ballOffset = vec3(posArr[0], posArr[1], posArr[2]);
	}

	ImGui::Separator();

	float lightPosArr[3] = {boxLightPosition.x, boxLightPosition.y, boxLightPosition.z};
	if (ImGui::SliderFloat3("Light Position", lightPosArr, -20.0f, 20.0f))
		boxLightPosition = glm::make_vec3(lightPosArr);

	float lightColArr[3] = { boxLightColor.x, boxLightColor.y, boxLightColor.z };
	if(ImGui::ColorEdit3("Light Color", lightColArr, ImGuiColorEditFlags_HDR))
		boxLightColor = glm::make_vec3(lightColArr);

	ImGui::End();
}