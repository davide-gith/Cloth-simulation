#ifndef SPHERE_MESHES_COLLIDER_H
#define SPHERE_MESHES_COLLIDER_H

#include <glm/glm.hpp>
#include <string>
#include "ICollider.h"
#include "Vectors.h"
#include "Display.h"
#include "SweptSphereCollider.h"

class SphereMeshesCollider : public ICollider {
private:
    std::string path;
public:
    std::vector<ICollider*> primitives;

    SphereMeshesCollider(const std::string& filepath);
    void init(const std::string& filepath, const double scale, const Vec3& translation);
    void resolveCollision(ClothData* data) override;
    void render() override;
    ~SphereMeshesCollider();
};

#endif