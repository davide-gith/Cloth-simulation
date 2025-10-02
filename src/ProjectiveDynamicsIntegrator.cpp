#include <iostream>
#include <iomanip>

#include "ProjectiveDynamicsIntegrator.h"
#include "ClothData.h"
#include "Vectors.h"
#include <Eigen/Dense>

using namespace std;

ProjectiveDynamicsIntegrator::ProjectiveDynamicsIntegrator(ClothData* data)
    : cloth(data),
    LHS(data->nodes.size() * 3, data->nodes.size() * 3),
    M(data->nodes.size() * 3, data->nodes.size() * 3),
    x_new(data->nodes.size() * 3),
    b(data->nodes.size() * 3),
    s_t(data->nodes.size() * 3)
{
    TIME_STEP = 0.01;
    stretchingCoef = 10000.0f;
    bendingCoef = 4000.0f;
    shearCoef = 500.0;
    gravity = Vec3(0.0, -9.8, 0.0);

    maxIterations =1;

    collision_stiffness = 10000.0f;
   
    initCoeff();
    initVars();
    removeZeroCoeffSpring();
}

void ProjectiveDynamicsIntegrator::initVars() {

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        cloth->nodes[i]->residual_G = Vec3(0.0, 0.0, 0.0);
    }

    // Initialize mass matrix M
    std::vector<Eigen::Triplet<double>> triplets;
    
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            int idx = i * 3 + j;
            triplets.push_back(Eigen::Triplet<double>(idx, idx, cloth->nodes[i]->mass));
        }
    }

    M.setFromTriplets(triplets.begin(), triplets.end());

    // Initialize Left Hand Side LHS
    triplets.clear();
    
    for (int k = 0; k < cloth->springs.size(); ++k) {
        for (int i = 0; i < 3; ++i) {
            
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node1->id*3 + i, cloth->springs[k]->node1->id * 3 + i, cloth->springs[k]->hookCoef));

                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node1->id * 3 + i, cloth->springs[k]->node2->id * 3 + i, -cloth->springs[k]->hookCoef));
                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node2->id * 3 + i, cloth->springs[k]->node1->id * 3 + i, -cloth->springs[k]->hookCoef));

                triplets.push_back(Eigen::Triplet<double>(cloth->springs[k]->node2->id * 3 + i, cloth->springs[k]->node2->id * 3 + i, cloth->springs[k]->hookCoef));
            
        }
    }      

    LHS.setFromTriplets(triplets.begin(), triplets.end());
	LHS += M / TIME_STEP / TIME_STEP;
	
    solver.analyzePattern(LHS);
	solver.compute(LHS);

}

void ProjectiveDynamicsIntegrator::initCoeff() {


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

void ProjectiveDynamicsIntegrator::removeZeroCoeffSpring() {
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

void ProjectiveDynamicsIntegrator::unpin() {
    int unpinnedCount = 0;

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->isFixed = false;
            unpinnedCount++;
        }
    }

}

void ProjectiveDynamicsIntegrator::update() {
    // Iteration loop
    for (int iter = 0; iter < maxIterations; iter++) {

        for (int i = 0; i < cloth->nodes.size(); i++) {
            cloth->nodes[i]->old_position = cloth->nodes[i]->position;
        }

        // Compute forces
        computeForce(TIME_STEP, gravity);

        // Compute inertia
        computeInertia(TIME_STEP);

        // Integrate and solve
        integrate(TIME_STEP);
    }
}

