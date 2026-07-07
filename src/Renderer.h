#pragma once
#include "Common.h"
#include "Camera.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include <vector>
#include "Scene.h"

class Renderer
{
public:
	Renderer(int w, int h, int samplePerPixel, const char* filepath);
	virtual ~Renderer();

	void Run();

private:
	Color RenderPixel(int x, int y);
	Color RenderSubPixel(float x, float y);
	Color GetIrradiance(const Ray& ray);
	Color GetRadiance(const Ray& ray);

	void RunRenderThread();

	int mViewportWidth = 800;
	int mViewportHeight = 600;
	int SamplePerPixel = 20;

	uint32_t* mBuffer = nullptr;

	static constexpr int TileSize = 16;

	std::atomic<int> mNextTileIndex = 0;
	std::atomic<bool> bCancelRender = false;

	Scene* mScene = nullptr;
};