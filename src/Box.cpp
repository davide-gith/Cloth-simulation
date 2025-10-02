#include "Box.h"
#include "glm/glm.hpp"
#include "Vertex.h"
#include "Vectors.h"

Box::Box(const Vec3& c, const Vec3& halfExtent, const glm::vec4& col)
	: center(c), extent(halfExtent), color(col)
{
	init();
}

Box::~Box()
{
	for (auto v : vertexes) delete v;
}

Vec3 Box::getCenter() const {
	return center;
}

Vec3 Box::getExtent() const {
	return extent;
}

Vec3 Box::computeFacenormal(Vertex* v1, Vertex* v2, Vertex* v3)
{
	return Vec3::cross(v3->position - v1->position,
		v2->position - v1->position);
}

void Box::computeNormals()
{
	// Azzera
	for (auto v : vertexes)
		v->normal.setZeroVec();

	// Accumula e normalizza (ora fa smooth shading)
	int triCount = (int)faces.size() / 3;
	for (int i = 0; i < triCount; ++i)
	{
		Vertex* v1 = faces[i * 3 + 0];
		Vertex* v2 = faces[i * 3 + 1];
		Vertex* v3 = faces[i * 3 + 2];
		Vec3 n = computeFacenormal(v1, v3, v2);
		v1->normal += n;
		v2->normal += n;
		v3->normal += n;
	}
	for (auto v : vertexes)
		v->normal.normalize();
}

void Box::addFace(Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3)
{
	// due triangoli CCW
	faces.push_back(v0); faces.push_back(v1); faces.push_back(v2);
	faces.push_back(v0); faces.push_back(v2); faces.push_back(v3);
}

Vertex* Box::makeVertex(const Vec3& p)
{
	Vertex* v = new Vertex(p);
	vertexes.push_back(v);
	return v;
}

void Box::init()
{
	const double ex = extent.x, ey = extent.y, ez = extent.z;

	// 8 angoli dell'AABB
	Vec3 c000 = center + Vec3(-ex, -ey, -ez);
	Vec3 c001 = center + Vec3(-ex, -ey, +ez);
	Vec3 c010 = center + Vec3(-ex, +ey, -ez);
	Vec3 c011 = center + Vec3(-ex, +ey, +ez);
	Vec3 c100 = center + Vec3(+ex, -ey, -ez);
	Vec3 c101 = center + Vec3(+ex, -ey, +ez);
	Vec3 c110 = center + Vec3(+ex, +ey, -ez);
	Vec3 c111 = center + Vec3(+ex, +ey, +ez);

	// Crea un solo vertice per angolo
	Vertex* v000 = makeVertex(c000);
	Vertex* v001 = makeVertex(c001);
	Vertex* v010 = makeVertex(c010);
	Vertex* v011 = makeVertex(c011);
	Vertex* v100 = makeVertex(c100);
	Vertex* v101 = makeVertex(c101);
	Vertex* v110 = makeVertex(c110);
	Vertex* v111 = makeVertex(c111);

	// 6 facce (ognuna con 2 triangoli)
	addFace(v100, v110, v111, v101);

	// -X face 
	//addFace(v000, v001, v011, v010);
	addFace(v011, v001, v000, v010);

	//// +Y face
	//addFace(v010, v011, v111, v110);
	addFace(v111, v011, v010, v110);

	//// -Y face 
	addFace(v000, v100, v101, v001);

	//// +Z face 
	addFace(v001, v101, v111, v011);
	//addFace(v111, v101, v001, v011);

	//// -Z face 
	addFace(v000, v010, v110, v100);

	// Calcola le normali
	computeNormals();
}