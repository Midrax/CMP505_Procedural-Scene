#include "Camera.h"

void Camera::Initialize(XMVECTOR start)
{
    float pitch = 0.f;
    float yaw = 0.f;
    float roll = 0.f;
    rotation = Vector3(pitch, yaw, roll);
    forward = Vector3(0.f, 0.f, 0.f);
    position = start;
}

void Camera::Update()
{
    float y = sinf(rotation.x * XM_PI / 180.0f);
    float r = cosf(rotation.x * XM_PI / 180.0f);
    float z = r * cosf(rotation.y * XM_PI / 180.0f);
    float x = r * sinf(rotation.y * XM_PI / 180.0f);

    forward = Vector3(x, y, z);
    forward.Normalize();

    forward.Cross(Vector3::UnitY, right);
    XMVECTOR lookAt = position + forward;

    view = Matrix::CreateLookAt(position, lookAt, Vector3::UnitY);
}
