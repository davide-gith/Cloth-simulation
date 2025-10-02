#ifndef SPHERE_COLLIDER_H
#define SPHERE_COLLIDER_H

#include <glm/glm.hpp>
#include "ICollider.h"
#include "Vectors.h"
#include "Sphere.h"

class SphereRender;

class SphereCollider : public ICollider {
public:
    Vec3 center;
    float radius;
    glm::vec4 color;

    Sphere* sphere;
	SphereRender* renderer;

    SphereCollider();
    SphereCollider(Vec3 center);
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~SphereCollider();
};

#endif