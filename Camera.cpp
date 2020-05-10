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
    view = (Matrix)DirectX::XMMatrixLookAtRH(position, lookAt, Vector3::UnitY);
}

void Camera::SetPosition(Vector3 newPos)
{
    SetPosition(newPos.x, newPos.y, newPos.z);
}

void Camera::SetPosition(float x, float y, float z)
{
    oldPosition = position;
    position = Vector3(x,y,z);
}

void Camera::SetPositionX(float x)
{
    oldPosition = position;
    position = Vector3(x, position.y, position.z);
}

void Camera::SetPositionY(float y)
{
    oldPosition = position;
    position = Vector3(position.x, y, position.z);
}

void Camera::SetPositionZ(float z)
{
    oldPosition = position;
    position = Vector3(position.x, position.y, z);
}

void Camera::SetRotation(Vector3 newRotation)
{
    rotation = newRotation;
}

void Camera::SetRotation(float x, float y, float z)
{
    rotation = Vector3(x, y, z);
}

void Camera::SetPitch(float x)
{
    rotation = Vector3(x, rotation.y, rotation.z);
}

void Camera::SetYaw(float y)
{
    rotation = Vector3(rotation.x, y, rotation.z);
}

void Camera::SetRoll(float z)
{
    rotation = Vector3(rotation.x, rotation.y, z);
}


