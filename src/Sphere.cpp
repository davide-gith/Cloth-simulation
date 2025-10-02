#include "Sphere.h"
#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

Sphere::Sphere(Vec3 cen, float r, glm::vec4 c)
{
    center = cen;
    radius = r;
    color = c;

    meridianNum = 24;
    parallelNum = 250;

    init();
}

Sphere::~Sphere()
{
    for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
    vertexes.clear();
    faces.clear();
}

Vertex* Sphere::getTop() {
    return vertexes[0]; 
}

Vertex* Sphere::getVertex(int x, int y)
{
    if (x < 0 || x >= parallelNum || y < 0 || y >= meridianNum) {
        printf("Vertex Index Out of Range.\n");
        exit(-1);
    }
    else {
        return vertexes[1 + x * meridianNum + y];
    }
}

Vertex* Sphere::getBottom() {
    return vertexes[vertexes.size() - 1]; 
}

Vec3 Sphere::computeFacenormal(Vertex* v1, Vertex* v2, Vertex* v3)
{
    return Vec3::cross(v2->position - v1->position, v3->position - v1->position);
}

void Sphere::computeSpherenormal()
{
    Vec3 normal(0.0, 0.0, 0.0);
    for (int i = 0; i < vertexes.size(); i++) {
        vertexes[i]->normal = normal;
    }

    // The normal of all faces of the first and last cycle should be calculated specially!
    for (int i = 0; i < faces.size() / 3; i++) {
        Vertex* v1 = faces[i * 3 + 0];
        Vertex* v2 = faces[i * 3 + 1];
        Vertex* v3 = faces[i * 3 + 2];

        normal = computeFacenormal(v1, v3, v2);
        v1->normal += normal;
        v2->normal += normal;
        v3->normal += normal;
    }

    for (int i = 0; i < vertexes.size(); i++) {
        vertexes[i]->normal.normalize();
    }
}

void Sphere::init() // Initialize vertexes coord and slice faces
{
    /** Compute vertex position **/
    double cycleInterval = radius * 2.0 / (parallelNum + 1);
    double radianInterval = 2.0 * M_PI / meridianNum;


    Vec3 pos(0.0, radius, 0.0);
    vertexes.push_back(new Vertex(pos)); // Top vertex

    for (int i = 0; i < parallelNum; i++) {
        pos.y -= cycleInterval;
        for (int j = 0; j < meridianNum; j++) {
            double xzLen = radius * sqrt(1.0 - pow(pos.y / radius, 2));
            double xRadian = j * radianInterval;  // The length of projection line on X-Z pane

            pos.x = xzLen * sin(xRadian);
            pos.z = xzLen * cos(xRadian);
            vertexes.push_back(new Vertex(pos));
        }
    }
    pos = Vec3(0.0, -radius, 0.0);
    vertexes.push_back(new Vertex(pos)); // Bottom vertex

    /** Slice faces **/
    // Top cycle
    for (int i = 0; i < meridianNum; i++) {
        faces.push_back(getVertex(0, i));                               //   *   //
        faces.push_back(getTop());                                      //  / \  //
        faces.push_back(getVertex(0, (i + 1) % meridianNum));               // *---* //
    }
    // Middle cycles
    for (int i = 0; i < parallelNum - 1; i++) {
        for (int j = 0; j < meridianNum; j++) {
            faces.push_back(getVertex(i, j));                           //  *--* //
            faces.push_back(getVertex(i, (j + 1) % meridianNum));           //  | /  //
            faces.push_back(getVertex(i + 1, j));                         //  *    //

            faces.push_back(getVertex(i + 1, (j + 1) % meridianNum));         //     * //
            faces.push_back(getVertex(i + 1, j));                         //   / | //
            faces.push_back(getVertex(i, (j + 1) % meridianNum));           //  *--* //
        }
    }
    // Bottom cycle
    for (int i = 0; i < meridianNum; i++) {
        faces.push_back(getBottom());                                   // *---* //
        faces.push_back(getVertex(parallelNum - 1, i));                   //  \ /  //
        faces.push_back(getVertex(parallelNum - 1, (i + 1) % meridianNum));   //   *   //
    }

    /** Set normals **/
    computeSpherenormal();
}