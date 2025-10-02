#include <glm/glm.hpp>
#include <algorithm>
#include <array>
#include "SweptSphereTri.h"
#include "SweptSphereTriCollider.h"
#include "Vectors.h"
#include "Display.h"

SweptSphereTriCollider::SweptSphereTriCollider() {
    center1 = Vec3(0.0f, -1.0f, -5.0f);
    center2 = Vec3(8.0f, 1.0f, -5.0f);
    center3 = Vec3(4.0f, -3.0f, -5.0f);
	
	radius1 = 1.0f;
	radius2 = 1.0f;
	radius3 = 1.0f;

	glm::vec4 color(1.0f, 0.647f, 0.0f, 1.0f);

	sweptspheretri = new SweptSphereTri(center1, radius1, center2, radius2, center3, radius3, color, 64, 32, 24);
	renderer = new SweptSphereTriRender(sweptspheretri);

	precompute();
}

SweptSphereTriCollider::SweptSphereTriCollider(Vec3 c1, float r1, Vec3 c2, float r2, Vec3 c3, float r3) :
	center1(c1), radius1(r1), 
	center2(c2), radius2(r2),
	center3(c3), radius3(r3) 
{
	glm::vec4 color(1.0f, 0.647f, 0.0f, 1.0f);

	sweptspheretri = new SweptSphereTri(center1, radius1, center2, radius2, center3, radius3, color, 16, 8, 6);
	renderer = new SweptSphereTriRender(sweptspheretri);

	precompute();
}

SweptSphereTriCollider::~SweptSphereTriCollider() {
	delete sweptspheretri;
	delete renderer;
}

void SweptSphereTriCollider::resolveCollision(ClothData* data)
{
    const double inflate = 1.05;
    const double inflate1 = radius1 * inflate;
    const double inflate2 = radius2 * inflate;
    const double inflate3 = radius3 * inflate;
    const double r1Sq = inflate1 * inflate1;
    const double r2Sq = inflate2 * inflate2;
    const double r3Sq = inflate3 * inflate3;

    auto likelyOutside = [&](const Vec3& wp)->bool {
        Vec3 d = Vec3(wp.x - bb_center.x, wp.y - bb_center.y, wp.z - bb_center.z);
        double dsq = Vec3::dot(d, d);
        return dsq > bb_radiusSq;
        };

    // Utility: proiezione su sfera (senza sqrt extra)
    auto projectOnSphere = [&](const Vec3& p, const Vec3& c, double safeR, double safeRSq, Vec3& outPos)->bool {
        Vec3 d = Vec3(p.x - c.x, p.y - c.y, p.z - c.z);
        double dsq = Vec3::dot(d, d);
        if (dsq < safeRSq) {
            double invLen = 0.0;
            if (dsq > 1e-30) invLen = 1.0 / std::sqrt(dsq);
            // posizione = centro + direzione * r_sicuro
            Vec3 supp = d * (safeR * invLen);
            outPos = Vec3(c.x + supp.x, c.y + supp.y, c.z + supp.z);
            return true;
        }
        return false;
        };

    const double fric = sweptspheretri->friction;

    // Dati compattati per i 3 centri/raggi (così loopiamo)
    const Vec3  C[3] = { center1, center2, center3 };
    const double R[3] = { inflate1, inflate2, inflate3 };
    const double RSq[3] = { r1Sq, r2Sq, r3Sq };

#pragma omp parallel for
    for (int i = 0; i < (int)data->nodes.size(); ++i)
    {
        Node* node = data->nodes[i];
        Vec3  p = data->getWorldPos(node);

        // 1) broad-phase
        if (likelyOutside(p)) {
            continue;
        }

        // 2) prova rapida contro le 3 sfere pure
        bool hit = false;
        Vec3 newPos;
        for (int s = 0; s < 3; ++s) {
            if (projectOnSphere(p, C[s], R[s], RSq[s], newPos)) {
                hit = true;
                break;
            }
        }
        if (hit) {
            data->setWorldPos(node, newPos);
            node->velocity = node->velocity * fric;
            continue;
        }

        // 3) sfera interpolata (SSTri): early-out cheap
        Vec3 nPlane = Vec3::cross(tri_v0, tri_v1);
        double nLen = nPlane.length();
        if (nLen > 1e-30) {
            nPlane = nPlane / nLen;
            double distPlane = std::fabs(Vec3::dot(p - tri_a, nPlane));
            double maxR = std::max(radius1, std::max(radius2, radius3));
            if (distPlane > maxR * 1.2) {
                continue;
            }
        }

        Vec3  cInterp;
        float rInterp;
        float bary[3];
        if (!closestSphere(p, cInterp, rInterp, bary)) {
            continue;
        }

        const double safeR = (double)rInterp * inflate;
        const double safeRSq = safeR * safeR;
        if (projectOnSphere(p, cInterp, safeR, safeRSq, newPos)) {
            data->setWorldPos(node, newPos);
            node->velocity = node->velocity * fric;
        }
    }
}

