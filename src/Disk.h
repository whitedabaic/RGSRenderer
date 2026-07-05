#pragma once
#include "Primitive.h"

class Disk : public Primitive
{
public:
	Disk(SceneObject* pSceneObject, float radius);
	virtual bool Intersect(Ray& ray, Intersection& isect) const override;

private:
	float mRadius;
};