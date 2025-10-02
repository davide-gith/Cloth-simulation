#include <iostream>
#include <memory>
#include "ClothInstance.h"
#include "ClothData.h"
#include "ExplicitEulerIntegrator.h"
#include "ImplicitNewtonIntegrator.h"
#include "SymplecticEulerIntegrator.h"
#include "PositionBasedIntegrator.h"
#include "ProjectiveDynamicsIntegrator.h"
#include "ICollider.h"
#include "SphereCollider.h"
#include "GroundCollider.h"


ClothInstance::ClothInstance(ClothData* d, IClothSimulator* s)
    : data(d), simulator(s) {
}

ClothInstance* ClothInstance::create(const std::string& shape, const std::string& method) {
    ClothData* data = new ClothData(shape);
    IClothSimulator* sim = nullptr;

    if (method == "ExplicitEuler") {
        sim = new ExplicitEulerIntegrator(data);
    }
	else if (method == "SymplecticEuler") {
		sim = new SymplecticEulerIntegrator(data);
	}
    else if (method == "PBD") {
		sim = new PositionBasedIntegrator( data,"PBD");
    }
	else if (method == "XPBD") {
		sim = new PositionBasedIntegrator(data, "XPBD");
	}
	else if (method == "ProjectiveDynamics") {
		sim = new ProjectiveDynamicsIntegrator(data);
	}
	else if (method == "ImplicitNewton") {
		sim = new ImplicitNewtonIntegrator(data);
	}
	else {
		std::cout << "ERROR::ClothInstance : Unsupported simulation method: " << method << std::endl;
		delete data;
		return nullptr;
	}

	//sim->addCollider(new GroundCollider());


    return new ClothInstance(data, sim);
}

void ClothInstance::update() {
    simulator->update();
}

void ClothInstance::unpin() {
	simulator->unpin();
}

void ClothInstance::computeNormal() {
	data->computeNormal();
}

ClothData* ClothInstance::getData() {
    return data;
}

IClothSimulator* ClothInstance::getSimulator() {
    return simulator;
}

std::vector<ICollider*> ClothInstance::getColliders() {
	return simulator->getColliders();
}

void ClothInstance::addCollider(ICollider* col) {
	simulator->addCollider(col);
}

void ClothInstance::restart() {
	for (int i = 0; i < data->nodes.size(); i++) {
		data->nodes[i]->normal.setZeroVec();
		data->nodes[i]->position = data->nodes[i]->initial_position;
		data->nodes[i]->old_position = data->nodes[i]->initial_position;
		data->nodes[i]->velocity.setZeroVec();
		data->nodes[i]->prec_velocity.setZeroVec();
		data->nodes[i]->acceleration.setZeroVec();
		data->nodes[i]->force.setZeroVec();
	}
	data->nodes[0]->isFixed = true;
	data->nodes[data->nodesPerRow - 1]->isFixed = true;
	data->nodes[0]->mass = std::numeric_limits<double>::infinity();
	data->nodes[0]->w = 0.0;
	data->nodes[data->nodesPerRow - 1]->mass = std::numeric_limits<double>::infinity();
	data->nodes[data->nodesPerRow - 1]->w = 0.0;
}