#ifndef SYMPLECTIC_EULER_SIMULATOR_H
#define SYMPLECTIC_EULER_SIMULATOR_H

#include <memory>
#include "ClothData.h"
#include "IClothSimulator.h"
#include "Vectors.h"
#include "ICollider.h"

class SymplecticEulerIntegrator : public IClothSimulator {
public:
	SymplecticEulerIntegrator(ClothData* data);
	void update() override;
	void unpin() override;
	std::vector<ICollider*> getColliders() override;
	void addCollider(ICollider* col) override;

private:
	ClothData* cloth;
	std::vector<ICollider*> colliders;
	Vec3 gravity;
	int iterationFreq;
	double AIR_FRICTION;
	double TIME_STEP;
	double stretchingCoef;
	double bendingCoef;
	double shearCoef;

	void initCoeff();
	void removeZeroCoeffSpring();
	void computeForce(double timeStep, Vec3 gravity);
	void integrate(double airFriction, double timeStep);
};

#endif