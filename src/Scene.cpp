#define _CRT_SECURE_NO_WARNINGS

#include "Scene.h"
#include "tinyxml2.h"
#include <sstream>
#include <string>
#include <algorithm>
#include <cstdio>
#include "Triangle.h"
#include "Sphere.h"
#include "Disk.h"
#include "Material.h"

using namespace tinyxml2;

static Vector3f ParseVector3f(const char* str)
{
    Vector3f v(0.0f);
    if (!str) return v;

    if (std::sscanf(str, " %f , %f , %f", &v.x, &v.y, &v.z) != 3)
    {
        v = Vector3f(0.0f);
    }
    return v;
}

static Vector2f ParseVector2f(const char* str)
{
    Vector2f v(0.0f);
    if (!str) return v;

    if (std::sscanf(str, " %f , %f", &v.x, &v.y) != 2)
    {
        v = Vector2f(0.0f);
    }
    return v;
}

static float GetChildFloat(XMLElement* parent, const char* childName, float defaultVal)
{
    XMLElement* child = parent->FirstChildElement(childName);
    if (child && child->GetText())
        return (float)atof(child->GetText());
    return defaultVal;
}

static const char* GetChildText(XMLElement* parent, const char* childName)
{
    XMLElement* child = parent->FirstChildElement(childName);

    if (child)
        return child->GetText();
    return nullptr;
}

