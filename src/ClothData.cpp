#include <omp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "ClothData.h"
#include "Node.h"
#include "Spring.h"
#include "Vectors.h"

ClothData::ClothData() {};

ClothData::ClothData(const std::string& shape) {
    if (shape == "grid")
        BuildGrid();
    if (shape == "tshirt")
		BuildFromObj("Mesh/tshirt.obj"); 

}

void ClothData::BuildGrid() {
    clothPos = Vec3(0.0, 7.0, 0.0);
    nodesPerRow = 40;
    nodesPerCol = 40;
    nodesDensity = 4;

    // Nodes
    for (int i = 0; i < nodesPerRow; ++i) {
        for (int j = 0; j < nodesPerCol; ++j) {
			// texture coordinates
            float u = (float)j / (nodesPerCol - 1); 
            float v = (float)i / (nodesPerRow - 1);
            
            Vec2 uv = Vec2(u,v);
            unsigned int id = i * nodesPerRow + j;
            Vec3 pos = Vec3((double)j / nodesDensity, 0, -((double)i / nodesDensity));

            nodes.push_back(new Node(id,pos,uv));
        }
    }

    // Springs
    //0: quad springs
    //1: first diagonal spring (/)
    //2: second diagonal spring (/)
    //3: quad springs
    for (int i = 0; i < nodesPerRow; i++) {
        for (int j = 0; j < nodesPerCol; j++) {
			// Quad springs
            if (i < nodesPerRow - 1) springs.push_back(new Spring(0, getNode(i, j), getNode(i + 1, j), 0.0f));
            if (j < nodesPerCol - 1) springs.push_back(new Spring(0, getNode(i, j), getNode(i, j + 1), 0.0f));
            
			// Diagonal springs
            if (i < nodesPerRow - 1 && j < nodesPerCol - 1) {
                springs.push_back(new Spring(1, getNode(i + 1, j), getNode(i, j + 1), 0.0f));
                springs.push_back(new Spring(2, getNode(i, j), getNode(i + 1, j + 1), 0.0f));
            }

			// Additional quad springs
            if (i < nodesPerRow - 2) springs.push_back(new Spring(3, getNode(i, j), getNode(i + 2, j), 0.0f));
            if (j < nodesPerCol - 2) springs.push_back(new Spring(3, getNode(i, j), getNode(i, j + 2), 0.0f));
        }
    }

    // Triangle faces
    for (int i = 0; i < nodesPerRow - 1; ++i) {
        for (int j = 0; j < nodesPerCol - 1; ++j) {
            // Left upper triangle
            faces.push_back(getNode(i + 1, j));
            faces.push_back(getNode(i, j));
            faces.push_back(getNode(i, j + 1));
            // Right bottom triangle
            faces.push_back(getNode(i + 1, j + 1));
            faces.push_back(getNode(i + 1, j));
            faces.push_back(getNode(i, j + 1));
        }
    }

    // === Tangent calculation ===
    for (size_t i = 0; i < faces.size(); i += 3) {
        Node* n0 = faces[i];
        Node* n1 = faces[i + 1];
        Node* n2 = faces[i + 2];

        Vec3 p0 = n0->position;
        Vec3 p1 = n1->position;
        Vec3 p2 = n2->position;

        Vec2 uv0 = n0->texCoord;
        Vec2 uv1 = n1->texCoord;
        Vec2 uv2 = n2->texCoord;

        Vec3 edge1 = p1 - p0;
        Vec3 edge2 = p2 - p0;
        Vec2 deltaUV1 = uv1 - uv0;
        Vec2 deltaUV2 = uv2 - uv0;

        float f = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (fabs(f) < 1e-6f) f = 1.0f;
        else f = 1.0f / f;

        Vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        for (int j = 0; j < 3; ++j) {
            Node* n = faces[i + j];
            n->tangentSum += tangent;
            n->tangentCount++;
        }
    }

    for (Node* node : nodes) {
        if (node->tangentCount > 0) {
            node->tangent = node->tangentSum / node->tangentCount;
            node->tangent.normalize();
        }
        else {
            node->tangent = Vec3(1.0f, 0.0f, 0.0f); // fallback
        }
    }

    nodes[0]->isFixed = true;
    nodes[nodesPerRow - 1]->isFixed = true;

    std::cout << "Grid: " << std::endl;
    std::cout << "- nodes: " << nodes.size() << std::endl;
    std::cout << "- springs: " << springs.size() << std::endl;
}

