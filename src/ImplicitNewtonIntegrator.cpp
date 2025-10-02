#include <iostream>
#include <iomanip>


#include "ImplicitNewtonIntegrator.h"
#include "ClothData.h"
#include "Vectors.h"
#include <Eigen/Dense>

using namespace std;

ImplicitNewtonIntegrator::ImplicitNewtonIntegrator(ClothData* data)
    : cloth(data),
    J(data->nodes.size() * 3, data->nodes.size() * 3),
    M(data->nodes.size() * 3, data->nodes.size() * 3),
    G(data->nodes.size() * 3)
{
    TIME_STEP = 0.01;
    stretchingCoef = 1000.0f;
    bendingCoef = 400.0f;
    shearCoef = 50.0;
    gravity = Vec3(0.0, -9.8, 0.0);

    maxIterations = 1;
    convergenceTol = 1e-5;

    initCoeff();
    initVars();
    removeZeroCoeffSpring();


}
std::vector<ICollider*> ImplicitNewtonIntegrator::getColliders() {
    return colliders;
}

void ImplicitNewtonIntegrator::addCollider(ICollider* col) {
    colliders.push_back(col);
}

void ImplicitNewtonIntegrator::initVars() {

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        cloth->nodes[i]->residual_G = Vec3(0.0, 0.0, 0.0);
    }

	// Initialize jacobian matrix J
    std::vector<Eigen::Triplet<double>> triplets;
    for (int k = 0;k < cloth->springs.size(); ++k) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node1->id + i, cloth->springs[k]->node1->id + j, 1.0));
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node1->id + i, cloth->springs[k]->node2->id + j, 1.0));
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node2->id + i, cloth->springs[k]->node1->id + j, 1.0));
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node2->id + i, cloth->springs[k]->node2->id + j, 1.0));
            }
        }
    }
    J.setFromTriplets(triplets.begin(), triplets.end());
    solver.analyzePattern(J);

	// Initialize mass matrix M
	triplets.clear();

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            int idx = i * 3 + j;
            triplets.push_back(Eigen::Triplet<double>(idx, idx, cloth->nodes[i]->mass));
        }
    }

    M.setFromTriplets(triplets.begin(), triplets.end());
}

void ImplicitNewtonIntegrator::initCoeff() {
  

    int stretchCount = 0, shearCount = 0, bendingCount = 0, unknownCount = 0;

    //0: quad springs
    //1: first diagonal spring (\)
    //2: second diagonal spring (/)
    //3: quad springs

    for (int s = 0; s < cloth->springs.size(); ++s) {
        if (cloth->springs[s]->id == 0) {
            cloth->springs[s]->hookCoef = stretchingCoef;
            stretchCount++;
        }
        else if (cloth->springs[s]->id == 1 || cloth->springs[s]->id == 2) {
            cloth->springs[s]->hookCoef = shearCoef;
            shearCount++;
        }
        else if (cloth->springs[s]->id == 3) {
            cloth->springs[s]->hookCoef = bendingCoef;
            bendingCount++;
        }
        else {
            unknownCount++;
        }
    }
}

void ImplicitNewtonIntegrator::removeZeroCoeffSpring() {
    int removedCount = 0;


    // Remove springs with zero coefficient
    for (int i = 0; i < cloth->springs.size(); i++) {
        if (cloth->springs[i]->hookCoef == 0.0) {
            delete cloth->springs[i];
            cloth->springs.erase(cloth->springs.begin() + i);
            i--;
            removedCount++;
        }
    }

}

void ImplicitNewtonIntegrator::unpin() {
    int unpinnedCount = 0;

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->isFixed = false;
            unpinnedCount++;
        }
    }

}

void ImplicitNewtonIntegrator::update() {
    double G_norm = 1.0;

    // Newton iteration loop
    for (int iter = 0; iter < maxIterations && G_norm > convergenceTol; iter++) {

        for (int i = 0; i < cloth->nodes.size(); i++) {
            cloth->nodes[i]->old_position = cloth->nodes[i]->position;
        }

        // Compute forces
        computeForce(TIME_STEP, gravity);

        // Integrate and solve
        integrate(TIME_STEP);
    }
}

void ImplicitNewtonIntegrator::computeForce(double timeStep, Vec3 gravity)
{
    // Reset forces
    for (int i = 0; i < cloth->nodes.size(); i++) {
        cloth->nodes[i]->force = Vec3(0.0, 0.0, 0.0);
    }

    /** Nodes **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {
        cloth->nodes[i]->addForce(gravity * cloth->nodes[i]->mass);
    }

    /** Springs **/
//#pragma omp parallel for
    for (int i = 0; i < cloth->springs.size(); i++)
    {
        cloth->springs[i]->applyInternalForce(timeStep);
    }

}

void ImplicitNewtonIntegrator::integrate(double timeStep)
{
    /** Nodes **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {
        cloth->nodes[i]->residualUpdate(timeStep);
    }

    /** Springs **/
#pragma omp parallel for
    for (int i = 0; i < cloth->springs.size(); i++)
    {
        cloth->springs[i]->jacobianUpdate(timeStep);
    }

    // Solve the linear system J * deltaX = -G
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        G[i * 3 + 0] = cloth->nodes[i]->residual_G.x;
        G[i * 3 + 1] = cloth->nodes[i]->residual_G.y;
        G[i * 3 + 2] = cloth->nodes[i]->residual_G.z;
    }

    J.setZero();

    for (int k = 0; k < cloth->springs.size(); ++k) {
        auto* spring = cloth->springs[k];
        unsigned int id1 = spring->node1->id;
        unsigned int id2 = spring->node2->id;

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                double val = spring->jacobianBlock[i * 3 + j];
                J.coeffRef(id1*3 + i, id1*3 + j) += val;
                J.coeffRef(id1*3 + i, id2*3 + j) += -val;
                J.coeffRef(id2*3 + i, id1*3 + j) += -val;
                J.coeffRef(id2*3+ i, id2*3 + j) += val;
            }
        }
    }

    J += M;


    //solver.analyzePattern(J);
    solver.compute(J);

    delta_x = solver.solve(-G);

    // Update positions
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (!cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->position += Vec3(delta_x[i * 3], delta_x[i * 3 + 1], delta_x[i * 3 + 2]);
            cloth->nodes[i]->velocity = (cloth->nodes[i]->position - cloth->nodes[i]->old_position) / TIME_STEP;
        }
    }

	//Handling collisions
    for (auto collider : colliders) {
        if (collider) {
            collider->resolveCollision(cloth);
        }
    }

}