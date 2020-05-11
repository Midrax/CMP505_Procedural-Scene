
#include "pch.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class WaterShader
{
private:
	struct MatrixBufferType
	{
		Matrix world;
		Matrix view;
		Matrix projection;
		Matrix reflection;
	};

	struct CamNormBufferType
	{
		Vector3 cameraPosition;
		float padding1;
		Vector2 normalMapTiling;
		Vector2 padding2;
	};

	struct WaterBufferType
	{
		Vector4 refractionTint;
		Vector3 lightDirection;
		float waterTranslation;
		float reflectRefractScale;
		float specularShininess;
		Vector2 padding;
	};

public:
	WaterShader();
	WaterShader(const WaterShader&);
	~WaterShader();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, Matrix, Matrix, Matrix, Matrix, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*,
		ID3D11ShaderResourceView*, Vector3, Vector2, float, float, Vector4, Vector3, float);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, Matrix, Matrix, Matrix, Matrix, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*,
		ID3D11ShaderResourceView*, Vector3, Vector2, float, float, Vector4, Vector3, float);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11Buffer* m_camNormBuffer = nullptr;
	ID3D11Buffer* m_waterBuffer = nullptr;
};