void ProjectiveDynamicsIntegrator::computeInertia(double timeStep)
{
    // Reset forces
    for (int i = 0; i < cloth->nodes.size(); i++) {
        cloth->nodes[i]->inertia=Vec3(0.0, 0.0, 0.0);
    }
    /** Nodes **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {

        cloth->nodes[i]->inertia= cloth->nodes[i]->position
                                + cloth->nodes[i]->velocity * timeStep
			                    + cloth->nodes[i]->force * cloth->nodes[i]->w * timeStep * timeStep;

                
    }
}
std::vector<ICollider*> ProjectiveDynamicsIntegrator::getColliders() {
    return colliders;
}

void ProjectiveDynamicsIntegrator::addCollider(ICollider* col) {
    colliders.push_back(col);
}

void ProjectiveDynamicsIntegrator::computeForce(double timeStep, Vec3 gravity)
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

}

void ProjectiveDynamicsIntegrator::integrate(double timeStep)
{


    /** Nodes **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {
        x_new(i * 3 + 0) = cloth->nodes[i]->inertia.x;
        x_new(i * 3 + 1) = cloth->nodes[i]->inertia.y;
        x_new(i * 3 + 2) = cloth->nodes[i]->inertia.z;

        s_t(i * 3 + 0) = cloth->nodes[i]->inertia.x * cloth->nodes[i]->mass / (timeStep * timeStep);
        s_t(i * 3 + 1) = cloth->nodes[i]->inertia.y * cloth->nodes[i]->mass / (timeStep * timeStep);
        s_t(i * 3 + 2) = cloth->nodes[i]->inertia.z * cloth->nodes[i]->mass / (timeStep * timeStep);

    }
    
    for (int i = 0; i < maxIterations; i++) {
        b = s_t;
	
        for (int j = 0; j < cloth->springs.size(); j++) {
            int idx1 = cloth->springs[j]->node1->id * 3;
			int idx2 = cloth->springs[j]->node2->id * 3;


            Vec3 p1 = Vec3(x_new(idx1 + 0), x_new(idx1 + 1), x_new(idx1 + 2));
            Vec3 p2 = Vec3(x_new(idx2 + 0), x_new(idx2 + 1), x_new(idx2 + 2));
			
            Vec3 diffPos = p1 - p2;
            double currLen = diffPos.length();
            Vec3 unit_vec = diffPos / currLen;

            double stretch = currLen - cloth->springs[j]->restLen;

            Vec3 correction = unit_vec * (stretch / 2.0);

            Vec3 p_c1 = p1 - correction;
            Vec3 p_c2 = p2 + correction;
       
            b[idx1 + 0] += cloth->springs[j]->hookCoef * (p_c1.x - p_c2.x);
            b[idx1 + 1] += cloth->springs[j]->hookCoef * (p_c1.y - p_c2.y);
            b[idx1 + 2] += cloth->springs[j]->hookCoef * (p_c1.z - p_c2.z);


            b[idx2 + 0] += cloth->springs[j]->hookCoef * (p_c2.x - p_c1.x);
            b[idx2 + 1] += cloth->springs[j]->hookCoef * (p_c2.y - p_c1.y);
            b[idx2 + 2] += cloth->springs[j]->hookCoef * (p_c2.z - p_c1.z);

        }

        for (auto collider : colliders) {
            if (collider) {
                collider->resolveCollision(cloth);
            }
		}
        
#pragma omp parallel for
        for (int i = 0; i < cloth->nodes.size(); i++) {
            
            if (cloth->nodes[i]->position != cloth->nodes[i]->old_position) {
                int idx = i * 3;
                //toChange.push_back(idx);
                b[idx + 0] += (cloth->nodes[i]->position.x - cloth->nodes[i]->old_position.x) * collision_stiffness;
                b[idx + 1] += (cloth->nodes[i]->position.y - cloth->nodes[i]->old_position.y) * collision_stiffness;
                b[idx + 2] += (cloth->nodes[i]->position.z - cloth->nodes[i]->old_position.z) * collision_stiffness;
            }
        }

        /*
        if (toChange.size() > 0) {
//#pragma omp parallel for
            for (int k = 0; k < toChange.size(); ++k) {
                int idx = toChange[k];
                LHS.coeffRef(idx + 0, idx + 0) += collision_stiffness;
                LHS.coeffRef(idx + 1, idx + 1) += collision_stiffness;
                LHS.coeffRef(idx + 2, idx + 2) += collision_stiffness;
            }
        
            solver.analyzePattern(LHS);
            solver.compute(LHS);
        }
        */

        x_new = solver.solve(b);

    }

#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (!cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->position = Vec3(x_new[i * 3 + 0], x_new[i * 3 + 1], x_new[i * 3 + 2]);
            cloth->nodes[i]->velocity = (cloth->nodes[i]->position - cloth->nodes[i]->old_position) / TIME_STEP;
        }
    }
    /*
    if (toChange.size() > 0) {
//#pragma omp parallel for
        for (int k = 0; k < toChange.size(); ++k) {
            int idx = toChange[k];
            LHS.coeffRef(idx + 0, idx + 0) -= collision_stiffness;
            LHS.coeffRef(idx + 1, idx + 1) -= collision_stiffness;
            LHS.coeffRef(idx + 2, idx + 2) -= collision_stiffness;
        }

        solver.analyzePattern(LHS);
        solver.compute(LHS);
        toChange.clear();
        
    }
    */
   
    
    /* attaching pbd collision resolver
    for (auto* collider : colliders) {
        if (collider)
            collider->resolveCollision(cloth);
    }
    */
}
