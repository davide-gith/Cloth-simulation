#include <glm/glm.hpp>
#include "Box.h"
#include "AABBCollider.h"
#include "Vectors.h"
#include "Display.h"


AABBCollider::AABBCollider() {
	//center = Vec3(4.0, -2.0, -1.0);
	center = Vec3(5.0, 0.0, -3.0);
	halfExtents = Vec3(2.0, 2.0, 2.0);
	glm::vec4 color(1.0f, 0.647f, 0.0f, 1.0f);

	box = new Box(center, halfExtents, color);
	renderer = new BoxRender(box);
}

AABBCollider::AABBCollider(Vec3 center) : center(center) {
	halfExtents = Vec3(2.0, 2.0, 2.0);
	glm::vec4 color(1.0f, 0.647f, 0.0f, 1.0f);

	box = new Box(center, halfExtents, color);
	renderer = new BoxRender(box);
}

AABBCollider::~AABBCollider() {
	delete box;
	delete renderer;
}

void AABBCollider::resolveCollision(ClothData* data) {

	float minX = center.x - halfExtents.x;
	float maxX = center.x + halfExtents.x;
	float minY = center.y - halfExtents.y;
	float maxY = center.y + halfExtents.y;
	float minZ = center.z - halfExtents.z;
	float maxZ = center.z + halfExtents.z;

	struct FaceInfo {
		float distance;
		Vec3 normal;
	};

	float epsilon = 0.15f;

#pragma omp parallel for
	for (int i = 0; i < data->nodes.size(); ++i) {
		Vec3 P = data->getWorldPos(data->nodes[i]);

		if (!(P.x > minX - epsilon && P.x < maxX + epsilon &&
			P.y > minY - epsilon && P.y < maxY + epsilon &&
			P.z > minZ - epsilon && P.z < maxZ + epsilon)) {
			continue; // Node is outside the AABB
		}


		FaceInfo candidates[6] = {
			{ std::abs(P.x - minX), Vec3(-1, 0, 0) }, // x-
			{ std::abs(maxX - P.x), Vec3(1, 0, 0) },  // x+
			{ std::abs(P.y - minY), Vec3(0, -1, 0) }, // y-
			{ std::abs(maxY - P.y), Vec3(0, 1, 0) },  // y+
			{ std::abs(P.z - minZ), Vec3(0, 0, -1) }, // z-
			{ std::abs(maxZ - P.z), Vec3(0, 0, 1) }   // z+
		};

		FaceInfo closest = candidates[0];
		for (int j = 1; j < 6; j++) {
			if (candidates[j].distance < closest.distance) {
				closest = candidates[j];
			}
		}

		data->setWorldPos(data->nodes[i], P + closest.normal * (closest.distance * 1.05));
		data->nodes[i]->velocity = data->nodes[i]->velocity * box->friction;
	}
}


void AABBCollider::render() {
	renderer->flush();
}