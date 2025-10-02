#include "SphereMeshesCollider.h"
#include "SweptSphereCollider.h"
#include "SweptSphereTriCollider.h"
#include <vector>
#include <string>
#include "Display.h"

SphereMeshesCollider::SphereMeshesCollider(const std::string& filepath) : path(filepath){
	init(path, 10.0, Vec3(3.0f, 0.0f, -2.0));
};

void SphereMeshesCollider::init(const std::string& filepath, const double scale, const Vec3& translation)
{
    // 0) Pulisci eventuali primitive già allocate (unico vettore di ICollider*)
    for (auto* p : primitives) delete p;
    primitives.clear();

    std::ifstream in(filepath);
    if (!in) {
        std::fprintf(stderr, "Errore: impossibile aprire \"%s\"\n", filepath.c_str());
        return;
    }

    // helper: riga vuota o commento ('#' come primo non-spazio)
    auto isCommentOrEmpty = [](const std::string& s) -> bool {
        for (unsigned char ch : s) {
            if (std::isspace(ch)) continue;
            return ch == '#';
        }
        return true;
        };

    // 1) Header: numSpheres, numSweptSphereTri, numSweptSphere
    std::string line;
    int nSpheres = -1, nTriDecl = -1, nEdgeDecl = -1;
    while (std::getline(in, line)) {
        if (!isCommentOrEmpty(line)) {
            std::istringstream iss(line);
            if (!(iss >> nSpheres >> nTriDecl >> nEdgeDecl) || nSpheres <= 0 || nTriDecl < 0 || nEdgeDecl < 0) {
                std::fprintf(stderr, "Errore: prima riga non valida (atteso: numSpheres>0 numTri>=0 numEdges>=0)\n");
                return;
            }
            break;
        }
    }
    if (nSpheres <= 0) {
        std::fprintf(stderr, "Errore: file privo di numero di sfere valido\n");
        return;
    }

    // 2) Leggi nSpheres righe di (cx, cy, cz, r)
    std::vector<Vec3>  centers; centers.reserve(nSpheres);
    std::vector<float> radii;   radii.reserve(nSpheres);

    int spheres_read = 0;
    while (spheres_read < nSpheres && std::getline(in, line)) {
        if (isCommentOrEmpty(line)) continue;
        std::istringstream iss(line);
        double cx, cy, cz, r;
        if (!(iss >> cx >> cy >> cz >> r)) {
            std::fprintf(stderr, "Errore: riga %d dei centri/raggi non valida\n", spheres_read + 1);
            return;
        }
        // Scala + trasla
        cx = cx * scale + translation.x;
        cy = cy * scale + translation.y;
        cz = cz * scale + translation.z;
        r = r * scale;

        centers.emplace_back(cx, cy, cz);
        radii.emplace_back(static_cast<float>(r));
        ++spheres_read;
    }
    if (spheres_read != nSpheres) {
        std::fprintf(stderr, "Errore: attese %d righe di centri/raggi, lette %d\n", nSpheres, spheres_read);
        return;
    }

    // 3) Leggi collegamenti fino a EOF
    //    Ogni riga può avere 2 o 3 indici (anche ripetuti sulla stessa riga).
    //    Regola: consuma triple finché possibile, poi un’eventuale coppia finale.
    size_t trisCreated = 0;
    size_t edgesCreated = 0;
    int    lineNo = nSpheres + 2;

    while (std::getline(in, line)) {
        ++lineNo;
        if (isCommentOrEmpty(line)) continue;

        std::istringstream iss(line);
        std::vector<int> idx;
        int v;
        while (iss >> v) idx.push_back(v);
        if (idx.empty()) continue;

        size_t pos = 0;
        // Consuma triple (triangoli)
        while (pos + 2 < idx.size()) {
            // Se restano esattamente 2 numeri, esci e tratta come edge sotto
            if ((idx.size() - pos) == 2) break;

            int a = idx[pos], b = idx[pos + 1], c = idx[pos + 2];
            if (a < 0 || a >= nSpheres || b < 0 || b >= nSpheres || c < 0 || c >= nSpheres) {
                std::fprintf(stderr, "Avviso: riga %d (tri) con indici fuori range: %d %d %d (n=%d)\n",
                    lineNo, a, b, c, nSpheres);
                pos += 3; // salta la tripletta invalida e continua
                continue;
            }

            ICollider* tri = new SweptSphereTriCollider(
                centers[a], radii[a],
                centers[b], radii[b],
                centers[c], radii[c]
            );

            primitives.push_back(tri);
            ++trisCreated;
            pos += 3;
        }

        // Se rimane una coppia, trattala come edge
        if (pos + 1 < idx.size()) {
            if ((idx.size() - pos) == 2) {
                int a = idx[pos], b = idx[pos + 1];
                if (a < 0 || a >= nSpheres || b < 0 || b >= nSpheres) {
                    std::fprintf(stderr, "Avviso: riga %d (edge) con indici fuori range: %d %d (n=%d)\n",
                        lineNo, a, b, nSpheres);
                }
                else {
                    ICollider* col = new SweptSphereCollider(
                        centers[a], radii[a],
                        centers[b], radii[b]
                    );
                    primitives.push_back(col);
                    ++edgesCreated;
                }
                pos += 2;
            }
            else {
                // numeri residui non multipli di 2 o 3
                std::fprintf(stderr, "Avviso: riga %d ignorata parzialmente (residuo di %zu interi non interpretabili)\n",
                    lineNo, idx.size() - pos);
            }
        }

		//renderer = new SphereMeshesRender(totalFaces);
    }

    std::fprintf(stdout,
        "Sphere-mesh caricata:\n"
        "  sfere: %d\n"
        "  tri dichiarati: %d, creati: %zu\n"
        "  edge dichiarati: %d, creati: %zu\n"
        "  colliders totali: %zu\n",
        nSpheres, nTriDecl, trisCreated, nEdgeDecl, edgesCreated, primitives.size());
}

void SphereMeshesCollider::resolveCollision(ClothData* data) {
    for (auto* p : primitives) {
        p->resolveCollision(data);
    }
};

void SphereMeshesCollider::render() {
    for (auto* p : primitives) {
        p->render();
    }
}

SphereMeshesCollider::~SphereMeshesCollider() {
    for (ICollider* s : primitives) {
        delete s;
    }
};