#ifndef AABB_COLLIDER_H
#define AABB_COLLIDER_H

#include "Box.h"
#include <glm/glm.hpp>
#include "ICollider.h"
#include "Vectors.h"

class BoxRender;

class AABBCollider : public ICollider {
public:
    Vec3 center;
    Vec3 halfExtents;
    glm::vec4 color;

    Box* box;
	BoxRender* renderer;

    AABBCollider();
    AABBCollider(Vec3 center);
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~AABBCollider();
};

#endif