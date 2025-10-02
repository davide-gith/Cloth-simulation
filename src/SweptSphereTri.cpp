#include "SweptSphereTri.h"
#include "SweptSphere.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>

// ===== utilità math locali =====
namespace {
    inline double dotv(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    inline Vec3   crossv(const Vec3& a, const Vec3& b) {
        return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }
    inline double normv(const Vec3& v) { return std::sqrt(dotv(v, v)); }
    inline Vec3   normalizev(const Vec3& v) {
        double n = normv(v);
        return (n > 0.0) ? Vec3(v.x / n, v.y / n, v.z / n) : Vec3(0, 0, 0);
    }
}

// ===== ctor/dtor =====
SweptSphereTri::SweptSphereTri(const Vec3& C0, float R0,
    const Vec3& C1, float R1,
    const Vec3& C2, float R2,
    const glm::vec4& col,
    int radial, int Rings, int Stacks)
    : radialSegments(std::max(3, radial))
    , rings(std::max(1, Rings))
    , stacks(std::max(2, Stacks))
    , c0(C0), c1(C1), c2(C2)
    , r0(R0), r1(R1), r2(R2)
    , color(col)
{
    init();
}

SweptSphereTri::~SweptSphereTri() {
    for (auto* v : vertexes) delete v;
    vertexes.clear();
    faces.clear();
}

// ===== helpers =====
void SweptSphereTri::appendMeshDeep(const std::vector<Vertex*>& srcV,
    const std::vector<Vertex*>& srcF)
{
    std::unordered_map<const Vertex*, Vertex*> remap;
    remap.reserve(srcV.size());

    // deep-copy vertici
    for (auto* v : srcV) {
        Vertex* nv = new Vertex(v->position);
        // FIX: tutte le normali NON-TRIANGOLO devono essere ENTRANTI -> flip qui
        nv->normal = v->normal * (-1.0);
        vertexes.push_back(nv);
        remap[v] = nv;
    }

    // rimappa facce con FLIP del winding (per tutte le superfici importate)
    faces.reserve(faces.size() + srcF.size());
    const size_t n = srcF.size();
    if (n % 3 != 0) {
        // fallback robusto: se non è multiplo di 3, rimappa uno a uno (comunque flippo a terne quando possibile)
        for (size_t i = 0; i + 2 < n; i += 3) {
            Vertex* a = remap[srcF[i]];
            Vertex* b = remap[srcF[i + 1]];
            Vertex* c = remap[srcF[i + 2]];
            // flip winding: (a,b,c) -> (a,c,b)
            faces.push_back(a);
            faces.push_back(c);
            faces.push_back(b);
        }
        // eventuali elementi spaiati (rarissimo) li aggiungo senza triangolare
        for (size_t i = (n / 3) * 3; i < n; ++i) {
            auto it = remap.find(srcF[i]);
            if (it != remap.end()) faces.push_back(it->second);
        }
    }
    else {
        for (size_t i = 0; i < n; i += 3) {
            Vertex* a = remap[srcF[i]];
            Vertex* b = remap[srcF[i + 1]];
            Vertex* c = remap[srcF[i + 2]];
            // flip winding: (a,b,c) -> (a,c,b)
            faces.push_back(a);
            faces.push_back(c);
            faces.push_back(b);
        }
    }
}

void SweptSphereTri::facePatchNormal(const Vec3& c0, float r0,
    const Vec3& c1, float r1,
    const Vec3& c2, float r2,
    Vec3& n_out)
{
    // Base (E1,E2,N) sul triangolo dei centri
    Vec3 e01 = Vec3(c1.x - c0.x, c1.y - c0.y, c1.z - c0.z);
    double L01 = normv(e01);
    if (L01 < 1e-12) { n_out = Vec3(0, 0, 1); return; }
    Vec3 E1 = e01 / L01;

    Vec3 v2 = Vec3(c2.x - c0.x, c2.y - c0.y, c2.z - c0.z);
    double a2 = dotv(v2, E1);
    Vec3 E2perp = v2 - E1 * a2;
    double b2 = normv(E2perp);
    if (b2 < 1e-12) {
        // quasi colineare: scegli E2 ortogonale a E1
        Vec3 tmp = (std::fabs(E1.z) < 0.999) ? Vec3(0, 0, 1) : Vec3(0, 1, 0);
        E2perp = crossv(tmp, E1);
        b2 = normv(E2perp);
        if (b2 < 1e-12) { n_out = Vec3(0, 0, 1); return; }
    }
    Vec3 E2 = E2perp / b2;
    Vec3 N = crossv(E1, E2); // unit già con E1,E2 ortonormali

    // Gradiente del raggio nel piano
    double g1 = (r1 - r0) / std::max(L01, 1e-12);
    double g2 = (r2 - r0 - g1 * a2) / std::max(b2, 1e-12);
    double G2 = g1 * g1 + g2 * g2;
    if (G2 > 1.0) { // clamp per garantire soluzione reale
        double s = std::sqrt((1.0 - 1e-6) / G2);
        g1 *= s; g2 *= s; G2 = g1 * g1 + g2 * g2;
    }
    double gamma = std::sqrt(std::max(0.0, 1.0 - G2));
    Vec3 n = E1 * (-g1) + E2 * (-g2) + N * gamma;

    // orienta verso l'esterno coerente con l'ordine (c0,c1,c2)
    double sign = dotv(crossv(Vec3(c1.x - c0.x, c1.y - c0.y, c1.z - c0.z), Vec3(c2.x - c0.x, c2.y - c0.y, c2.z - c0.z)), n);
    if (sign < 0.0) n = n * (-1.0);

    n_out = normalizev(n);
}

void SweptSphereTri::addFacePatch(const Vec3& c0, float r0,
    const Vec3& c1, float r1,
    const Vec3& c2, float r2)
{
    Vec3 n; facePatchNormal(c0, r0, c1, r1, c2, r2, n);

    Vec3 nr0 = n * (double)r0;
    Vec3 nr1 = n * (double)r1;
    Vec3 nr2 = n * (double)r2;
    Vertex* v0 = new Vertex(Vec3(c0.x + nr0.x, c0.y + nr0.y, c0.z + nr0.z));
    Vertex* v1 = new Vertex(Vec3(c1.x + nr1.x, c1.y + nr1.y, c1.z + nr1.z));
    Vertex* v2 = new Vertex(Vec3(c2.x + nr2.x, c2.y + nr2.y, c2.z + nr2.z));

    // TRIANGOLI: invariati (normali uscenti)
    v0->normal = n; v1->normal = n; v2->normal = n;

    vertexes.push_back(v0);
    vertexes.push_back(v1);
    vertexes.push_back(v2);

    // ----- Lato -n -----
    Vec3 nm = n * (-1.0);
    Vec3 nr0m = nm * (double)r0;
    Vec3 nr1m = nm * (double)r1;
    Vec3 nr2m = nm * (double)r2;
    Vec3 q0 = Vec3(c0.x + nr0m.x, c0.y + nr0m.y, c0.z + nr0m.z);
    Vec3 q1 = Vec3(c1.x + nr1m.x, c1.y + nr1m.y, c1.z + nr1m.z);
    Vec3 q2 = Vec3(c2.x + nr2m.x, c2.y + nr2m.y, c2.z + nr2m.z);

    Vertex* w0 = new Vertex(q0);
    Vertex* w1 = new Vertex(q1);
    Vertex* w2 = new Vertex(q2);
    w0->normal = nm; w1->normal = nm; w2->normal = nm;

    vertexes.push_back(w0);
    vertexes.push_back(w1);
    vertexes.push_back(w2);

    // Per ottenere normale -n, usa winding invertito rispetto al lato +n
    // (così rispetti la regola della mano destra).
    faces.push_back(w0);
    faces.push_back(w2);
    faces.push_back(w1);
}

// ===== build =====
void SweptSphereTri::init()
{
    // Pulisci eventuale mesh precedente
    for (auto* v : vertexes) delete v;
    vertexes.clear();
    faces.clear();

    // 1) Bordi: 3 swept-sphere (riuso della tua classe)
    SweptSphere ab(c0, r0, c1, r1, color, radialSegments, rings, stacks);
    SweptSphere bc(c1, r1, c2, r2, color, radialSegments, rings, stacks);
    SweptSphere ca(c2, r2, c0, r0, color, radialSegments, rings, stacks);

    // FIX: durante l'append faccio flip di normali e winding
    appendMeshDeep(ab.vertexes, ab.faces);
    appendMeshDeep(bc.vertexes, bc.faces);
    appendMeshDeep(ca.vertexes, ca.faces);

    // 2) Faccia planare gonfiata (triangoli invariati)
    addFacePatch(c0, r0, c1, r1, c2, r2);
}
