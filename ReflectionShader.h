#include "pch.h"
#include "AlignedAllocationPolicy.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class ReflectionShader : public AlignedAllocationPolicy<16>
{
private:
	struct MatrixBufferType
	{
		Matrix world;
		Matrix view;
		Matrix projection;
	};

	struct ClipPlaneBufferType
	{
		Vector4 clipPlane;
	};

	struct LightBufferType
	{
		Vector4 lightDiffuseColor;
		Vector3 lightDirection;
		float colorTextureBrightness;
	};

public:
	ReflectionShader();
	ReflectionShader(const ReflectionShader&);
	~ReflectionShader();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, Matrix, Matrix, Matrix, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, Vector4,
		Vector3, float, Vector4);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*, const WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, Matrix, Matrix, Matrix, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, Vector4,
		Vector3, float, Vector4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11Buffer* m_clipPlaneBuffer = nullptr;
	ID3D11Buffer* m_lightBuffer = nullptr;
};