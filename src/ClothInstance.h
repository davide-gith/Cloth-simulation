#ifndef CLOTHINSTANCE_H
#define CLOTHINSTANCE_H

#include <string>
#include <vector>
#include "ClothData.h"
#include "IClothSimulator.h"
#include "Icollider.h"
#include <memory>

class ClothInstance {
public:
    enum DrawModeEnum {
        DRAW_NODES,
        DRAW_LINES,
        DRAW_FACES
    };
    DrawModeEnum drawMode = DRAW_FACES;

    static ClothInstance* create(const std::string& shape, const std::string& method);

    void update();
    void unpin();
    void computeNormal();
	void restart();
    ClothData* getData();
    IClothSimulator* getSimulator();
    std::vector<ICollider*> getColliders();
    void addCollider(ICollider* col);

private:
    ClothInstance(ClothData* data, IClothSimulator* simulator);

    ClothData* data;
    IClothSimulator* simulator;
};

#endif