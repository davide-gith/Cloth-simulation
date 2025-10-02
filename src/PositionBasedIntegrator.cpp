#include <string>
#include <iostream>

#include "PositionBasedIntegrator.h"
#include "ClothData.h"
#include "Vectors.h"

PositionBasedIntegrator::PositionBasedIntegrator(ClothData* d, std::string m) : cloth(d), method(m) {
    stretchingCoef = 0.0001f;
    bendingCoef = 0.3f;
    gravity = Vec3(0.0, -9.8, 0.0);
    iterationFreq = 25.0;
    double simulation_time_step = 1.0f / 60.0f;
    TIME_STEP = simulation_time_step / iterationFreq;

    if (method == "PBD") {
        removeAdditionalSpring();
    }

    if (method == "XPBD") {
        initCoeff();
    }

    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->mass = std::numeric_limits<double>::infinity();
            cloth->nodes[i]->w = 0;

            cloth->nodes[i]->mass = std::numeric_limits<double>::infinity();
            cloth->nodes[i]->w = 0;
        }
    }

    std::cout << "Method: " << std::endl;
    std::cout << "- " << method << std::endl;
}

void PositionBasedIntegrator::initCoeff() {
    for (int s = 0; s < cloth->springs.size(); ++s) {
        if (cloth->springs[s]->id == 0) {
            cloth->springs[s]->hookCoef = stretchingCoef;
        }
        else if (cloth->springs[s]->id == 1 || cloth->springs[s]->id == 2) {
            cloth->springs[s]->hookCoef = bendingCoef;
        }
        else if (cloth->springs[s]->id == 3) {
            cloth->springs[s]->hookCoef = stretchingCoef;
        }
    }
}

void PositionBasedIntegrator::unpin() {
    for (int i = 0; i < cloth->nodes.size(); ++i) {
        if (cloth->nodes[i]->isFixed) {
            cloth->nodes[i]->isFixed = false;
            cloth->nodes[i]->mass = 1.0f;
            cloth->nodes[i]->w = 1.0f;
        }
    }
}

void PositionBasedIntegrator::removeAdditionalSpring() {
    for (int i = 0; i < cloth->springs.size(); i++) {
        if (cloth->springs[i]->id == 3) {
            delete cloth->springs[i];
            cloth->springs.erase(cloth->springs.begin() + i);
            i--;
        }
    }
}

void PositionBasedIntegrator::predict(double timeStep, Vec3 gravity) {
#pragma omp parallel for
    for (int n = 0; n < cloth->nodes.size(); n++) {
        if (cloth->nodes[n]->w == 0.f)
            continue;

        cloth->nodes[n]->velocity += gravity * timeStep;
        cloth->nodes[n]->old_position = cloth->nodes[n]->position;
        cloth->nodes[n]->position += cloth->nodes[n]->velocity * timeStep;
    }
}

void PositionBasedIntegrator::solveConstraint(double timeStep) {
#pragma omp parallel for
    for (int c = 0; c < cloth->springs.size(); c++) {
        float alpha = cloth->springs[c]->hookCoef / timeStep / timeStep;
        if ((cloth->springs[c]->node1->w + cloth->springs[c]->node2->w) == 0.f)
            continue;

        Vec3 distance = cloth->springs[c]->node1->position - cloth->springs[c]->node2->position;
        double distance_lenght = distance.length();

        if (distance_lenght == 0.0f)
            continue;

        distance.normalize();
        double rest_length = cloth->springs[c]->restLen;
        double error = distance_lenght - rest_length;
        double correction = -error / (cloth->springs[c]->node1->w + cloth->springs[c]->node2->w + alpha);
        double first_correction = correction * cloth->springs[c]->node1->w;
        double second_correction = -correction * cloth->springs[c]->node2->w;

        cloth->springs[c]->node1->position += distance * first_correction;
        cloth->springs[c]->node2->position += distance * second_correction;
    }
}

void PositionBasedIntegrator::updateVelocities(double timeStep) {
#pragma omp parallel for
    for (int n = 0; n < cloth->nodes.size(); n++) {
        if (cloth->nodes[n]->w == 0.0f)
            continue;

        cloth->nodes[n]->velocity = (cloth->nodes[n]->position - cloth->nodes[n]->old_position) * (1.0f / timeStep);
    }
}

void PositionBasedIntegrator::update() {
    for (int i = 0; i < iterationFreq; i++) {
        predict(TIME_STEP, gravity);
        solveConstraint(TIME_STEP);
        for (auto* collider : colliders) {
            if (collider)
                collider->resolveCollision(cloth);
        }
        updateVelocities(TIME_STEP);
    }
}

std::vector<ICollider*> PositionBasedIntegrator::getColliders() {
    return colliders;
}

void PositionBasedIntegrator::addCollider(ICollider* col) {
    colliders.push_back(col);
}