#include <glm/glm.hpp>

#include "CapsuleCollider.h"
#include "Capsule.h"
#include "Vectors.h"
#include "Display.h"

CapsuleCollider::CapsuleCollider() {
	//center1 = Vec3(0.0f, -1.0f, -5.0f)
	//center2 = Vec3(6.0f, 1.0f, -5.0f);
	top = Vec3(5.0, 2.0, -4.0);
	bottom = Vec3(5.0, -8.0, -4.0);
	radius = 2;
	color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	capsule = new Capsule(top, bottom, radius, color);
	renderer = new CapsuleRender(capsule);
}

CapsuleCollider::CapsuleCollider(Vec3 top, Vec3 bottom) : top(top), bottom(bottom) {
	radius = 2;
	color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	capsule = new Capsule(top, bottom, radius, color);
	renderer = new CapsuleRender(capsule);
}

CapsuleCollider::~CapsuleCollider() {
	delete capsule;
	delete renderer;
}

void CapsuleCollider::resolveCollision(ClothData* data) {
#pragma omp parallel for
	for (int i = 0; i < data->nodes.size(); ++i) {
		// point
		Vec3 P = data->getWorldPos(data->nodes[i]);
		

		// top -> point
		Vec3 AP = P - capsule->centerTopHemisphere;

		// top -> bottom
		Vec3 AB = capsule->centerBottomHemisphere - capsule->centerTopHemisphere;

		double F = Vec3::dot(AP, AB) / Vec3::dot(AB, AB);

		// closest point
		Vec3 C = capsule->centerTopHemisphere + AB * (double)glm::clamp(F, 0.0, 1.0);

		Vec3 distVec = P - C;
		double dist = distVec.length();
		float safeDist = radius * 1.05;

		double distLenSemiSphereTop = (C - capsule->centerTopHemisphere).length() < radius ? (C - P).length() : radius;
		double distLenSemiSphereBottom = (C - bottom).length() < radius ? (C - P).length() : radius;
		if (distLenSemiSphereTop < radius || distLenSemiSphereBottom < radius) {
			safeDist = distLenSemiSphereTop < distLenSemiSphereBottom ? distLenSemiSphereTop : distLenSemiSphereBottom * 1.05;
		}

		if (dist < safeDist) {
			distVec.normalize();
			data->setWorldPos(data->nodes[i], distVec * safeDist + C);
			data->nodes[i]->velocity = data->nodes[i]->velocity * capsule->friction;
		}

	}
}

void CapsuleCollider::render() {
	renderer->flush();
}