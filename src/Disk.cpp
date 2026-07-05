#include "Disk.h"
#include "Primitive.h"
#include "SceneObject.h"

Disk::Disk(SceneObject* pSceneObject, float radius)
	:Primitive(pSceneObject), mRadius(radius)
{
	
}

bool Disk::Intersect(Ray& ray, Intersection& isect) const
{
	//ray in object space
	Ray r = m_pSceneObject->GetWorldToObject() * ray;

	if (fabs(r.d.z) < 1e-6f)
		return false;

	float t = -r.o.z / r.d.z;

	if (t < r.mint || t > r.maxt)
		return false;

	Vector3f p = r.o + t * r.d;

	if (glm::dot(p, p) > mRadius * mRadius)
		return false;

	isect.position = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p, 1.0f));
	isect.normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(0.0f, 0.0f, 1.0f, 0.0f)));
	isect.t = t;

	return true;
}