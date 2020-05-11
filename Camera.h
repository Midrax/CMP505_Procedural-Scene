#pragma once
#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:

	Matrix		view;
	Vector3		forward;
	Vector3		right;
	Vector3		lookAt;
	Vector3		oldPosition;
	Matrix		reflectionViewMatrix;

	void Initialize(XMVECTOR start);
	void Update();

	Vector3 GetPosition() { return m_position; }
	void SetPosition(Vector3 newPos);
	void SetPosition(float x, float y, float z);
	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	Vector3 GetRotation() { return m_rotation; }
	void SetRotation(Vector3 newRotation);
	void SetRotation(float x, float y, float z);
	void SetPitch(float x);
	void SetYaw(float y);
	void SetRoll(float z);

	void RenderReflection(float height);

private:
	Vector3		m_position;
	Vector3		m_rotation;
};