//void ClothData::BuildFromObj(const std::string& path) {
//    // Non used for obj construction ---
//    clothPos = Vec3(0.0, 0.0, 0.0);
//    nodesPerRow = 0;
//    nodesPerCol = 0;
//    nodesDensity = 0;
//    // ---------------------
//
//    std::ifstream file(path);
//    if (!file.is_open()) {
//        std::cerr << "Errore apertura file: " << path << std::endl;
//        return;
//    }
//
//    std::vector<Vec3> positions;
//    std::vector<std::vector<int>> facesIndices;
//
//    std::string line;
//    while (std::getline(file, line)) {
//        std::stringstream ss(line);
//        std::string prefix;
//        ss >> prefix;
//
//        if (prefix == "v") {
//            float x, y, z;
//            ss >> x >> y >> z;
//            positions.emplace_back(x, y, z);
//        }
//        else if (prefix == "f") {
//            std::vector<int> face;
//            std::string token;
//            while (ss >> token) {
//                std::stringstream fs(token);
//                std::string indexStr;
//                std::getline(fs, indexStr, '/');
//                face.push_back(std::stoi(indexStr) - 1);
//            }
//            if (face.size() == 3)
//                facesIndices.push_back(face);
//        }
//    }
//
//    // Nodes
//    for (const Vec3& pos : positions) {
//        nodes.push_back(new Node(pos));
//    }
//
//    // Faces and springs
//    for (int i = 0; i < facesIndices.size(); i++) {
//        faces.push_back(nodes[facesIndices[i][0]]);
//        faces.push_back(nodes[facesIndices[i][1]]);
//        faces.push_back(nodes[facesIndices[i][2]]);
//
//        springs.push_back(new Spring(0, nodes[facesIndices[i][0]], nodes[facesIndices[i][1]], 0.0f));
//        springs.push_back(new Spring(0, nodes[facesIndices[i][1]], nodes[facesIndices[i][2]], 0.0f));
//        springs.push_back(new Spring(0, nodes[facesIndices[i][2]], nodes[facesIndices[i][0]], 0.0f));
//    }
//
//    nodes[0]->isFixed = true;
//    nodes[10]->isFixed = true;
//
//    std::cout << "Tshirt: " << std::endl;
//    std::cout << "- nodes: " << nodes.size() << std::endl;
//    std::cout << "- springs: " << springs.size() << std::endl;
//}

