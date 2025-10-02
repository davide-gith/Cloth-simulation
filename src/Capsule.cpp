#include "Capsule.h"
#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <algorithm>
#include <cmath>    

Capsule::Capsule(Vec3 topP, Vec3 bottomP, int r, glm::vec4 c)
    : topPoint(topP), bottomPoint(bottomP), radius(r), color(c)
{
    meridianNum = 24;
    parallelNum = 16;

    init();
}

Capsule::~Capsule()
{
    for (Vertex* v : vertexes) delete v;
    vertexes.clear();
    faces.clear();
}

Vertex* Capsule::getTop() {
    return vertexes[0];
}
Vertex* Capsule::getBottom() {
    return vertexes[(int)vertexes.size() - 1];
}

Vertex* Capsule::getRingVertex(int ringIndex, int meridianIndex) {
    int T = (parallelNum - 1) * meridianNum;
    int baseTopRings = 1;
    int baseTopEquator = baseTopRings + T;
    int baseBottomEquator = baseTopEquator + meridianNum;
    int baseBottomRings = baseBottomEquator + meridianNum;

    if (ringIndex >= 0 && ringIndex <= parallelNum - 2) {
        // top rings
        return vertexes[baseTopRings + ringIndex * meridianNum + meridianIndex];
    }
    else if (ringIndex == parallelNum - 1) {
        // top equator
        return vertexes[baseTopEquator + meridianIndex];
    }
    else if (ringIndex == parallelNum) {
        // bottom equator
        return vertexes[baseBottomEquator + meridianIndex];
    }
    else if (ringIndex >= parallelNum + 1 && ringIndex <= 2 * parallelNum - 1) {
        // bottom rings
        int i = ringIndex - (parallelNum + 1); // 0..(parallelNum-2)
        return vertexes[baseBottomRings + i * meridianNum + meridianIndex];
    }
    else {
        std::printf("Ring index out of range\n");
        std::exit(-1);
    }
}

Vec3 Capsule::faceNormal(Vertex* v1, Vertex* v2, Vertex* v3)
{
    return Vec3::cross(v2->position - v1->position, v3->position - v1->position);
}

//void Capsule::accumulateAndNormalizeNormals()
//{
//    for (Vertex* v : vertexes) v->normal = Vec3(0.0, 0.0, 0.0);
//
//    // (v1, v3, v2) mantiene coerenza con la tua mesh originale
//    for (size_t i = 0; i + 2 < faces.size(); i += 3) {
//        Vertex* v1 = faces[i];
//        Vertex* v2 = faces[i + 1];
//        Vertex* v3 = faces[i + 2];
//        Vec3 n = faceNormal(v1, v3, v2);
//        v1->normal += n; v2->normal += n; v3->normal += n;
//    }
//
//    for (Vertex* v : vertexes) v->normal.normalize();
//}

void Capsule::accumulateAndNormalizeNormals()
{
    Vec3 axis = bottomPoint - topPoint;
    double L = axis.length();
    if (L < 1e-12) L = 1.0;
    Vec3 n = axis / L;

    Vec3 c0 = centerTopHemisphere;
    double shaftLen = (std::max)(0.0, L - 2.0 * static_cast<double>(radius));

    for (Vertex* v : vertexes) {
        Vec3 p = v->position;

        Vec3 c0p(p.x - c0.x, p.y - c0.y, p.z - c0.z);
        double t = Vec3::dot(c0p, n);
        if (t < 0.0)           t = 0.0;
        else if (t > shaftLen) t = shaftLen;

        Vec3 a(c0.x + n.x * t, c0.y + n.y * t, c0.z + n.z * t);
        Vec3 N(p.x - a.x, p.y - a.y, p.z - a.z);

        double len = N.length();
        if (len < 1e-12) {
            Vec3 tx, ty;
            orthonormalBasis(n, tx, ty);
            N = tx;
        }
        else {
            N = Vec3(N.x / len, N.y / len, N.z / len);
        }
        v->normal = N;
    }
}

