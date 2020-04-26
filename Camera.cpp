#include "Camera.h"

void Camera::Initialize(XMVECTOR start)
{
    pitch = 0;
    yaw = 0;
    position = start;
}

void Camera::Render()
{
    float y = sinf(pitch);
    float r = cosf(pitch);
    float z = r * cosf(yaw);
    float x = r * sinf(yaw);

    XMVECTOR lookAt = position + Vector3(x, y, z);

    view = XMMatrixLookAtRH(position, lookAt, Vector3::Up);
}
