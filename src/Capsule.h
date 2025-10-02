#ifndef CAPSULE_H
#define CAPSULE_H

#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Capsule
{
public:
    int meridianNum;   
    int parallelNum;   

    // Dati geometrici principali
    Vec3 topPoint;                // polo superiore (sulla superficie)
    Vec3 bottomPoint;             // polo inferiore (sulla superficie)
    int  radius;                  // raggio
    glm::vec4 color;
    const double friction = 0.8;

    Vec3 centerTopHemisphere;
    Vec3 centerBottomHemisphere;

    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;

    Capsule(Vec3 topP, Vec3 bottomP, int r, glm::vec4 c);

    ~Capsule();

    Vertex* getTop();
    Vertex* getBottom();
    Vertex* getRingVertex(int ringIndex, int meridianIndex);

private:
    static Vec3 faceNormal(Vertex* v1, Vertex* v2, Vertex* v3);
    void accumulateAndNormalizeNormals();
    static void orthonormalBasis(const Vec3& n, Vec3& x, Vec3& y);
    void addRing(const Vec3& ringCenter, double ringRadius, const Vec3& ux, const Vec3& uy);
    void addFanFaces(Vertex* pole, int ringStartIndex);
    void addStripFaces(int ringAStart, int ringBStart);
    void init();
};


#endif