#ifndef VECTORS_H
#define VECTORS_H

#include <math.h>
#include <vector>
#include "matrices.h"

struct Vec2
{
	double x;
	double y;

	Vec2(void)
	{
		x = 0.0;
		y = 0.0;
	}
	Vec2(double x0, double y0)
	{
		x = x0;
		y = y0;
	}
	~Vec2() {}

	Vec2 operator+(Vec2 v)
	{
		return Vec2(x + v.x, y + v.y);
	}
	Vec2 operator-(Vec2 v)
	{
		return Vec2(x - v.x, y - v.y);
	}

	void operator+=(Vec2 v)
	{
		x += v.x;
		y += v.y;
	}
	void operator-=(Vec2 v)
	{
		x -= v.x;
		y -= v.y;
	}

};

struct Vec3
{
	double x;
	double y;
	double z;

	Vec3(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}
	Vec3(double x0, double y0, double z0)
	{
		x = x0;
		y = y0;
		z = z0;
	}
	~Vec3() {}

	static Vec3 cross(Vec3 v1, Vec3 v2)
	{
		return Vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
	}
	static double dot(Vec3 v1, Vec3 v2)
	{
		return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
	}
	static Mat3x3 outer(const Vec3& v1, const Vec3& v2)
	{
		return Mat3x3{
			v1.x * v2.x, v1.x * v2.y, v1.x * v2.z,
			v1.y * v2.x, v1.y * v2.y, v1.y * v2.z,
			v1.z * v2.x, v1.z * v2.y, v1.z * v2.z
		};
	}



	static double dist(Vec3 v1, Vec3 v2)
	{
		return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
	}

	Vec3 minus()
	{
		return Vec3(-x, -y, -z);
	}

	Vec3 operator+(Vec3 v)
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}
	Vec3 operator-(Vec3 v)
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}


	void operator+=(Vec3 v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator-=(Vec3 v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	Vec3 operator*(double n)
	{
		return Vec3(x * n, y * n, z * n);
	}
	Vec3 operator/(double n)
	{
		return Vec3(x / n, y / n, z / n);
	}

	bool operator ==(Vec3& v)
	{
		return x == v.x && y == v.y && z == v.z;
	}
	bool operator!=(Vec3& v)
	{
		return x != v.x || y != v.y || z != v.z;
	}

	double length()
	{
		return sqrt(x * x + y * y + z * z);
	}

	void normalize()
	{
		double w = length();
		if (w < 0.00001) return;

		x /= w;
		y /= w;
		z /= w;
	}

	void setZeroVec() {
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}
};


struct Triangle
{
	Vec3 a;
	Vec3 b;
	Vec3 c;

	Triangle(void)
	{
		a = Vec3(0.0, 0.0, 0.0);
		b = Vec3(0.0, 0.0, 0.0);
		c = Vec3(0.0, 0.0, 0.0);
	}
	Triangle(Vec3 a0, Vec3 b0, Vec3 c0)
	{
		a = a0;
		b = b0;
		c = c0;
	}
	~Triangle() {}


	double area() {
		Vec3 e1 = a - b;
		Vec3 e2 = a - c;
		Vec3 cr = cr.cross(e1, e2);

		return (cr.length() / 2);


	}

	Vec3 centroid() {
		return  Vec3((a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3, (a.z + b.z + c.z) / 3);
	}

	Vec3 normal() {
		Vec3 e1 = b - a;
		Vec3 e2 = c - a;

		Vec3 n = n.cross(e1, e2);
		n.normalize();

		if (n.x == -0.0) { n.x = 0.0; }
		if (n.y == -0.0) { n.y = 0.0; }
		if (n.z == -0.0) { n.z = 0.0; }

		return n;
	}

	void print_triangle(int pos) {
		printf("------------- TRIANGLE %i -------------\n", pos);
		printf("Vertex a: (%f, %f, %f)\n", a.x, a.y, a.z);
		printf("Vertex b: (%f, %f, %f)\n", b.x, b.y, b.z);
		printf("Vertex c: (%f, %f, %f)\n", c.x, c.y, c.z);
		printf("\n");
	}
};

#endif// VECTORS_H