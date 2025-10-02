#include <iostream>

#include "SymplecticEulerIntegrator.h"
#include "ClothData.h"
#include "Vectors.h"

SymplecticEulerIntegrator::SymplecticEulerIntegrator(ClothData* data) : cloth(data) {
    AIR_FRICTION = 0.02;
    TIME_STEP = 0.01;
    stretchingCoef = 1000.0f;
    bendingCoef = 400.0f;
    shearCoef = 50.0;
    iterationFreq = 25;
    gravity = Vec3(0.0, -9.8 / iterationFreq, 0.0);

    std::cout << "Method: " << std::endl;
    std::cout << "- Symplectic Euler" << std::endl;

    initCoeff();
    removeZeroCoeffSpring();
}

void SymplecticEulerIntegrator::initCoeff() {
    for (int s = 0; s < cloth->springs.size(); ++s) {
        if (cloth->springs[s]->id == 0) {
            cloth->springs[s]->hookCoef = stretchingCoef;
        }
        else if (cloth->springs[s]->id == 1 || cloth->springs[s]->id == 2) {
            cloth->springs[s]->hookCoef = shearCoef;
        }
        else if (cloth->springs[s]->id == 3) {
            cloth->springs[s]->hookCoef = bendingCoef;
        }
    }
}

void SymplecticEulerIntegrator::removeZeroCoeffSpring() {
    // Remove springs with zero coefficient
    for (int i = 0; i < cloth->springs.size(); i++) {
        if (cloth->springs[i]->hookCoef == 0.0) {
            delete cloth->springs[i];
            cloth->springs.erase(cloth->springs.begin() + i);
            i--;
        }
    }
}

void SymplecticEulerIntegrator::unpin() {
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->isFixed = false;
        }
    }
}

void SymplecticEulerIntegrator::update() {
    for (int i = 0; i < iterationFreq; ++i) {
        computeForce(TIME_STEP, gravity);
        integrate(AIR_FRICTION, TIME_STEP);
        for (auto* collider : colliders) {
            if (collider)
                collider->resolveCollision(cloth);
        }
        //std::fill(cloth->IsCollided.begin(), cloth->IsCollided.end(), false);
    }
}

void SymplecticEulerIntegrator::computeForce(double timeStep, Vec3 gravity)
{
    /** Nodes **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {
        cloth->nodes[i]->addForce(gravity * cloth->nodes[i]->mass);
    }
    /** Springs **/
#pragma omp parallel for
    for (int i = 0; i < cloth->springs.size(); i++)
    {
        cloth->springs[i]->applyInternalForce(timeStep);
    }
}

void SymplecticEulerIntegrator::integrate(double airFriction, double timeStep)
{
    /** Node **/
#pragma omp parallel for
    for (int i = 0; i < cloth->nodes.size(); i++)
    {
        cloth->nodes[i]->SimpletticIntegrate(timeStep);
    }
}

std::vector<ICollider*> SymplecticEulerIntegrator::getColliders() {
    return colliders;
}

void SymplecticEulerIntegrator::addCollider(ICollider* col) {
    colliders.push_back(col);
}