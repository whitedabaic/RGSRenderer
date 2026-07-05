#pragma once
#include "Primitive.h"

class Triangle : public Primitive
{
public:
	Triangle(SceneObject* pSceneObject, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);
	virtual bool Intersect(Ray& ray, Intersection& isect) const override;

private:
	Vector3f mVertices[3];
	Vector3f mNormal;
};