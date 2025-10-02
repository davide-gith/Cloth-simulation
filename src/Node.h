#ifndef NODE_H
#define NODE_H

#include "Vectors.h"

class Node{
public:
    unsigned int id; // Unique ID
    double  mass;
    double  w;
    bool    isFixed;
    Vec2    texCoord;
    Vec3    normal;
    Vec3    tangentSum;
    int     tangentCount = 0;
    Vec3    tangent;
    Vec3    bitangent;
    Vec3	position;
    Vec3    old_position;
	Vec3    initial_position;
    Vec3    velocity;
    Vec3    prec_velocity;
    Vec3	acceleration;    
    Vec3    force;

    Vec3    residual_G;
    Vec3    inertia;

	Node();
	Node(Vec3 p);
    Node(int id, Vec3 p, Vec2 texCoord);

    void addForce(Vec3 f);
	void ExplicitIntegrate(double timeStep);
	void SimpletticIntegrate(double timeStep);
    void residualUpdate(double timeStep);

};

#endif