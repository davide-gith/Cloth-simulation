#include "SweptSphere.h"
#include "Vectors.h"
#include <cmath>
#include <algorithm>

static inline float dotv(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vec3  crossv(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float normv(const Vec3& v) { return std::sqrt(dotv(v, v)); }
static inline Vec3  normalizev(const Vec3& v) { float n = normv(v); return (n > 0) ? Vec3(v.x * (1.0f / n), v.y * (1.0f / n), v.z * (1.0f / n)) : Vec3(); }

SweptSphere::SweptSphere(Vec3 C0, float R0, Vec3 C1, float R1, glm::vec4 col, int radial, int rings, int stacks) :
    radialSegments(std::max(3, radial)),
    ringSegments(std::max(1, rings)),
    hemiStacks(std::max(2, stacks)),
    c0(C0), 
    r0(R0), 
    c1(C1), 
    r1(R1),
    color(col)
{
    init();
}

SweptSphere::~SweptSphere() {
    for (auto* v : vertexes) delete v;
    vertexes.clear();
    faces.clear();
}

Vertex* SweptSphere::addVertex(const Vec3& p) {
    Vertex* v = new Vertex(p);
    vertexes.push_back(v);
    return v;
}

void SweptSphere::makeOrthonormalFrame(const Vec3& axisZ, Vec3& X, Vec3& Y, Vec3& Z) {
    Z = normalizev(axisZ);
    Vec3 tmp = (std::fabs(Z.z) < 0.999f) ? Vec3(0, 0, 1) : Vec3(0, 1, 0);
    X = normalizev(crossv(tmp, Z));
    Y = crossv(Z, X);
}

std::vector<int> SweptSphere::addRingTangent(const Vec3& C, float R,
    const Vec3& X, const Vec3& Y, const Vec3& Z,
    int radial, float k)
{

    float kk = std::clamp(k, -0.9999f, 0.9999f);
    float sXY = std::sqrt(std::max(0.0f, 1.0f - kk * kk));
    const float TAU = 6.283185307179586f;

    std::vector<int> ring; ring.reserve(radial);
    for (int i = 0; i < radial; i++) {
        float th = TAU * (float)i / (float)radial;
        float ct = std::cos(th), st = std::sin(th);
        Vec3 Xct = Vec3(X.x * ct, X.y * ct, X.z * ct);
        Vec3 Yst = Vec3(Y.x * st, Y.y * st, Y.z * st);
        Vec3 dirXY = Xct + Yst;

        Vec3 Zkk = Vec3(Z.x * (-kk), Z.y * (-kk), Z.z * (-kk));
        Vec3 n = dirXY * sXY + Zkk;
        Vec3 nR = n * R;
        Vec3 p = Vec3(C.x + nR.x, C.y + nR.y, C.z + nR.z);

        int idx = (int)vertexes.size();
        addVertex(p);
        ring.push_back(idx);
    }
    return ring;
}

std::vector<int> SweptSphere::addSphericalCapPhi(const Vec3& C, float R, const Vec3& X, const Vec3& Y, const Vec3& Z, int radial, int stacks, float hemiSign, float phi0, float phi1, bool useCW)
{
    if (phi1 < phi0) std::swap(phi0, phi1);
    const float TAU = 6.283185307179586f;

    std::vector<int> lastRing;
    int prevStart = -1;

    for (int s = 0; s <= stacks; ++s) {
        float t = (float)s / (float)stacks;
        float phi = phi0 + (phi1 - phi0) * t;
        float sp = std::sin(phi), cp = std::cos(phi);
        float ringR = R * sp;
        Vec3 ringC = Vec3(
            C.x + Z.x * (hemiSign * R * cp),
            C.y + Z.y * (hemiSign * R * cp),
            C.z + Z.z * (hemiSign * R * cp)
        );

        std::vector<int> ring; ring.reserve(radial);
        for (int i = 0; i < radial; i++) {
            float th = TAU * (float)i / (float)radial;
            float ct = std::cos(th), st = std::sin(th);
            Vec3 dir = Vec3(X.x * ct + Y.x * st, X.y * ct + Y.y * st, X.z * ct + Y.z * st);
            Vec3 p = ringC + dir * ringR;

            int idx = (int)vertexes.size();
            addVertex(p);
            ring.push_back(idx);
        }

        if (prevStart >= 0) {
            if (useCW)  stitchRingPairCW(lastRing, ring);
            else        stitchRingPairCCW(lastRing, ring);
        }
        lastRing = ring;
        prevStart = lastRing.front();
    }

    return lastRing;
}


void SweptSphere::stitchRingPairCCW(const std::vector<int>& ringA, const std::vector<int>& ringB) {
    int N = (int)ringA.size();
    for (int i = 0; i < N; i++) {
        int i0 = ringA[i];
        int i1 = ringA[(i + 1) % N];
        int j0 = ringB[i];
        int j1 = ringB[(i + 1) % N];

        faces.push_back(vertexes[i0]);
        faces.push_back(vertexes[i1]);
        faces.push_back(vertexes[j0]);

        faces.push_back(vertexes[j1]);
        faces.push_back(vertexes[j0]);
        faces.push_back(vertexes[i1]);
    }
}

Vec3 SweptSphere::computeFaceNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 ba = Vec3(b.x - a.x, b.y - a.y, b.z - a.z);
    Vec3 ca = Vec3(c.x - a.x, c.y - a.y, c.z - a.z);
    return crossv(ba, ca);
}

void SweptSphere::computeNormals() {
    // reset
    for (auto* v : vertexes) v->normal = Vec3(0, 0, 0);

    // somma face normals
    for (size_t t = 0; t < faces.size(); t += 3) {
        Vertex* v1 = faces[t + 0];
        Vertex* v2 = faces[t + 1];
        Vertex* v3 = faces[t + 2];
        Vec3 n = computeFaceNormal(v1->position, v3->position, v2->position);

        v1->normal = Vec3(v1->normal.x + n.x, v1->normal.y + n.y, v1->normal.z + n.z);
        v2->normal = Vec3(v2->normal.x + n.x, v2->normal.y + n.y, v2->normal.z + n.z);
        v3->normal = Vec3(v3->normal.x + n.x, v3->normal.y + n.y, v3->normal.z + n.z);

    }

    // normalize
    for (auto* v : vertexes) {
        float l = normv(v->normal);
        if (l > 1e-20f) v->normal = v->normal * (1.0f / l);
    }
}

void SweptSphere::init() {
    if (r0 > r1) {
        Vec3 c0Tmp = c0;
        float r0Tmp = r0;

        c0 = c1;
        r0 = r1;

        c1 = c0Tmp;
        r1 = r0Tmp;
    }

    for (auto* v : vertexes) delete v;
    vertexes.clear();
    faces.clear();

    // Geometria base
    Vec3 axis = c1 - c0;
    float L = normv(axis);

    // Caso degenerato: sfera unica del raggio max
    if (L < 1e-7f) {
        Vec3 X, Y, Z; makeOrthonormalFrame({ 0,0,1 }, X, Y, Z);
        float R = std::max(r0, r1);
        // Due cap da polo a equatore (come in Sphere)
        auto capTop = addSphericalCapPhi(c0, R, X, Y, Z, radialSegments, hemiStacks, +1.0f, 0.0f, float(M_PI) * 0.5f, false);
        auto capBottom = addSphericalCapPhi(c0, R, X, Y, Z, radialSegments, hemiStacks, -1.0f, 0.0f, float(M_PI) * 0.5f, false);
        // Cucitura tra l’ultimo ring del top e il primo del bottom
        stitchRingPairCCW(capTop, capBottom);
        computeNormals();
        return;
    }

    Vec3 X, Y, Z; makeOrthonormalFrame(axis, X, Y, Z);

    float k = (r1 - r0) / L;
    float kk = std::clamp(k, -0.9999f, 0.9999f);
    float phiMax = std::acos(std::fabs(kk));
    const float PI = 3.14159265358979323846f;

    std::vector<std::vector<int>> rings; rings.reserve(ringSegments + 1);
    for (int s = 0; s <= ringSegments; ++s) {
        float t = (float)s / (float)ringSegments;
        Vec3  C = c0 + (c1 - c0) * t;
        float R = r0 * (1.0f - t) + r1 * t;
        rings.push_back(addRingTangent(C, R, X, Y, Z, radialSegments, kk));
        if (s > 0) {
            stitchRingPairCCW(rings[s - 1], rings[s]);
        }
    }

    // Cap START: usa il polo con hemiSign = -sign(k), dal polo alla tangenza
    {
        float hemiStartSign = (kk >= 0.0f) ? -1.0f : +1.0f; // = -sign(k)
        auto capStartRings = addSphericalCapPhi(c0, r0, X, Y, Z, radialSegments, hemiStacks,
            hemiStartSign, 0.0f, phiMax,
            /*useCW=*/false); // CCW

        stitchRingPairCCW(capStartRings, rings.front());    // CCW
    }

    // Cap END: prendi l’ALTRA metà della sfera (verso il polo opposto)
    {
        float hemiEndSign = (kk >= 0.0f) ? +1.0f : -1.0f;  // = +sign(k)
        float phiSeam = PI - phiMax;
        auto capEndRings = addSphericalCapPhi(c1, r1, X, Y, Z, radialSegments, hemiStacks,
            hemiEndSign, 0.0f, phiSeam,
            /*useCW=*/true);  // CW
        stitchRingPairCW(rings.back(), capEndRings);        // CW
    }

    // Normali come nel tuo Sphere
    computeNormals();
}

void SweptSphere::stitchRingPairCW(const std::vector<int>& ringA, const std::vector<int>& ringB) {
    // Winding opposto a CCW: inverte l'orientamento delle facce
    int N = (int)ringA.size();
    for (int i = 0; i < N; i++) {
        int i0 = ringA[i];
        int i1 = ringA[(i + 1) % N];
        int j0 = ringB[i];
        int j1 = ringB[(i + 1) % N];

        // Tri 1 (reverse di CCW):
        faces.push_back(vertexes[i0]);
        faces.push_back(vertexes[j0]);
        faces.push_back(vertexes[i1]);

        // Tri 2 (reverse di CCW):
        faces.push_back(vertexes[i1]);
        faces.push_back(vertexes[j0]);
        faces.push_back(vertexes[j1]);
    }
}