void Capsule::orthonormalBasis(const Vec3& n, Vec3& x, Vec3& y)
{
    Vec3 a = (std::fabs(n.x) < 0.9) ? Vec3(1.0, 0.0, 0.0) : Vec3(0.0, 1.0, 0.0);
    x = Vec3::cross(n, a);
    double lx = x.length();
    if (lx < 1e-12) {
        a = Vec3(0.0, 0.0, 1.0);
        x = Vec3::cross(n, a);
        lx = x.length();
    }
    x = x / lx;
    y = Vec3::cross(n, x);
    y.normalize();
}

void Capsule::addRing(const Vec3& ringCenter, double ringRadius, const Vec3& ux, const Vec3& uy)
{
    const double dTheta = 2.0 * M_PI / (double)meridianNum;
    for (int j = 0; j < meridianNum; ++j) {
        double theta = j * dTheta;
        Vec3 uxT = Vec3(ux.x * std::cos(theta), ux.y * std::cos(theta), ux.z * std::cos(theta));
        Vec3 uyT = Vec3(uy.x * std::sin(theta), uy.y * std::sin(theta), uy.z * std::sin(theta));
        Vec3 pos = Vec3(ringCenter.x + (uxT.x + uyT.x) * ringRadius, ringCenter.y + (uxT.y + uyT.y) * ringRadius, ringCenter.z + (uxT.z + uyT.z) * ringRadius);
        vertexes.push_back(new Vertex(pos));
    }
}

void Capsule::addFanFaces(Vertex* pole, int ringStartIndex) // ringStartIndex: indice base del ring nel vettore vertexes
{
    // ringStartIndex punta al primo vertice dell’anello (di meridianNum vertici)
    for (int j = 0; j < meridianNum; ++j) {
        Vertex* a = vertexes[ringStartIndex + j];
        Vertex* b = vertexes[ringStartIndex + ((j + 1) % meridianNum)];
        // Triangolo (a, pole, b) con winding coerente con le altre strip
        faces.push_back(a);
        faces.push_back(pole);
        faces.push_back(b);
    }
}

void Capsule::addStripFaces(int ringAStart, int ringBStart)
{
    for (int j = 0; j < meridianNum; ++j) {
        int jn = (j + 1) % meridianNum;
        Vertex* a0 = vertexes[ringAStart + j];
        Vertex* a1 = vertexes[ringAStart + jn];
        Vertex* b0 = vertexes[ringBStart + j];
        Vertex* b1 = vertexes[ringBStart + jn];

        // Due triangoli: (a0, a1, b0), (b1, b0, a1)
        faces.push_back(a0);
        faces.push_back(a1);
        faces.push_back(b0);

        faces.push_back(b1);
        faces.push_back(b0);
        faces.push_back(a1);
    }
}

