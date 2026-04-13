#pragma once

#include "defines.h"
#include "half-edge.h"
#include <iostream>

// constexpr float dt = 1.0f / 48.0f; // TODO: make based on fps or an input

class VBDSolver {
public:
	VBDSolver();

	void ResetSimulation(uPtr<HalfEdgeMesh> newStartPoseMesh = nullptr);
	void SimulateUpToFrame(uint frameIndex);
	uPtr<HalfEdgeMesh> lastSimulatedMesh;

private:
	friend class VBDManager;

	uPtr<HalfEdgeMesh> startPoseMesh;
	int lastSimulatedFrame;

	void SimulateOneFrame();
	vec3 PredictPosition(HVertex* vert, vec3 externalPos);

	int iterCount = 5;
	float dt = 1.0f / 24.0f;

	vec3 g = vec3(0.0f, -0.98f, 0.0f);
	float m = 1.0f;
	float k = 150.0f;
	float restLen = 0.3;
};