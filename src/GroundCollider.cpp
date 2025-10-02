#include <glm/glm.hpp>
#include "Ground.h"
#include "GroundCollider.h"
#include "Vectors.h"
#include "Display.h"


GroundCollider::GroundCollider() {
	Vec3 groundPos(-5, -10, 3);
	Vec2 groundSize(20, 15);
	glm::vec4 groundColor(0.3f, 0.3f, 0.3f, 0.4f);

	ground = new Ground(groundPos, groundSize, groundColor);
	renderer = new GroundRender(ground);
}

GroundCollider::~GroundCollider() {
	delete ground;
	delete renderer;
}

void GroundCollider::resolveCollision(ClothData* data) {
#pragma omp parallel for
	for (int i = 0; i < data->nodes.size(); ++i) {
		if (data->getWorldPos(data->nodes[i]).y < ground->position.y) {
			data->nodes[i]->position.y = ground->position.y - data->clothPos.y + 0.01;
			data->nodes[i]->velocity = data->nodes[i]->velocity * ground->friction;
		}
	}
}

void GroundCollider::render() {
	renderer->flush();
}