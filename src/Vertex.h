#ifndef VERTEX_H
#define VERTEX_H

#include "Vectors.h"

class Vertex {
public:
	Vec3 position;
	Vec3 normal;

	Vertex();
	Vertex(Vec3 p);

	~Vertex();

};

#endif