void Capsule::init()
{
    // Direzione asse e lunghezze
    Vec3 axis = bottomPoint - topPoint;
    double L = axis.length();
    if (L < 1e-12) {
        std::printf("Capsule init error: topPoint and bottomPoint coincide.\n");
        std::exit(-1);
    }
    Vec3 n = axis / L;

    // Centri delle semisfere e lunghezza fusto
    centerTopHemisphere = topPoint + n * (double)radius;
    centerBottomHemisphere = bottomPoint - n * (double)radius;
    double shaftLen = std::max(0.0, L - 2.0 * (double)radius);

    // Base locale ortonormale per generare i cerchi
    Vec3 ux, uy;
    orthonormalBasis(n, ux, uy);

    // === VERTICI ===
    // Polo top
    vertexes.push_back(new Vertex(topPoint)); // idx 0

    // --- Semisfera superiore (anelli tra polo ed equatore, escluso equatore) ---
    // v in (0 .. 1), lat = -pi/2 + v*(pi/2) => (-pi/2 .. 0)
    if (parallelNum < 1) {
        std::printf("parallelNum must be >= 1\n");
        std::exit(-1);
    }
    const int hemiInner = parallelNum - 1; // anelli interni (senza equatore)

    for (int i = 1; i <= hemiInner; ++i) {
        double v = (double)i / (double)parallelNum;      // (0..1)
        double lat = -M_PI * 0.5 + v * (M_PI * 0.5);       // (-pi/2 .. 0)
        double r = (double)radius * std::cos(lat);
        double z = (double)radius * std::sin(lat);
        Vec3 ringCenter = centerTopHemisphere + n * z;
        addRing(ringCenter, r, ux, uy);
    }

    // --- Anello equatore superiore ---
    addRing(centerTopHemisphere, (double)radius, ux, uy);
    // Memorizza indice base del ring equatore top
    int baseTopEquator = 1 + (parallelNum - 1) * meridianNum;

    // --- Anello equatore inferiore ---
    // È lo stesso cerchio, traslato lungo n di shaftLen (ovvero centrato in centerBottomHemisphere)
    addRing(centerBottomHemisphere, (double)radius, ux, uy);
    int baseBottomEquator = baseTopEquator + meridianNum;

    // --- Semisfera inferiore (anelli interni tra equatore e polo) ---
    for (int i = 1; i <= hemiInner; ++i) {
        double v = (double)i / (double)parallelNum;     // (0..1)
        double lat = v * (M_PI * 0.5);                    // (0 .. pi/2)
        double r = (double)radius * std::cos(lat);
        double z = (double)radius * std::sin(lat);
        Vec3 ringCenter = centerBottomHemisphere + n * z;
        addRing(ringCenter, r, ux, uy);
    }

    // Polo bottom
    vertexes.push_back(new Vertex(bottomPoint));
    int bottomPoleIdx = (int)vertexes.size() - 1;

    // === FACCE ===
    if (hemiInner > 0) {
        int baseFirstTopRing = 1; // subito dopo il polo
        addFanFaces(getTop(), baseFirstTopRing);
    }
    else {
        // Se parallelNum == 1, non ci sono anelli interni: collega polo direttamente all’equatore top
        addFanFaces(getTop(), baseTopEquator);
    }

    // Strip semisfera top: tra anelli consecutivi fino all’equatore top
    if (hemiInner > 1) {
        for (int i = 0; i < hemiInner - 1; ++i) {
            int ringA = 1 + i * meridianNum;
            int ringB = 1 + (i + 1) * meridianNum;
            addStripFaces(ringA, ringB);
        }
        int lastInner = 1 + (hemiInner - 1) * meridianNum;
        addStripFaces(lastInner, baseTopEquator);
    }
    else if (hemiInner == 1) {
        // Un solo anello interno: collegalo all’equatore
        int onlyInner = 1;
        addStripFaces(onlyInner, baseTopEquator);
    }

    addStripFaces(baseTopEquator, baseBottomEquator);

    if (hemiInner > 0) {
        int baseFirstBottomRing = baseBottomEquator + meridianNum;
        addStripFaces(baseBottomEquator, baseFirstBottomRing);

        // anelli interni bottom tra loro
        for (int i = 0; i < hemiInner - 1; ++i) {
            int ringA = baseBottomEquator + meridianNum + i * meridianNum;
            int ringB = baseBottomEquator + meridianNum + (i + 1) * meridianNum;
            addStripFaces(ringA, ringB);
        }

        int baseLastBottomRing = baseBottomEquator + meridianNum + (hemiInner - 1) * meridianNum;

        for (int j = 0; j < meridianNum; ++j) {
            Vertex* a = vertexes[baseLastBottomRing + j];
            Vertex* b = vertexes[baseLastBottomRing + ((j + 1) % meridianNum)];
            faces.push_back(getBottom());
            faces.push_back(a);
            faces.push_back(b);
        }
    }
    else {
        for (int j = 0; j < meridianNum; ++j) {
            Vertex* a = vertexes[baseBottomEquator + j];
            Vertex* b = vertexes[baseBottomEquator + ((j + 1) % meridianNum)];
            faces.push_back(getBottom());
            faces.push_back(a);
            faces.push_back(b);
        }
    }

    accumulateAndNormalizeNormals();
}