Scene* Scene::LoadSceneFromXML(const char* filepath, int W, int H)
{
    XMLDocument doc;
    if (doc.LoadFile(filepath) != XML_SUCCESS)
        return nullptr;

    XMLElement* pRoot = doc.FirstChildElement("Scene");

    if (!pRoot)
        return nullptr;

    Scene* pScene = new Scene();

    // -------------------------
    // Camera
    // -------------------------
    XMLElement* pCameraElem = pRoot->FirstChildElement("Camera");

    if (pCameraElem)
    {
        Vector3f position = ParseVector3f(GetChildText(pCameraElem, "Position"));
        Vector3f target = ParseVector3f(GetChildText(pCameraElem, "Target"));
        Vector3f up = ParseVector3f(GetChildText(pCameraElem, "Up"));
        float nearZ = GetChildFloat(pCameraElem, "NearZ", 0.1f);
        float farZ = GetChildFloat(pCameraElem, "FarZ", 1000.0f);
        float fovDeg = GetChildFloat(pCameraElem, "Fov", 60.0f);
        float fovRad = glm::radians(fovDeg);

        Camera camera;
        camera.Initialize(position, target, up, fovRad, nearZ, farZ, W, H );
        pScene->SetCamera(camera);
    }

    // Materials
    XMLElement* pMaterialsElem = pRoot->FirstChildElement("Materials");
    if (pMaterialsElem)
    {
        for (XMLElement* pMatElem = pMaterialsElem->FirstChildElement("Material");
            pMatElem != nullptr;
            pMatElem = pMatElem->NextSiblingElement("Material"))
        {
            const char* nameText = GetChildText(pMatElem, "Name");
            const char* typeText = GetChildText(pMatElem, "Type");
            if (!nameText || !typeText)
                continue;

            std::string name = nameText;
            std::string type = typeText;

            if (type == "Lambert")
            {
                Color albedo = ParseVector3f(GetChildText(pMatElem, "Albedo"));
                pScene->CreateMaterial<LambertMaterial>(name, albedo);
            }
        }
    }
    // -------------------------
    // SceneObjects
    // -------------------------
    XMLElement* pSceneObjectsElem = pRoot->FirstChildElement("SceneObjects");

    if (pSceneObjectsElem)
    {
        for (XMLElement* pObjElem = pSceneObjectsElem->FirstChildElement("SceneObject");
            pObjElem != nullptr;
            pObjElem = pObjElem->NextSiblingElement("SceneObject"))
        {
            SceneObject* pSceneObject = nullptr;

            XMLElement* pTransformElem = pObjElem->FirstChildElement("Transform");

            if (pTransformElem)
            {
                Vector3f position = ParseVector3f(GetChildText(pTransformElem, "Position"));
                Vector3f rotation = ParseVector3f(GetChildText(pTransformElem, "Rotation"));
                float scale = GetChildFloat(pTransformElem, "Scale", 1.0f);

                rotation = glm::radians(rotation);
                pSceneObject = pScene->CreateSceneObject(position, rotation, scale);
            }

            if (!pSceneObject)
                continue;

            //Material
            const char* materialName = GetChildText(pObjElem, "Material");
            if (materialName)
            {
                Material* pMaterial = pScene->GetMaterial(materialName);
                if (pMaterial)
                    pSceneObject->SetMaterial(pMaterial);
            }

            // Primitives
            XMLElement* pPrimitivesElem = pObjElem->FirstChildElement("Primitives");
            if (pPrimitivesElem)
            {
                // Sphere
                for (XMLElement* pElem = pPrimitivesElem->FirstChildElement("Sphere");
                    pElem != nullptr;
                    pElem = pElem->NextSiblingElement("Sphere"))
                {
                    float radius = GetChildFloat(pElem, "Radius", 1.0f);
                    pSceneObject->CreatePrimitive<Sphere>(radius);
                }

                // Disk
                for (XMLElement* pElem = pPrimitivesElem->FirstChildElement("Disk");
                    pElem != nullptr;
                    pElem = pElem->NextSiblingElement("Disk"))
                {
                    float radius = GetChildFloat(pElem, "Radius", 1.0f);
                    pSceneObject->CreatePrimitive<Disk>(radius);
                }

                // Triangle
                for (XMLElement* pElem = pPrimitivesElem->FirstChildElement("Triangle");
                    pElem != nullptr;
                    pElem = pElem->NextSiblingElement("Triangle"))
                {
                    XMLElement* pV0 = pElem->FirstChildElement("Vertex");
                    XMLElement* pV1 = pV0 ? pV0->NextSiblingElement("Vertex") : nullptr;
                    XMLElement* pV2 = pV1 ? pV1->NextSiblingElement("Vertex") : nullptr;

                    if (pV0 && pV1 && pV2)
                    {
                        Vector3f v0 = ParseVector3f(pV0->GetText());
                        Vector3f v1 = ParseVector3f(pV1->GetText());
                        Vector3f v2 = ParseVector3f(pV2->GetText());
                        pSceneObject->CreatePrimitive<Triangle>(v0, v1, v2);
                    }
                }
            }
        }
    }

    // Lights
    XMLElement* pLightsElem = pRoot->FirstChildElement("Lights");
    if (pLightsElem)
    {
        for (XMLElement* pElem = pLightsElem->FirstChildElement("DirectionalLight");
            pElem != nullptr;
            pElem = pElem->NextSiblingElement("DirectionalLight"))
        {
            Vector3f direction = ParseVector3f(GetChildText(pElem, "Direction"));
            Color radiance = ParseVector3f(GetChildText(pElem, "Radiance"));
            pScene->CreateLight<DirectionalLight>(direction, radiance);
        }

        //PointLight
        for (XMLElement* pElem = pLightsElem->FirstChildElement("PointLight");
            pElem != nullptr;
            pElem = pElem->NextSiblingElement("PointLight"))
        {
            Vector3f position = ParseVector3f(GetChildText(pElem, "Position"));
            Color intensity = ParseVector3f(GetChildText(pElem, "Intensity"));
            Vector3f attenuations = ParseVector3f(GetChildText(pElem, "Attenuations"));
            pScene->CreateLight<PointLight>(position, intensity, attenuations);
        }

        //SpotLight
        for (XMLElement* pElem = pLightsElem->FirstChildElement("SpotLight");
            pElem != nullptr;
            pElem = pElem->NextSiblingElement("SpotLight"))
        {
            Vector3f position = ParseVector3f(GetChildText(pElem, "Position"));
            Vector3f direction = ParseVector3f(GetChildText(pElem, "Direction"));
            Color intensity = ParseVector3f(GetChildText(pElem, "Intensity"));
            float innerAngle = GetChildFloat(pElem, "InnerAngle", 0.0f);
            float outerAngle = GetChildFloat(pElem, "OuterAngle", 0.0f);
            Vector3f attenuations = ParseVector3f(GetChildText(pElem, "Attenuations"));

            innerAngle = glm::radians(innerAngle);
            outerAngle = glm::radians(outerAngle);

            pScene->CreateLight<SpotLight>(position, direction, intensity, innerAngle, outerAngle, attenuations);
        }
    }
    return pScene;
}

SceneObject* Scene::Intersect(Ray ray, Intersection& isect) const
{
	SceneObject* pHitObject = nullptr;
	for (const auto pSceneObject : mSceneObjects)
	{
		if (pSceneObject->Intersect(ray, isect))
		{
			ray.maxt = isect.t;
			pHitObject = pSceneObject;
		}
	}
	return pHitObject;
}

SceneObject* Scene::CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale)
{
	SceneObject* pSceneObject = new SceneObject(position, euler, scale);
	mSceneObjects.push_back(pSceneObject);
	return pSceneObject;
}

Scene::~Scene()
{
	for (SceneObject* pSceneObject : mSceneObjects)
		delete pSceneObject;

    for (Light* pLight : mLights)
        delete pLight;

    for (auto& pair : mMaterials)
        delete pair.second;
}
