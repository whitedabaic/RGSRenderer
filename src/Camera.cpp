#include "Camera.h"

void Camera::Initialize(const Vector3f& p, const Vector3f& target, const Vector3f& up, float fov, float n, float f, float width, float height)
{
	mPosition = p;

	//Vector3f l = glm::normalize(target - p);
	//Vector3f r = glm::normalize(glm::cross(up, l));
	//Vector3f u = glm::cross(l, r);

	//Matrix4x4 viewMatrix = glm::transpose(Matrix4x4(
	//	r.x, r.y, r.z, 0.0f,
	//	u.x, u.y, u.z, 0.0f,
	//	l.x, l.y, l.z, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f)) * MakeTranslation(-p);

	Matrix4x4 viewMatrix = glm::lookAtLH(p, target, up);

	Matrix4x4 porjectionMatrix = glm::perspectiveFovLH_ZO(fov, float(width), float(height), n, f);

	Matrix4x4 viewportMatrix = Matrix4x4(
		width / 2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -height / 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		width / 2.0f, height / 2.0f, 0.0f, 1.0f);

	Matrix4x4 combinedMatrix = viewportMatrix * porjectionMatrix * viewMatrix;
	Matrix4x4 inverseCombinedMatrix = glm::inverse(combinedMatrix);

	mCombinedMatrix = combinedMatrix;
	mInverseCombinedMatrix = inverseCombinedMatrix;
}

Ray Camera::GetRay(int x, int y) const
{
	Ray ray;
	ray.o = mPosition;

	Vector4f p(x, y, 0.0f, 1.0f);
	Vector4f worldPos = mInverseCombinedMatrix * p;

	worldPos /= worldPos.w;

	ray.d = glm::normalize(Vector3f(worldPos) - mPosition);

	return ray;
}
