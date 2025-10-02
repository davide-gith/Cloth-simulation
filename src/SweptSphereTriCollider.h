#ifndef SWEPT_SPHERE_TRI_COLLIDER_H
#define SWEPT_SPHERE_TRI_COLLIDER_H

#include <glm/glm.hpp>
#include "ICollider.h"
#include "Vectors.h"
#include "Display.h"
#include "SweptSphereTri.h"

class SweptSphereTriRender;

class SweptSphereTriCollider : public ICollider {
public:
    Vec3 center1;
    Vec3 center2;
    Vec3 center3;
    float radius1;
    float radius2;
    float radius3;
    glm::vec4 color;

    // --- precompute per broad-phase e baricentriche ---
    Vec3 tri_a, tri_b, tri_c;      // = center1,2,3 (copie locali)
    Vec3 tri_v0, tri_v1;           // b-a, c-a
    double tri_d00, tri_d01, tri_d11;
    double tri_invDen;             // 1 / (d00*d11 - d01*d01), 0 se degenere

    // bounding sphere molto semplice per broad-phase
    Vec3  bb_center;               // media dei centri
    double bb_radiusSq;            // (max(|Ci - bb_center| + ri))^2

    SweptSphereTri* sweptspheretri;
    SweptSphereTriRender* renderer;

    SweptSphereTriCollider();
    SweptSphereTriCollider(Vec3 c1, float r1, Vec3 c2, float r2, Vec3 c3, float r3);
    void resolveCollision(ClothData* data) override;
    bool closestSphere(const Vec3& p, Vec3& centerOut, float& radiusOut, float* baryOut = nullptr) const;
    std::array<double, 3> baryFromPlaneProjectionFast(const Vec3& p) const;
    static void projectSimplex(double b[3]);    void render() override;
    void precompute();
    ~SweptSphereTriCollider();
};

#endif