#pragma once
#include "Ray.h"

class Disk
{
public:
	Disk(const Vector3f& center, const Vector3f& euler, float radius);
	bool Intersect(Ray& ray, Intersection& isect) const;

private:
	float mRadius;
	Matrix4x4	mObjectToWorld;
	Matrix4x4	mWorldToObject;
};