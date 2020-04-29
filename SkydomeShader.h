////////////////////////////////////////////////////////////////////////////////
// Filename: SkydomeShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SKYDOMESHADER_H_
#define _SKYDOMESHADER_H_


//////////////
// INCLUDES //
//////////////
#include "pch.h"
#include <fstream>
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;


////////////////////////////////////////////////////////////////////////////////
// Class name: SkydomeShader
////////////////////////////////////////////////////////////////////////////////
class SkydomeShader
{
private:
	struct MatrixBufferType
	{
		Matrix world;
		Matrix view;
		Matrix projection;
	};

	struct GradientBufferType
	{
		Vector4 apexColor;
		Vector4 centerColor;
	};

public:
	SkydomeShader();
	SkydomeShader(const SkydomeShader&);
	~SkydomeShader();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, Matrix, Matrix, Matrix, Vector4, Vector4);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, Matrix, Matrix, Matrix, Vector4, Vector4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_gradientBuffer;
};

#endif