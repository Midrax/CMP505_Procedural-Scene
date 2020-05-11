#include "WaterShader.h"
#include <d3dcompiler.h>

WaterShader::WaterShader()
{
}


WaterShader::WaterShader(const WaterShader& other)
{
}


WaterShader::~WaterShader()
{
}


bool WaterShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	
	return InitializeShader(device, hwnd, L"water_vs.hlsl", L"water_ps.hlsl");
}


void WaterShader::Shutdown()
{
	
	ShutdownShader();
}


bool WaterShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, Matrix worldMatrix, Matrix viewMatrix,
	Matrix projectionMatrix, Matrix reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
	ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* normalTexture, Vector3 cameraPosition,
	Vector2 normalMapTiling, float waterTranslation, float reflectRefractScale, Vector4 refractionTint,
	Vector3 lightDirection, float specularShininess)
{
	
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix, refractionTexture, reflectionTexture,
		normalTexture, cameraPosition, normalMapTiling, waterTranslation, reflectRefractScale, refractionTint, lightDirection,
		specularShininess))
	{
		return false;
	}

	
	RenderShader(deviceContext, indexCount);

	return true;
}


bool WaterShader::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "WaterVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(psFilename, NULL, NULL, "WaterPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	
	
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	
	D3D11_BUFFER_DESC camNormBufferDesc;
	camNormBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	camNormBufferDesc.ByteWidth = sizeof(CamNormBufferType);
	camNormBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camNormBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camNormBufferDesc.MiscFlags = 0;
	camNormBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&camNormBufferDesc, NULL, &m_camNormBuffer);
	if (FAILED(result))
	{
		return false;
	}

	
	D3D11_BUFFER_DESC waterBufferDesc;
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;

	
	result = device->CreateBuffer(&waterBufferDesc, NULL, &m_waterBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void WaterShader::ShutdownShader()
{
	
	if (m_waterBuffer)
	{
		m_waterBuffer->Release();
		m_waterBuffer = 0;
	}

	
	if (m_camNormBuffer)
	{
		m_camNormBuffer->Release();
		m_camNormBuffer = 0;
	}

	
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}


void WaterShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	
	OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));

	
	errorMessage->Release();
	errorMessage = 0;

	
	MessageBox(hwnd, L"Error compiling shader.", shaderFilename, MB_OK);
}


bool WaterShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, Matrix worldMatrix, Matrix viewMatrix, Matrix projectionMatrix,
	Matrix reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
	ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* normalTexture,
	Vector3 cameraPosition, Vector2 normalMapTiling, float waterTranslation, float reflectRefractScale,
	Vector4 refractionTint, Vector3 lightDirection, float specularShininess)
{
	
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	reflectionMatrix = XMMatrixTranspose(reflectionMatrix);

	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	dataPtr->reflection = reflectionMatrix;

	
	deviceContext->Unmap(m_matrixBuffer, 0);

	
	unsigned int bufferNumber = 0;

	
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	
	if (FAILED(deviceContext->Map(m_camNormBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	
	CamNormBufferType* dataPtr2 = (CamNormBufferType*)mappedResource.pData;

	
	dataPtr2->cameraPosition = cameraPosition;
	dataPtr2->padding1 = 0.0f;
	dataPtr2->normalMapTiling = normalMapTiling;
	dataPtr2->padding2 = Vector2(0.0f, 0.0f);

	
	deviceContext->Unmap(m_camNormBuffer, 0);

	
	bufferNumber = 1;

	
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_camNormBuffer);

	
	deviceContext->PSSetShaderResources(0, 1, &refractionTexture);
	deviceContext->PSSetShaderResources(1, 1, &reflectionTexture);
	deviceContext->PSSetShaderResources(2, 1, &normalTexture);

	
	if (FAILED(deviceContext->Map(m_waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	
	WaterBufferType* dataPtr3 = (WaterBufferType*)mappedResource.pData;

	
	dataPtr3->waterTranslation = waterTranslation;
	dataPtr3->reflectRefractScale = reflectRefractScale;
	dataPtr3->refractionTint = refractionTint;
	dataPtr3->lightDirection = lightDirection;
	dataPtr3->specularShininess = specularShininess;
	dataPtr3->padding = Vector2(0.0f, 0.0f);

	
	deviceContext->Unmap(m_waterBuffer, 0);

	
	bufferNumber = 0;

	
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_waterBuffer);

	return true;
}


void WaterShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	
	deviceContext->IASetInputLayout(m_layout);

	
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
