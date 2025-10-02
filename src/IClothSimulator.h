#ifndef ICLOTHSIMULATOR_H
#define ICLOTHSIMULATOR_H

#include "ICollider.h"
#include <memory>

class IClothSimulator {
public:
    virtual void update() = 0;
	virtual void unpin() = 0;
    virtual std::vector<ICollider*> getColliders() = 0;
    virtual void addCollider(ICollider* col) = 0;
    virtual ~IClothSimulator() {}
};

#endif