#include <glm/glm.hpp>
#include "Sphere.h"
#include "SphereCollider.h"
#include "Vectors.h"
#include "Display.h"


SphereCollider::SphereCollider() {
	//center = Vec3(5.0, 1.0, 0.0);
	center = Vec3(5.0, 0.0, -3.0);
	//center = Vec3(0.0, 0.0, 0.0);
	//radius = 0.2;
	radius = 2.0;
	color = glm::vec4(0.1f, 0.9f, 0.1f, 1.0f);
	sphere = new Sphere(center, radius, color);
	renderer = new SphereRender(sphere);
}

SphereCollider::~SphereCollider() {
	delete sphere;
	delete renderer;
}

SphereCollider::SphereCollider(Vec3 center) : center(center){
	//center = Vec3(5.0, 1.0, 0.0);
	radius = 2;
	color = glm::vec4(0.1f, 0.9f, 0.1f, 1.0f);
	sphere = new Sphere(center, radius, color);
	renderer = new SphereRender(sphere);
}

void SphereCollider::resolveCollision(ClothData* data) {
#pragma omp parallel for
	for (int i = 0; i < data->nodes.size(); ++i) {
		Vec3 distVec = data->getWorldPos(data->nodes[i]) - center;
		double distLen = distVec.length();
		double safeDist = radius * 1.15;
		if (distLen < safeDist) {
			distVec.normalize();
			data->setWorldPos(data->nodes[i], distVec * safeDist + center);
			data->nodes[i]->velocity = data->nodes[i]->velocity * sphere->friction;
		}
	}
}

void SphereCollider::render() {
	renderer->flush();
}