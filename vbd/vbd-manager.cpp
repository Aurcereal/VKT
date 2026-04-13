#include "vbd-manager.h"

#include "vbd-solver.h"

void VBDManager::Initialize(const VulkanReferences& ref) {
	this->ref = &ref;
	this->initialMesh = mkU<HalfEdgeMesh>();
	initialMesh->LoadFromOBJ("models/sphere.obj");

	Bake();
}

void VBDManager::Bake() {
	ref->graphicsQueue.waitIdle();
	meshes.clear();

	solver.ResetSimulation(std::move(initialMesh));
	for (int i = 0; i < 250; i++) {
		solver.SimulateUpToFrame(i);
		meshes.push_back(std::move(solver.lastSimulatedMesh->convertToMesh(*ref)));
	}
}

void VBDManager::DrawUI() {
	ImGui::Begin("VBD Solver");

	if (ImGui::Button("Bake")) {
		Bake();
	}

	float g = solver.g.y;
	if (ImGui::SliderFloat("Gravity", &g, -3, 3, "%.2f")) {
		solver.g = vec3(0, g, 0);
	}

	ImGui::SliderFloat("Mass", &solver.m, 0.05f, 10.0f, "%.2f");
	ImGui::SliderFloat("Spring Constant", &solver.k, 1.0f, 1000.0f, "%.1f");
	ImGui::SliderFloat("Rest Length", &solver.restLen, 0.01f, 2.0f, "%.2f");

	ImGui::End();
}