#pragma once
#include "Ray.h"

class Light
{
public:
	// 对于一点p，求L（p）
	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const = 0;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight(const Vector3f& direction, const Vector3f& radiance)
		: mDirection(glm::normalize(direction)), mRadiance(radiance) {}

	Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override
	{
		sourcePos = p - mDirection * 100000.0f;
		return mRadiance;
	}
private:
	Vector3f mDirection;
	Vector3f mRadiance;
};

//Point Light
class PointLight : public Light
{
public:
	PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuations)
		: mPosition(position), mIntensity(intensity), mAttenuations(attenuations) {}

	Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override
	{
		sourcePos = mPosition;
		float R = glm::length(p - mPosition);
		float attenuation = 1.0f / (mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);
		return mIntensity * attenuation;
	}
private:
	Vector3f	mPosition;
	Color		mIntensity;
	Vector3f	mAttenuations;
};

// SpotLight
class SpotLight : public Light
{
public:
	SpotLight(const Vector3f& position, const Vector3f& direction, const Color& intensity,
		float innerAngle, float outerAngle, const Vector3f& attenuations)
		: mPosition(position), mDirection(direction), mIntensity(intensity), mCosInnerAngle(std::cos(innerAngle)),
		mCosOuterAngle(std::cos(outerAngle)), mAttenuations(attenuations) {
	}

	Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override
	{
		sourcePos = mPosition;
		float R = glm::length(p - mPosition);
		float k1 = 1.0f / (mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);

		Vector3f L = glm::normalize(p - mPosition);
		float cosTheta = glm::dot(L, mDirection);

		float k2 = (cosTheta - cos(mCosOuterAngle)) / (cos(mCosInnerAngle) - cos(mCosOuterAngle));

		return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
	}
private:
	Vector3f	mDirection;
	Vector3f	mPosition;
	Color		mIntensity;

	float		mCosInnerAngle;
	float		mCosOuterAngle;

	Vector3f	mAttenuations;
};