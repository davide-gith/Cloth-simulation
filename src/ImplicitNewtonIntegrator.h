#ifndef IMPLICIT_NEWTON_SIMULATOR_H
#define IMPLICIT_NEWTON_SIMULATOR_H

#include "ClothData.h"
#include "IClothSimulator.h"
#include "Vectors.h"
#include <Eigen/Sparse>

class ImplicitNewtonIntegrator : public IClothSimulator {
public:
    ImplicitNewtonIntegrator(ClothData* data);
    void update() override;
	void unpin() override;
	std::vector<ICollider*> getColliders() override;
	void addCollider(ICollider* col) override;

private:
    ClothData* cloth;
	std::vector<ICollider*> colliders;
    Vec3 gravity;
	double TIME_STEP;
	double stretchingCoef;
	double bendingCoef;
	double shearCoef;

	int maxIterations;
	double convergenceTol;
	
	Eigen::SparseMatrix<double> J;
	Eigen::SparseMatrix<double> M;
	Eigen::VectorXd G;
	Eigen::SimplicialLDLT < Eigen::SparseMatrix<double >> solver;
	Eigen::VectorXd delta_x;

	void initCoeff();
	void initVars();
	void removeZeroCoeffSpring();
	void computeForce(double timeStep, Vec3 gravity);
	void integrate(double timeStep);
};

#endif