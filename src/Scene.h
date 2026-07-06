#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"

class Scene
{
public:
	~Scene();
	static Scene* LoadSceneFromXML(const char* filepath, int W, int H);

	void SetCamera(const Camera& camera) { mCamera = camera; }
	const Camera& GetCamera() const { return mCamera; }
	Camera& GetCamera() { return mCamera; }

	SceneObject* Intersect(Ray ray, Intersection& isect) const;

	SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);

	template<typename T, typename... Args>
	T* CreateLight(Args&&...args)
	{
		Light* light = new T(std::forward<Args>(args)...);
		mLights.push_back(light);
		return (T*)light;
	}

	const std::vector<Light*>& GetLights() const { return mLights; }
private:
	Camera						mCamera;
	std::vector<SceneObject*>	mSceneObjects;
	std::vector<Light*>			mLights;
};