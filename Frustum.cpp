#include "Frustum.h"

Frustum::Frustum()
{
}


Frustum::Frustum(const Frustum& other)
{
}


Frustum::~Frustum()
{
}

void Frustum::ConstructFrustum(float screenDepth, Matrix projectionMatrix, Matrix viewMatrix)
{
	float zMinimum, r;
	Matrix matrix;


	// Calculate the minimum Z distance in the frustum.
	zMinimum = -projectionMatrix._43 / projectionMatrix._33;
	r = screenDepth / (screenDepth - zMinimum);
	projectionMatrix._33 = r;
	projectionMatrix._43 = -r * zMinimum;

	// Create the frustum matrix from the view matrix and updated projection matrix.
	matrix = viewMatrix * projectionMatrix;

	// Calculate near plane of frustum.
	m_planes[0].x = matrix._14 + matrix._13;
	m_planes[0].y = matrix._24 + matrix._23;
	m_planes[0].z = matrix._34 + matrix._33;
	m_planes[0].w = matrix._44 + matrix._43;
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// Calculate far plane of frustum.
	m_planes[1].x = matrix._14 - matrix._13;
	m_planes[1].y = matrix._24 - matrix._23;
	m_planes[1].w = matrix._34 - matrix._33;
	m_planes[1].z = matrix._44 - matrix._43;
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// Calculate left plane of frustum.
	m_planes[2].x = matrix._14 + matrix._11;
	m_planes[2].y = matrix._24 + matrix._21;
	m_planes[2].z = matrix._34 + matrix._31;
	m_planes[2].w = matrix._44 + matrix._41;
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// Calculate right plane of frustum.
	m_planes[3].x = matrix._14 - matrix._11;
	m_planes[3].y = matrix._24 - matrix._21;
	m_planes[3].z = matrix._34 - matrix._31;
	m_planes[3].w = matrix._44 - matrix._41;
	m_planes[3] = XMPlaneNormalize(m_planes[3]);


	// Calculate top plane of frustum.
	m_planes[4].x = matrix._14 - matrix._12;
	m_planes[4].y = matrix._24 - matrix._22;
	m_planes[4].z = matrix._34 - matrix._32;
	m_planes[4].w = matrix._44 - matrix._42;
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// Calculate bottom plane of frustum.
	m_planes[5].x = matrix._14 + matrix._12;
	m_planes[5].y = matrix._24 + matrix._22;
	m_planes[5].z = matrix._34 + matrix._32;
	m_planes[5].w = matrix._44 + matrix._42;
	m_planes[5] = XMPlaneNormalize(m_planes[5]);
}

bool Frustum::CheckPoint(float x, float y, float z)
{
	int i;


	// Check if the point is inside all six planes of the view frustum.
	for (i = 0; i < 6; i++)
	{
		Vector4 result = XMPlaneDotCoord(m_planes[i], Vector3(x, y, z));
		if ( result.x < 0.0f)
		{
			return false;
		}
	}

	return true;
}


bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	int i;


	// Check if any one point of the cube is in the view frustum.
	for (i = 0; i < 6; i++)
	{
		Vector3 result;
		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - radius), (yCenter - radius), (zCenter - radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + radius), (yCenter - radius), (zCenter - radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - radius), (yCenter + radius), (zCenter - radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + radius), (yCenter + radius), (zCenter - radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - radius), (yCenter - radius), (zCenter + radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + radius), (yCenter - radius), (zCenter + radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - radius), (yCenter + radius), (zCenter + radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + radius), (yCenter + radius), (zCenter + radius)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}

bool Frustum::CheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
	int i;

	// Check if the radius of the sphere is inside the view frustum.
	for (i = 0; i < 6; i++)
	{
		Vector3 result;
		result = XMPlaneDotCoord(m_planes[i], Vector3(xCenter, yCenter, zCenter));
		if (result.x < -radius)
		{
			return false;
		}
	}

	return true;
}


bool Frustum::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
	int i;

	// Check if any of the 6 planes of the rectangle are inside the view frustum.
	for (i = 0; i < 6; i++)
	{
		Vector3 result;
		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		result = XMPlaneDotCoord(m_planes[i], Vector3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize)));
		if (result.x >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}
