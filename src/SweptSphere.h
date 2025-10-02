#ifndef SWEPT_SPHERE_H
#define SWEPT_SPHERE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"
#include <vector>

class SweptSphere
{
public:
    int radialSegments;   // numero di meridiani (>=3)
    int ringSegments;     // anelli lungo l’asse (>=1)
    int hemiStacks;       // suddivisione dei cap (>=2)

    // Parametri geometrici
    Vec3 c0; 
    Vec3 c1; 
    float r0;
    float r1;
    glm::vec4 color;
    const double friction = 0.8;

    // Mesh (compatibile col tuo progetto)
    std::vector<Vertex*> vertexes;  // tutti i vertici
    std::vector<Vertex*> faces;     // triplette (tris) come puntatori ai vertex

    SweptSphere(Vec3 C0, float R0, Vec3 C1, float R1, glm::vec4 col, int radial = 64, int rings = 24, int stacks = 16);
    ~SweptSphere();

    void init();

private:
    Vertex* addVertex(const Vec3& p);
    void stitchRingPairCCW(const std::vector<int>& ringA, const std::vector<int>& ringB);
    Vec3 computeFaceNormal(const Vec3& a, const Vec3& b, const Vec3& c);
    void computeNormals();

    // Frame ortonormale con Z allineata a (c1-c0)
    void makeOrthonormalFrame(const Vec3& axisZ, Vec3& X, Vec3& Y, Vec3& Z);

    // Generatori anelli/cap
    std::vector<int> addRingTangent(const Vec3& C, float R, const Vec3& X, const Vec3& Y, const Vec3& Z, int radial, float k);
    std::vector<int> addSphericalCapPhi(const Vec3& C, float R, const Vec3& X, const Vec3& Y, const Vec3& Z, int radial, int stacks, float hemiSign, float phi0, float phi1, bool useCW);
    void stitchRingPairCW(const std::vector<int>& ringA, const std::vector<int>& ringB);

};

#endif // SWEPT_SPHERE_H
