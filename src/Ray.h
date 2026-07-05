#pragma once
#include "Common.h"

struct Intersection
{
	Vector3f position; // Intersection point
	Vector3f normal; // Normal at intersection
	float t;    // Ray parameter at intersection
};

struct Ray
{
	Vector3f o;
	Vector3f d;

	float mint = 0.0f;
	float maxt = FLT_MAX;
};

inline Ray operator*(const Matrix4x4& m, const Ray& r)
{
	Ray result;
	result.o = Vector3f(m * Vector4f(r.o, 1.0f));
	result.d = Vector3f(m * Vector4f(r.d, 0.0f));
	result.mint = r.mint;
	result.maxt = r.maxt;
	return result;
}