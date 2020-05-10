#pragma once
#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:

	XMMATRIX	view;
	Vector3		forward;
	Vector3		right;
	Vector3		lookAt;
	Vector3		oldPosition;

	void Initialize(XMVECTOR start);
	void Update();

	Vector3 GetPosition() { return position; }
	void SetPosition(Vector3 newPos);
	void SetPosition(float x, float y, float z);
	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	Vector3 GetRotation() { return rotation; }
	void SetRotation(Vector3 newRotation);
	void SetRotation(float x, float y, float z);
	void SetPitch(float x);
	void SetYaw(float y);
	void SetRoll(float z);

private:
	Vector3		position;
	Vector3		rotation;
};

