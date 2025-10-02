#include <utility>
#include "Spring.h"
#include "Node.h"
#include "Vectors.h"

#include <iostream>
#include <iomanip>

Spring::Spring(int id, Node* n1, Node* n2, double k) :
	id(id),
	node1(n1),
	node2(n2),
    restLen((n1->position - n2->position).length()),
	hookCoef(k),
	dampCoef(5.0)
{}

Spring::~Spring() {}


void Spring::applyInternalForce(double timeStep) // Compute spring internal force
{
    double currLen = Vec3::dist(node1->position, node2->position);
    Vec3 fDir1 = (node2->position - node1->position) / currLen;
    Vec3 diffV1 = node2->velocity - node1->velocity;
    Vec3 f1 = fDir1 * ((currLen - restLen) * hookCoef + Vec3::dot(diffV1, fDir1) * dampCoef);

    node1->addForce(f1);
    node2->addForce(f1.minus());

}

void Spring::jacobianUpdate(double timeStep) {
	
    Vec3 diffPos = node2->position - node1->position;
	double currLen = diffPos.length();
	Vec3 unit_vec = diffPos / currLen;

    Mat3x3 K_block =(Mat3x3::identity() * (1 - restLen / currLen)  +              //k_len_term
                     Vec3::outer(diffPos,diffPos)*restLen * (1 / pow(currLen, 3)) //k_dir_term
                    ) * (-hookCoef);

    Mat3x3 D_block = Vec3::outer(unit_vec, unit_vec) * ( - dampCoef);

    jacobianBlock = -K_block * timeStep * timeStep - D_block * timeStep;
    
}