void SweptSphereTriCollider::precompute()
{
    tri_a = center1; tri_b = center2; tri_c = center3;
    tri_v0 = tri_b - tri_a;
    tri_v1 = tri_c - tri_a;
    tri_d00 = Vec3::dot(tri_v0, tri_v0);
    tri_d01 = Vec3::dot(tri_v0, tri_v1);
    tri_d11 = Vec3::dot(tri_v1, tri_v1);
    double den = tri_d00 * tri_d11 - tri_d01 * tri_d01;
    tri_invDen = (std::fabs(den) > 1e-12) ? 1.0 / den : 0.0;

    // bounding sphere grezza: centro = media, r = max( |Ci - media| + ri )
    bb_center = (center1 + center2 + center3) * (1.0 / 3.0);
    double r0 = (center1 - bb_center).length() + radius1;
    double r1 = (center2 - bb_center).length() + radius2;
    double r2 = (center3 - bb_center).length() + radius3;
    double r = std::max(r0, std::max(r1, r2));
    // margine 5%
    r *= 1.05;
    bb_radiusSq = r * r;
}

void SweptSphereTriCollider::projectSimplex(double b[3]) {
    double u[3] = { b[0], b[1], b[2] };
    int idx[3] = { 0,1,2 };
    std::sort(idx, idx + 3, [&](int i, int j) { return u[i] > u[j]; });

    double tmpsum = 0.0, theta = 0.0;
    int rho = -1;
    for (int j = 0; j < 3; ++j) {
        tmpsum += u[idx[j]];
        double t = (tmpsum - 1.0) / double(j + 1);
        if (u[idx[j]] - t > 0.0) { rho = j; theta = t; }
    }
    for (int j = 0; j < 3; ++j) b[j] = std::max(0.0, u[j] - theta);

    double s = b[0] + b[1] + b[2];
    if (s > 1e-12) { b[0] /= s; b[1] /= s; b[2] /= s; }
    else { b[0] = 1.0; b[1] = b[2] = 0.0; }
}

// ------- Baricentriche clampate della proiezione di p sul piano -------
std::array<double, 3> SweptSphereTriCollider::baryFromPlaneProjectionFast(const Vec3& p) const
{
    if (tri_invDen == 0.0) return { 1.0, 0.0, 0.0 }; // degenere: cadi sul vertice A

    Vec3 v2 = Vec3(p.x - tri_a.x, p.y - tri_a.y, p.z - tri_a.z);
    double d20 = Vec3::dot(v2, tri_v0);
    double d21 = Vec3::dot(v2, tri_v1);

    double v = (tri_d11 * d20 - tri_d01 * d21) * tri_invDen;
    double w = (tri_d00 * d21 - tri_d01 * d20) * tri_invDen;
    double u = 1.0 - v - w;

    double b[3] = { u, v, w };
    projectSimplex(b);
    return { b[0], b[1], b[2] };
}

