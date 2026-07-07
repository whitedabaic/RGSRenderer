#include "Renderer.h"
#include <MiniFB.h>
#include <thread>
#include <vector>
#include "Material.h"

Renderer::Renderer(int w, int h, int samplePerPixel, const char* filepath)
	:mViewportHeight(h), mViewportWidth(w), SamplePerPixel(samplePerPixel)
{
    mScene = Scene::LoadSceneFromXML(filepath, w, h);
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
    if (!mScene)
        return Color(0, 0, 0);

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
    Color color = GetRadiance(ray);
    return color;
}

Color Renderer::GetIrradiance(const Ray& ray)
{
    Intersection isect;
    if (!mScene->Intersect(ray, isect))
        return Color(0, 0, 0);

    Color E(0, 0, 0);
    for (Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color L = pLight->GetRadiance(isect.position, sourcePos);


        // ÇòShadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 1e-4f;
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadow_isect;
        if (mScene->Intersect(shadowRay, shadow_isect))
            continue;

        float cosTheta = glm::dot(isect.normal, shadowRay.d);

        E += L * glm::max(cosTheta, 0.0f);
    }
    return E;
}

Color Renderer::GetRadiance(const Ray& ray)
{
    Intersection isect;
    SceneObject* pSceneObject = mScene->Intersect(ray, isect);
    if (pSceneObject == nullptr)
        return Color(0, 0, 0);

    Material* pMaterial = pSceneObject->GetMaterial();
    Color Lo(0, 0, 0);

    Matrix3x3 localToWorld = MakeCoordinateSystem(isect.normal);
    Matrix3x3 worldToLocal = glm::inverse(localToWorld);

    Vector3f wo = worldToLocal * (-ray.d);

    for (Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color L = pLight->GetRadiance(isect.position, sourcePos);

        // ÇòShadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 1e-3f;
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadow_isect;
        if (mScene->Intersect(shadowRay, shadow_isect))
            continue;

        Vector3f wi = worldToLocal * shadowRay.d;
        float cosTheta = glm::dot(isect.normal, shadowRay.d);
        Color brdf = pMaterial->BRDF(wo, wi);
        Lo += brdf * L * glm::max(cosTheta, 0.0f);
    }
    return Lo;
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