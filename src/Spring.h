#ifndef SPRING_H
#define SPRING_H

#include <utility>
#include "Node.h"
#include "Matrices.h"

class Spring {
public:
	unsigned int id; // Unique ID for the spring
	Node* node1;
	Node* node2;
	double restLen;
	double hookCoef; // Compliance in PBD Approach
	double dampCoef;

	Mat3x3 jacobianBlock;

	Spring(int id, Node* n1, Node* n2, double k);
	void applyInternalForce(double timeStep);
	void jacobianUpdate(double timeStep);
	
	~Spring();
};

#endif