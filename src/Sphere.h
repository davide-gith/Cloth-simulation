#ifndef SPHERE_H
#define SPHERE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

class Sphere
{
public:
    int meridianNum;
    int parallelNum;

    Vec3 center;
    float radius;
    glm::vec4 color;
    const double friction = 0.8;

    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;

    Sphere(Vec3 cen, float r, glm::vec4 c);
    ~Sphere();

    Vertex* getTop();
    Vertex* getVertex(int x, int y);
    Vertex* getBottom();
    Vec3 computeFacenormal(Vertex* v1, Vertex* v2, Vertex* v3);
    void computeSpherenormal();
    void init();
};
#endif