#include "Vertex.h"

// Contructors
Vertex::Vertex() : position(Vec3(0.0, 0.0, 0.0)), normal(Vec3(0.0, 0.0, 0.0)) {}
Vertex::Vertex(Vec3 p) : position(p), normal(Vec3(0.0, 0.0, 0.0)) {}

// Destructor
Vertex::~Vertex() {}