// ------- Sfera interpolata più “a contatto” con p -------
bool SweptSphereTriCollider::closestSphere(const Vec3& p, Vec3& centerOut, float& radiusOut, float* baryOut) const {
    const double eps = 1e-12;
    const int    it1D = 24;    // massimo 24 iterazioni sugli edge
    const int    it2D = 25;    // massimo 25 iterazioni nell’interno
    const double alpha0 = 0.5;   // passo iniziale ascent
    const double shrink = 0.5;   // backtracking

    Vec3   C[3] = { center1, center2, center3 };
    double R[3] = { radius1, radius2, radius3 };

    auto vlen = [&](const Vec3& v)->double { return std::sqrt(std::max(0.0, Vec3::dot(v, v))); };
    auto clamp01 = [](double t) { return (t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t)); };

    auto evalFromBary = [&](const double b[3], Vec3& c, double& r, double& phi) {
        c = C[0] * b[0] + C[1] * b[1] + C[2] * b[2];
        r = b[0] * R[0] + b[1] * R[1] + b[2] * R[2];
        phi = r - vlen(Vec3(p.x - c.x, p.y - c.y, p.z - c.z));
        };

    auto gradPhi = [&](const double b[3], Vec3& c, double& r, double g[3], double& phi) {
        evalFromBary(b, c, r, phi);
        Vec3 d = Vec3(p.x - c.x, p.y - c.y, p.z - c.z);
        double L = vlen(d);
        Vec3 u = (L > eps) ? (d * (1.0 / L)) : Vec3(0, 1, 0);
        g[0] = R[0] + Vec3::dot(u, C[0]);
        g[1] = R[1] + Vec3::dot(u, C[1]);
        g[2] = R[2] + Vec3::dot(u, C[2]);
        };

    // best candidate
    double bestPhi = -1e100;
    double bestB[3] = { 1,0,0 };
    Vec3   bestC;
    double bestR = 0.0;

    auto considerBary = [&](const double b[3]) {
        Vec3 c; double r, phi;
        evalFromBary(b, c, r, phi);
        if (phi > bestPhi) { bestPhi = phi; bestB[0] = b[0]; bestB[1] = b[1]; bestB[2] = b[2]; bestC = c; bestR = r; }
        };

    // 1) Vertici
    { double b0[3] = { 1,0,0 }; considerBary(b0); }
    { double b1[3] = { 0,1,0 }; considerBary(b1); }
    { double b2[3] = { 0,0,1 }; considerBary(b2); }

    // 2) Edge (golden-section)
    auto maximizeOnEdge = [&](int i, int j) {
        auto phiAt = [&](double t)->double {
            t = clamp01(t);
            Vec3 c = C[i] * (1.0 - t) + C[j] * t;
            double r = (1.0 - t) * R[i] + t * R[j];
            return r - vlen(Vec3(p.x - c.x, p.y - c.y, p.z - c.z));
            };
        const double gr = (std::sqrt(5.0) - 1.0) * 0.5;
        double a = 0.0, b = 1.0;
        double c1 = b - gr * (b - a), c2 = a + gr * (b - a);
        double f1 = phiAt(c1), f2 = phiAt(c2);
        for (int k = 0; k < it1D; ++k) {
            if (f1 < f2) { a = c1; c1 = c2; f1 = f2; c2 = a + gr * (b - a); f2 = phiAt(c2); }
            else { b = c2; c2 = c1; f2 = f1; c1 = b - gr * (b - a); f1 = phiAt(c1); }
        }
        double t = clamp01(0.5 * (a + b));
        double bary[3] = { 0,0,0 };
        bary[i] = 1.0 - t; bary[j] = t;
        considerBary(bary);
        };
    maximizeOnEdge(0, 1);
    maximizeOnEdge(1, 2);
    maximizeOnEdge(2, 0);

    // 3) Interno (projected gradient ascent)
    {
        auto b0arr = baryFromPlaneProjectionFast(p);
        double bcur[3] = { b0arr[0], b0arr[1], b0arr[2] };

        for (int it = 0; it < it2D; ++it) {
            Vec3 c; double r, phi; double g[3];
            gradPhi(bcur, c, r, g, phi);

            double dir[3] = { g[0], g[1], g[2] };
            double alpha = alpha0;
            bool accepted = false;
            for (int ls = 0; ls < 8; ++ls) {
                double trial[3] = { bcur[0] + alpha * dir[0],
                                    bcur[1] + alpha * dir[1],
                                    bcur[2] + alpha * dir[2] };
                projectSimplex(trial);

                Vec3 ct; double rt, phit;
                evalFromBary(trial, ct, rt, phit);
                double gnorm1 = std::fabs(dir[0]) + std::fabs(dir[1]) + std::fabs(dir[2]);
                if (phit >= phi + 1e-6 * alpha * gnorm1) {
                    bcur[0] = trial[0]; bcur[1] = trial[1]; bcur[2] = trial[2];
                    accepted = true;
                    break;
                }
                alpha *= shrink;
            }
            if (!accepted) break;
        }
        considerBary(bcur);
    }

    if (bestPhi <= -1e20) return false;

    centerOut = bestC;
    radiusOut = (float)bestR;
    if (baryOut) { baryOut[0] = (float)bestB[0]; baryOut[1] = (float)bestB[1]; baryOut[2] = (float)bestB[2]; }
    return true;
}

void SweptSphereTriCollider::render() {
	renderer->flush();
}