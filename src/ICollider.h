#ifndef ICOLLIDER_H
#define ICOLLIDER_H

#include "ClothData.h"

class ICollider {
public:
	virtual void resolveCollision(ClothData* data) = 0; 
	virtual void render() = 0;
	virtual ~ICollider() = default;

};

#endif