#ifndef GROUND_H
#define GROUND_H

#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

class Ground
{
public:
    Vec3 position;
    int width, height;
    glm::vec4 color;
    const double friction = 0.9;

    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;

    Ground(Vec3 pos, Vec2 size, glm::vec4 c);
    ~Ground();

    void init();
};
#endif