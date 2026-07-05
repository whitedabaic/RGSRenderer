#include "Renderer.h"
#include <MiniFB.h>
#include <thread>
#include <vector>

Renderer::Renderer(int w, int h, int samplePerPixel, const char* filepath)
	:mViewportHeight(h), mViewportWidth(w), SamplePerPixel(samplePerPixel)
{
    mScene = Scene::LoadSceneFromXML(filepath, w, h);
    //mScene = new Scene();

    //Camera camera;
    //camera.Initialize(
    //    Vector3f(0.0, 0.0, 0.0),
    //    Vector3f(0.0, 0.0, 1.0),
    //    Vector3f(0.0f, 1.0f, 0.0f),
    //    glm::radians(60.0f),
    //    0.1f,
    //    1000.0f,
    //    w, h
    //);
    //mScene->SetCamera(camera);

    //SceneObject* pSceneObject = mScene->CreateSceneObject(Vector3f(0, 0, 5), Vector3f(0, 0, 0), 2.0f);
    //pSceneObject->CreatePrimitive<Triangle>(Vector3f(-1, -1, 0), Vector3f(1, -1, 0), Vector3f(1, 1, 0));
    //pSceneObject->CreatePrimitive<Triangle>(Vector3f(-1, -1, 0), Vector3f(1, 1, 0), Vector3f(-1, 1, 0));

    //SceneObject* pSceneObject2 = mScene->CreateSceneObject(Vector3f(0, 0, 2), Vector3f(0, 0, 0), 1.0f);
    //pSceneObject2->CreatePrimitive<Sphere>(0.5f);
}

Renderer::~Renderer()
{
    if (mScene)
		delete mScene;
}

void Renderer::Run()
{
	struct mfb_window* window = mfb_open_ex("Renderer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
	if (window == NULL)
		return;

	const size_t pixelCount = static_cast<size_t>(mViewportWidth) * static_cast<size_t>(mViewportHeight);

	mBuffer = static_cast<uint32_t*>(malloc(pixelCount * sizeof(uint32_t)));

    if (mBuffer == nullptr)
        return;

    mNextTileIndex.store(0, std::memory_order_relaxed);
    bCancelRender.store(false, std::memory_order_relaxed);

    unsigned int numThreads = std::thread::hardware_concurrency();

    if (numThreads == 0)
        numThreads = 1;

	std::vector<std::thread> renderThreads(numThreads);
    renderThreads.reserve(numThreads);

    for (unsigned int i = 0; i < numThreads; ++i)
    {
        renderThreads.emplace_back(&Renderer::RunRenderThread, this);
    }

	mfb_update_state state;
	do {
		// TODO: add some fancy rendering to the buffer of size 800 * 600

		state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);

		if (state != MFB_STATE_OK)
			break;

	} while (mfb_wait_sync(window));

    bCancelRender.store(true, std::memory_order_relaxed);

    for (std::thread& thread : renderThreads)
    {
        if (thread.joinable())
            thread.join();
    }

	free(mBuffer);
	mBuffer = NULL;
	window = NULL;

}

Color Renderer::RenderPixel(int x, int y)
{
    // SSAA
    const int N = SamplePerPixel;
    Color resultColor(0, 0, 0);

    for (int i = 0; i < N; i++)
    {
        float px = x + glm::linearRand(0.0f, 1.0f);
        float py = y + glm::linearRand(0.0f, 1.0f);
        
		Color color = RenderSubPixel(px, py);
        resultColor += (color / (float)N);
    }
    return resultColor;
}

Color Renderer::RenderSubPixel(float x, float y)
{
	Ray ray = mScene->GetCamera().GetRay(x, y);
	Intersection isect;
	Color color(0, 0, 0);

    if (mScene->Intersect(ray, isect))
    {
		color = isect.normal * 0.5f + 0.5f;
    }
    return color;
}

void Renderer::RunRenderThread()
{
    const int tilesX = (mViewportWidth + TileSize - 1) / TileSize;
    const int tilesY = (mViewportHeight + TileSize - 1) / TileSize;
    const uint32_t totalTiles = static_cast<uint32_t>(tilesX * tilesY);

    while (!bCancelRender.load(std::memory_order_relaxed))
    {
        const uint32_t tileIndex = mNextTileIndex.fetch_add(1, std::memory_order_relaxed);

        if (tileIndex >= totalTiles)
            break;

        const int tileX = tileIndex % tilesX;
        const int tileY = tileIndex / tilesX;

        const int startX = tileX * TileSize;
        const int startY = tileY * TileSize;

        const int endX = std::min(startX + TileSize, mViewportWidth);
        const int endY = std::min(startY + TileSize, mViewportHeight);

        for (int y = startY; y < endY; ++y)
        {
            if (bCancelRender.load(std::memory_order_relaxed))
                return;

            for (int x = startX; x < endX; ++x)
            {
                Color color = RenderPixel(x, y);

                const float rf = glm::clamp(color.r, 0.0f, 1.0f);
                const float gf = glm::clamp(color.g, 0.0f, 1.0f);
                const float bf = glm::clamp(color.b, 0.0f, 1.0f);

                const uint32_t r = static_cast<uint32_t>(std::round(rf * 255.0f));
                const uint32_t g = static_cast<uint32_t>(std::round(gf * 255.0f));
                const uint32_t b = static_cast<uint32_t>(std::round(bf * 255.0f));

                mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | b;
            }
        }
    }
}