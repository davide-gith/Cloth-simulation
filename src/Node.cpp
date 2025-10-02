#include "Node.h"
#include "Vectors.h"

Node::Node() :
	mass(1.0),
	w(1.0 / mass),
	isFixed(false),
	texCoord(Vec2()),
	normal(Vec3()),
	position(Vec3()),
	old_position(Vec3()),
	initial_position(Vec3()),
	velocity(Vec3()),
	prec_velocity(Vec3()),
	acceleration(Vec3()),
	force(Vec3())
{}

Node::Node(Vec3 p) :
	mass(1.0),
	w(1.0 / mass),
	isFixed(false),
	texCoord(Vec2()),
	normal(Vec3()),
	position(p),
	old_position(p),
	initial_position(p),
	velocity(Vec3()),
	prec_velocity(Vec3()),
	acceleration(Vec3()),
	force(Vec3())
{}


Node::Node(int id, Vec3 p, Vec2 texCoord) :
	id(id),
	mass(1.0),
	w(1.0 / mass),
	isFixed(false),
	texCoord(texCoord),
	normal(Vec3()),
	tangentSum(Vec3()),
	tangentCount(0),
	tangent(Vec3()),
	position(p),
	old_position(p),
	initial_position(p),
	velocity(Vec3()),
	prec_velocity(Vec3()),
	acceleration(Vec3()),
	force(Vec3())
{
}



void Node::addForce(Vec3 f)
{
	force += f;
}

void Node::ExplicitIntegrate(double timeStep) // Only non-fixed nodes take integration
{
	if (!isFixed)
	{
		position += velocity * timeStep;
		acceleration = force / mass;
		velocity += acceleration * timeStep;
	}
	force.setZeroVec();
}

void Node::SimpletticIntegrate(double timeStep) // Only non-fixed nodes take integration
{
	if (!isFixed)
	{
		acceleration = force / mass;
		velocity += acceleration * timeStep;
		position += velocity * timeStep;
	}
	force.setZeroVec();
}

void Node::residualUpdate(double timeStep)
{
	if (!isFixed) {
		residual_G = ((position - old_position - velocity * timeStep) * mass) - (force * (timeStep * timeStep));
	}
}