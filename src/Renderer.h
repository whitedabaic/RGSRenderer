#pragma once
#include "Common.h"
#include "Camera.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"

class Renderer
{
public:
	Renderer(int w, int h);
	virtual ~Renderer() {};

	void Run();

private:
	Color RenderPixel(int x, int y);
	void RunRenderThread();

	int mViewportWidth = 800;
	int mViewportHeight = 600;
	uint32_t* mBuffer = nullptr;

	static constexpr int TileSize = 16;

	std::atomic<int> mNextTileIndex = 0;
	std::atomic<bool> bCancelRender = false;

	Camera mCamera;

	Sphere* mSphere = nullptr;
	Disk* mDisk = nullptr;
	Triangle* mTriangle = nullptr;
};