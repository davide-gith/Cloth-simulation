#ifndef POSITION_NBASED_INTEGRATOR_H
#define POSITION_NBASED_INTEGRATOR_H

#include <string>
#include <memory>
#include "ClothData.h"
#include "IClothSimulator.h"
#include "Vectors.h"
#include "ICollider.h"

class PositionBasedIntegrator : public IClothSimulator {
public:
	PositionBasedIntegrator(ClothData* data, std::string method);
	void update() override;
	void unpin() override;
	std::vector<ICollider*> getColliders() override;
	void addCollider(ICollider* col) override;

private:
	ClothData* cloth;
	std::vector<ICollider*> colliders;
	std::string method;
	Vec3 gravity;
	double iterationFreq;
	double TIME_STEP;
	double stretchingCoef; // stretching compliance
	double bendingCoef; // bending compliance

	void initCoeff();
	void removeAdditionalSpring();

	void predict(double timeStep, Vec3 gravity);
	void solveConstraint(double timeStep);
	void updateVelocities(double timeStep);
};

#endif