#include "pch.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Frustum
{
public:
	Frustum();
	Frustum(const Frustum&);
	~Frustum();

	void ConstructFrustum(float screenDepth, Matrix projectionMatrix, Matrix viewMatrix);

	bool CheckPoint(float, float, float);
	bool CheckCube(float, float, float, float);
	bool CheckSphere(float, float, float, float);
	bool CheckRectangle(float, float, float, float, float, float);

private:
	Plane m_planes[6];
};