void ClothData::BuildFromObj(const std::string& path) {
    clothPos = Vec3(0.0, 0.0, 0.0);
    nodesPerRow = 0;
    nodesPerCol = 0;
    nodesDensity = 0;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Errore apertura file: " << path << std::endl;
        return;
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> texcoords;
    std::vector<std::vector<std::pair<int, int>>> faceIndices; // 3 faces 

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        }
        else if (prefix == "vt") {
            float u, v;
            ss >> u >> v;
            texcoords.emplace_back(u, v);
        }
        else if (prefix == "f") { // f v1/vt1 v2/vt2 v3/vt3
			std::vector<std::pair<int, int>> point; // pair<vertex index, texcoord index>
            std::string token;
            while (ss >> token) {
                std::stringstream fs(token);
                std::string vStr, vtStr;
                std::getline(fs, vStr, '/');
                std::getline(fs, vtStr, '/');

                int vIdx = std::stoi(vStr) - 1;
                int vtIdx = vtStr.empty() ? -1 : std::stoi(vtStr) - 1;
                point.emplace_back(vIdx, vtIdx);
            }
            if (point.size() == 3) {
                faceIndices.push_back(point);
            }
        }
    }

    // Map to ensure unique vertex per (position, texcoord)
    std::map<std::pair<int, int>, Node*> uniqueNodes;

    for (const auto& face : faceIndices) {
        Node* tri[3];
        for (int i = 0; i < 3; ++i) {
            auto key = face[i];
            if (uniqueNodes.count(key) == 0) {  // Determine whether a point has been instantiated
                Vec3 pos = positions[key.first];
                Node* node = new Node(pos);
				if (key.second >= 0 && key.second < texcoords.size()) { // Determine whether the texture coordinates are valid
                    node->texCoord = texcoords[key.second];
                }
                uniqueNodes[key] = node;
                nodes.push_back(node);
            }
            tri[i] = uniqueNodes[key];
        }

        faces.push_back(tri[0]);
        faces.push_back(tri[1]);
        faces.push_back(tri[2]);

        springs.push_back(new Spring(0, tri[0], tri[1], 0.0f));
        springs.push_back(new Spring(0, tri[1], tri[2], 0.0f));
        springs.push_back(new Spring(0, tri[2], tri[0], 0.0f));

		// calculate tangent
        Vec3 p0 = tri[0]->position;
        Vec3 p1 = tri[1]->position;
        Vec3 p2 = tri[2]->position;

        Vec2 uv0 = tri[0]->texCoord;
        Vec2 uv1 = tri[1]->texCoord;
        Vec2 uv2 = tri[2]->texCoord;

        Vec3 edge1 = p1 - p0;
        Vec3 edge2 = p2 - p0;
        Vec2 deltaUV1 = uv1 - uv0;
        Vec2 deltaUV2 = uv2 - uv0;

        /* 
            [Edge1 Edge2] = [T B] * |uv1.x uv2.x|
									|uv1.y uv2.y|

			[T B] = [Edge1 Edge2] * |uv1.x uv2.x|^-1
			                        |uv1.y uv2.y|

        */
        float f = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (fabs(f) < 1e-6f) f = 1.0f;
        /*
        1 / det(|uv1.x uv2.x|
                |uv1.y uv2.y|)
        */
        else f = 1.0f / f;

        Vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        for (int i = 0; i < 3; ++i) {
            tri[i]->tangentSum += tangent;
            tri[i]->tangentCount++;
        }

    }

	// Normalize tangents
    for (auto node : nodes) {
        if (node->tangentCount > 0) {
            node->tangent = node->tangentSum / node->tangentCount;
            node->tangent.normalize();
        }
        else {
            node->tangent = Vec3(1.0f, 0.0f, 0.0f); // fallback
        }
    }

    if (!nodes.empty()) nodes[0]->isFixed = true;
    if (nodes.size() > 10) nodes[10]->isFixed = true;

    std::cout << "Tshirt OBJ Loaded:" << std::endl;
    std::cout << "- nodes: " << nodes.size() << std::endl;
    std::cout << "- springs: " << springs.size() << std::endl;
}

Node* ClothData::getNode(int x, int y) const {
    return nodes[y * nodesPerRow + x];
}

Vec3 ClothData::getWorldPos(Node* n) const {
    return Vec3(clothPos.x + n->position.x, clothPos.y + n->position.y, clothPos.z + n->position.z);
}

void ClothData::setWorldPos(Node* n, Vec3 pos) const {
    n->position = pos - Vec3(0.0, 7.0, 0.0);
}

Vec3 ClothData::computeFaceNormal(Node* n1, Node* n2, Node* n3) {
    return Vec3::cross(n2->position - n1->position, n3->position - n1->position);
}

void ClothData::computeNormal()
{
    /** Reset nodes' normal **/
    Vec3 normal(0.0, 0.0, 0.0);

#pragma omp parallel for
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->normal = normal;
    }
    /** Compute normal of each face **/
    for (int i = 0; i < faces.size() / 3; i++) { // 3 nodes in each face
        Node* n1 = faces[3 * i + 0];
        Node* n2 = faces[3 * i + 1];
        Node* n3 = faces[3 * i + 2];

        // Face normal
        normal = computeFaceNormal(n1, n2, n3);
        // Add all face normal
        n1->normal += normal;
        n2->normal += normal;
        n3->normal += normal;
    }
    #pragma omp parallel for
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->normal.normalize();
    }
}

ClothData::~ClothData() {
	for (Node* node : nodes) {
		delete node;
	}
	for (Spring* spring : springs) {
		delete spring;
	}
	nodes.clear();
	springs.clear();
	faces.clear();
}