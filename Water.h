#include "pch.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Water
{
private:
	struct VertexType
	{
		Vector3 position;
		Vector2 texture;
	};

public:
	Water();
	Water(const Water&);
	~Water();

	bool Initialize(ID3D11Device*, const WCHAR*, float, float);
	void Shutdown();
	void Update();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	float GetWaterHeight();
	Vector2 GetNormalMapTiling();
	float GetWaterTranslation();
	float GetReflectRefractScale();
	Vector4 GetRefractionTint();
	float GetSpecularShininess();

private:
	bool InitializeBuffers(ID3D11Device*, float);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	float m_waterHeight = 0.0f;
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Texture;
	Vector2 m_normalMapTiling;
	float m_waterTranslation = 0.0f;
	float m_reflectRefractScale = 0.0f;
	Vector4 m_refractionTint;
	float m_specularShininess = 0.0f;
};