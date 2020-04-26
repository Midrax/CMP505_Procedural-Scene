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
	Matrix		projection;
	float		pitch;
	float		yaw;

	void Initialize(XMVECTOR start);
	void Render();

};

