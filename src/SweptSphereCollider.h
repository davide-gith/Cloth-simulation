#ifndef SWEPT_SPHERE_COLLIDER_H
#define SWEPT_SPHERE_COLLIDER_H

#include <glm/glm.hpp>
#include "ICollider.h"
#include "Vectors.h"
#include "Display.h"
#include "SweptSphere.h"

class SweptSphereRender;

class SweptSphereCollider : public ICollider {
public:
    Vec3 center1;
    Vec3 center2;
    float radius1;
    float radius2;
    glm::vec4 color;

    SweptSphere* sweptsphere;
    SweptSphereRender* renderer;

    SweptSphereCollider();
    SweptSphereCollider(Vec3 c1, float r1, Vec3 c2, float r2);
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~SweptSphereCollider();
};

#endif