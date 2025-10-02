#ifndef GROUND_COLLIDER_H
#define GROUND_COLLIDER_H

#include <glm/glm.hpp>
#include "ICollider.h"
#include "Vectors.h"
#include "Ground.h"

class GroundRender;

class GroundCollider : public ICollider {
public:
    Ground* ground;
	GroundRender* renderer;

    GroundCollider();
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~GroundCollider();
};

#endif