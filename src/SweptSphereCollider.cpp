#include <glm/glm.hpp>
#include "SweptSphere.h"
#include "SweptSphereCollider.h"
#include "Vectors.h"
#include "Display.h"

SweptSphereCollider::SweptSphereCollider() {
	center1 = Vec3(0.0f, -1.0f, -5.0f);
	radius1 = 1.0f;
	center2 = Vec3(6.0f, 1.0f, -5.0f);
	radius2 = 2.0f;
	color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

	sweptsphere = new SweptSphere(center1, radius1, center2, radius2, color, 64, 32, 24);
	renderer = new SweptSphereRender(sweptsphere);

}

SweptSphereCollider::SweptSphereCollider(Vec3 c1, float r1, Vec3 c2, float r2) : center1(c1), radius1(r1), center2(c2), radius2(r2) {
	color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

	sweptsphere = new SweptSphere(center1, radius1, center2, radius2, color, 16, 8, 6);
	renderer = new SweptSphereRender(sweptsphere);
}

SweptSphereCollider::~SweptSphereCollider() {
	delete sweptsphere;
	delete renderer;
}

void SweptSphereCollider::resolveCollision(ClothData* data) {
#pragma omp parallel for
	for (int i = 0; i < data->nodes.size(); ++i) {
		
		Vec3 c1c2 = center2 - center1;
		double distc1c2 = c1c2.length();

		Vec3 distVec = data->getWorldPos(data->nodes[i]) - center1;
		double distLen = distVec.length();
		double safeDist = radius1 * 1.05;
		if (distLen < safeDist) {
			distVec.normalize();
			data->setWorldPos(data->nodes[i], distVec * safeDist + center1);
			data->nodes[i]->velocity = data->nodes[i]->velocity * sweptsphere->friction;
			continue;
		}
		
		distVec = data->getWorldPos(data->nodes[i]) - center2;
		distLen = distVec.length();
		safeDist = radius2 * 1.05;
		if (distLen < safeDist) {
			distVec.normalize();
			data->setWorldPos(data->nodes[i], distVec * safeDist + center2);
			data->nodes[i]->velocity = data->nodes[i]->velocity * sweptsphere->friction;
			continue;
		}
		
		Vec3 c1P = data->getWorldPos(data->nodes[i]) - center1;
		double diffr1r2 = radius2 - radius1;
		double t = (Vec3::dot(c1P, c1c2) + diffr1r2 * radius1) / ((distc1c2 * distc1c2) - (diffr1r2 * diffr1r2));
		t = (t < 0.0) ? 0.0 : (t > 1.0) ? 1.0 : t;
		Vec3 center = center1 + c1c2 * t;
		double radiusLen = radius1 + diffr1r2 * t;
		distVec = data->getWorldPos(data->nodes[i]) - center;
		distLen = distVec.length();
		safeDist = radiusLen * 1.05;
		if (distLen < safeDist) {
			distVec.normalize();
			data->setWorldPos(data->nodes[i], distVec * safeDist + center);
			data->nodes[i]->velocity = data->nodes[i]->velocity * sweptsphere->friction;
		}
	}
}

void SweptSphereCollider::render() {
	renderer->flush();
}