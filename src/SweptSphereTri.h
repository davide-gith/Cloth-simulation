#ifndef SWEPT_SPHERE_TRI_H
#define SWEPT_SPHERE_TRI_H

#include <vector>
#include "glm/glm.hpp"
#include "Vectors.h"   // Usa il tuo Vec3 (double-based)
#include "Vertex.h"    // Tuo Vertex (pos, normal)

class SweptSphereTri {
public:
    // Risoluzioni (per i bordi che riusano SweptSphere)
    int radialSegments;   // >=3
    int rings;            // >=1
    int stacks;           // >=2

    // Tre sfere ai vertici
    Vec3  c0, c1, c2;
    float r0, r1, r2;

    // Colore (se lo usi nel render)
    glm::vec4 color;
    const double friction = 0.8;


    // Mesh risultante (compatibile col tuo progetto)
    std::vector<Vertex*> vertexes;  // vertici allocati con new
    std::vector<Vertex*> faces;     // tri come puntatori (triplette)

    SweptSphereTri(const Vec3& C0, float R0,
        const Vec3& C1, float R1,
        const Vec3& C2, float R2,
        const glm::vec4& col,
        int radial = 32, int Rings = 16, int Stacks = 16);

    ~SweptSphereTri();

    // Se hai una pipeline di draw, puoi implementarla simile a SweptSphere
    // void render() const;

private:
    void init();

    // Calcola la normale costante della patch planare di faccia
    static void facePatchNormal(const Vec3& c0, float r0,
        const Vec3& c1, float r1,
        const Vec3& c2, float r2,
        Vec3& n_out);

    // Aggiunge il triangolo di faccia "gonfiato" (planare)
    void addFacePatch(const Vec3& c0, float r0,
        const Vec3& c1, float r1,
        const Vec3& c2, float r2);

    // Aggiunge (deep copy) una mesh (vertexes/faces) alla mesh di questa classe
    void appendMeshDeep(const std::vector<Vertex*>& srcV,
        const std::vector<Vertex*>& srcF);
};

#endif // SWEPT_SPHERE_TRI_H
