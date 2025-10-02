#include "Ground.h"
#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"


Ground::Ground(Vec3 pos, Vec2 size, glm::vec4 c) {
    position = pos;
    width = size.x;
    height = size.y;
    color = c;

    init();
}

Ground::~Ground()
{
    for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
    vertexes.clear();
    faces.clear();
}

void Ground::init() {
    vertexes.push_back(new Vertex(Vec3(0.0, 0.0, 0.0)));
    vertexes.push_back(new Vertex(Vec3(width, 0.0, 0.0)));
    vertexes.push_back(new Vertex(Vec3(0.0, 0.0, -height)));
    vertexes.push_back(new Vertex(Vec3(width, 0.0, -height)));

    for (int i = 0; i < vertexes.size(); i++) {
        vertexes[i]->normal = Vec3(0.0, 1.0, 0.0);
        //printf("Ground[%d]: (%f, %f, %f) - (%f, %f, %f)\n", i, vertexes[i]->position.x, vertexes[i]->position.y, vertexes[i]->position.z, vertexes[i]->normal.x, vertexes[i]->normal.y, vertexes[i]->normal.z);
    }

    faces.push_back(vertexes[0]);
    faces.push_back(vertexes[1]);
    faces.push_back(vertexes[2]);
    faces.push_back(vertexes[1]);
    faces.push_back(vertexes[2]);
    faces.push_back(vertexes[3]);
    /*std::cout << "Ground: " << std::endl;
    std::cout << "- vertexes: " << vertexes.size() << std::endl;
    std::cout << "- faces: " << faces.size() << std::endl;*/
}
