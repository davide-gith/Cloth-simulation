#ifndef CLOTHDDATA_H
#define CLOTHDDATA_H

#include <vector>
#include <string>
#include "Node.h"
#include "Spring.h"

class ClothData {
public:
    std::vector<Node*> nodes;
    std::vector<Spring*> springs;
    std::vector<Node*> faces;

    int nodesPerRow;
	int nodesPerCol;
    int nodesDensity;
    Vec3 clothPos;

    ClothData();
    ClothData(const std::string& shape);
    void BuildGrid();
    void BuildFromObj(const std::string& path);

    void computeNormal();
    Vec3 computeFaceNormal(Node* n1, Node* n2, Node* n3);

    //void pin();
    //void unpin();
    Node* getNode(int x, int y) const;
    Vec3 getWorldPos(Node* n) const;
    void setWorldPos(Node* n, Vec3 pos) const;

    ~ClothData();
};

#endif