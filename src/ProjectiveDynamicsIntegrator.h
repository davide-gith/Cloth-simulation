#ifndef PROJECIVE_DYNAMICS_INTEGRATOR_H
#define PROJECIVE_DYNAMICS_INTEGRATOR_H

#include "ClothData.h"
#include "IClothSimulator.h"
#include "Vectors.h"
#include <Eigen/Sparse>

class ProjectiveDynamicsIntegrator : public IClothSimulator {
public:
	ProjectiveDynamicsIntegrator(ClothData* data);
	void update() override;
	void unpin() override;
	std::vector<ICollider*> getColliders() override;
	void addCollider(ICollider* col) override;

private:
	ClothData* cloth;
	std::vector<ICollider*> colliders;
	std::string method;
	Vec3 gravity;
	int iterationFreq;
	double TIME_STEP;
	double stretchingCoef;
	double bendingCoef;
	double shearCoef;

	int maxIterations;

	double collision_stiffness;
	std::vector<int> toChange;

	Eigen::SparseMatrix<double> LHS;
	Eigen::SparseMatrix<double> M;
	Eigen::SimplicialLLT < Eigen::SparseMatrix<double >> solver;
	
	Eigen::VectorXd x_new;
	Eigen::VectorXd b;
	Eigen::VectorXd s_t;
	
	void initCoeff();
	void initVars();
	void removeZeroCoeffSpring();
	void computeForce(double timeStep, Vec3 gravity);
	void computeInertia(double timeStep);
	void integrate(double timeStep);
};

#endif