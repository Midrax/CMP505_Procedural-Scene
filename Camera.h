#pragma once
#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:

	XMMATRIX	view;
	Vector3		position;
	Vector3		rotation;
	Vector3		forward;
	Vector3		right;
	Vector3		lookAt;

	void Initialize(XMVECTOR start);
	void Update();

};

