#ifndef CAPSULE_COLLIDER_H
#define CAPSULE_COLLIDER_H

#include <glm/glm.hpp>
#include "ICollider.h"
#include "Capsule.h"
#include "Vectors.h"

class CapsuleRender;

class CapsuleCollider : public ICollider {
public:
    Vec3 top;
	Vec3 bottom;
    int radius;
    glm::vec4 color;

    Capsule* capsule;
	CapsuleRender* renderer;

    CapsuleCollider();
    CapsuleCollider(Vec3 top, Vec3 bottom);
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~CapsuleCollider();
};

#endif