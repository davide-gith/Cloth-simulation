#ifndef BOX_H
#define BOX_H

#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

class Box
{
public:
    Vec3 center;
    Vec3 extent;
    glm::vec4 color;
    const double friction = 0.8;

    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;

    Box(const Vec3& c, const Vec3& halfExtent, const glm::vec4& col);
    ~Box();

    Vec3 getCenter() const;
    Vec3 getExtent() const;

private:
    Vec3 computeFacenormal(Vertex* v1, Vertex* v2, Vertex* v3);
    void computeNormals();
    void addFace(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3);
    Vertex* makeVertex(const Vec3& p);
    void init();
};

#endif