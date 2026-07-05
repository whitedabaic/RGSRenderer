#pragma once
#include "Ray.h"

class Triangle
{
public:
	Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Matrix4x4& worldMatrix);
	bool Intersect(Ray& ray, Intersection& isect) const;

private:
	Vector3f mVertices[3];
	Vector3f mNormal;
};