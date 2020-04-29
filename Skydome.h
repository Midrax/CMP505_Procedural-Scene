#ifndef _SKYDOME_H_
#define _SKYDOME_H_
#include "pch.h"
#include <fstream>
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Skydome
{
private:
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VertexType
	{
		Vector3 position;
	};

public:
	Skydome();
	Skydome(const Skydome&);
	~Skydome();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	Vector4 GetApexColor();
	Vector4 GetCenterColor();

private:
	bool LoadSkyDomeModel(char*);
	void ReleaseSkyDomeModel();

	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ModelType* m_model;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	Vector4 m_apexColor, m_centerColor;
};

#endif