#include "Camera.h"

void Camera::Initialize(XMVECTOR start)
{
    float pitch = 0.f;
    float yaw = 0.f;
    float roll = 0.f;
    m_rotation = Vector3(pitch, yaw, roll);
    forward = Vector3(0.f, 0.f, 0.f);
    m_position = start;
}

void Camera::Update()
{
    float y = sinf(m_rotation.x * XM_PI / 180.0f);
    float r = cosf(m_rotation.x * XM_PI / 180.0f);
    float z = r * cosf(m_rotation.y * XM_PI / 180.0f);
    float x = r * sinf(m_rotation.y * XM_PI / 180.0f);

    forward = Vector3(x, y, z);
    forward.Normalize();

    forward.Cross(Vector3::UnitY, right);
    XMVECTOR lookAt = m_position + forward;
    view = (Matrix)DirectX::XMMatrixLookAtRH(m_position, lookAt, Vector3::UnitY);
}

void Camera::SetPosition(Vector3 newPos)
{
    SetPosition(newPos.x, newPos.y, newPos.z);
}

void Camera::SetPosition(float x, float y, float z)
{
    oldPosition = m_position;
    m_position = Vector3(x,y,z);
}

void Camera::SetPositionX(float x)
{
    oldPosition = m_position;
    m_position = Vector3(x, m_position.y, m_position.z);
}

void Camera::SetPositionY(float y)
{
    oldPosition = m_position;
    m_position = Vector3(m_position.x, y, m_position.z);
}

void Camera::SetPositionZ(float z)
{
    oldPosition = m_position;
    m_position = Vector3(m_position.x, m_position.y, z);
}

void Camera::SetRotation(Vector3 newRotation)
{
    m_rotation = newRotation;
}

void Camera::SetRotation(float x, float y, float z)
{
    m_rotation = Vector3(x, y, z);
}

void Camera::SetPitch(float x)
{
    m_rotation = Vector3(x, m_rotation.y, m_rotation.z);
}

void Camera::SetYaw(float y)
{
    m_rotation = Vector3(m_rotation.x, y, m_rotation.z);
}

void Camera::SetRoll(float z)
{
    m_rotation = Vector3(m_rotation.x, m_rotation.y, z);
}

void Camera::RenderReflection(float height)
{
    Vector3 newPosition = -m_position;
    newPosition.y = -newPosition.y + (height * 2.0f);
    float y = sinf(m_rotation.x * XM_PI / 180.0f);
    float r = cosf(m_rotation.x * XM_PI / 180.0f);
    float z = r * cosf(m_rotation.y * XM_PI / 180.0f);
    float x = r * sinf(m_rotation.y * XM_PI / 180.0f);

    forward = Vector3(x, y, z);
    forward.Normalize();

    forward.Cross(Vector3::UnitY, right);
    XMVECTOR lookAt = newPosition + forward;
    reflectionViewMatrix = (Matrix)DirectX::XMMatrixLookAtRH(newPosition, lookAt, Vector3::UnitY);
}


