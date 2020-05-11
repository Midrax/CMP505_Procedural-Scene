#include "Water.h"

Water::Water()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}


Water::Water(const Water& other)
{
}


Water::~Water()
{
}


bool Water::Initialize(ID3D11Device* device, const WCHAR* textureFilename, float waterHeight, float waterRadius)
{
	
	m_waterHeight = waterHeight;

	
	if (!InitializeBuffers(device, waterRadius))
	{
		return false;
	}

	CreateDDSTextureFromFile(device, textureFilename, nullptr, m_Texture.ReleaseAndGetAddressOf());

	
	m_normalMapTiling.x = 0.01f;  
	m_normalMapTiling.y = 0.02f;  

	
	m_waterTranslation = 0.0f;

	
	m_reflectRefractScale = 0.03f;

	
	m_refractionTint = Vector4(0.0f, 0.8f, 1.0f, 1.0f);

	
	m_specularShininess = 200.0f;

	return true;
}


void Water::Shutdown()
{	
	ShutdownBuffers();
}


void Water::Update()
{
	
	m_waterTranslation += 0.001f;
	if (m_waterTranslation > 1.0f)
	{
		m_waterTranslation -= 1.0f;
	}
}


void Water::Render(ID3D11DeviceContext* deviceContext)
{
	
	RenderBuffers(deviceContext);
}


int Water::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* Water::GetTexture()
{
	return m_Texture.Get();
}


float Water::GetWaterHeight()
{
	return m_waterHeight;
}


Vector2 Water::GetNormalMapTiling()
{
	return m_normalMapTiling;
}


float Water::GetWaterTranslation()
{
	return m_waterTranslation;
}


float Water::GetReflectRefractScale()
{
	return m_reflectRefractScale;
}


Vector4 Water::GetRefractionTint()
{
	return m_refractionTint;
}


float Water::GetSpecularShininess()
{
	return m_specularShininess;
}


bool Water::InitializeBuffers(ID3D11Device* device, float waterRadius)
{
	
	m_vertexCount = 6;

	
	m_indexCount = 6;

	
	VertexType* vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	
	ULONG* indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	
	vertices[0].position = Vector3(-waterRadius, 0.0f, waterRadius);  
	vertices[0].texture = Vector2(0.0f, 0.0f);

	vertices[1].position = Vector3(waterRadius, 0.0f, waterRadius);  
	vertices[1].texture = Vector2(1.0f, 0.0f);

	vertices[2].position = Vector3(-waterRadius, 0.0f, -waterRadius);  
	vertices[2].texture = Vector2(0.0f, 1.0f);

	vertices[3].position = Vector3(-waterRadius, 0.0f, -waterRadius);  
	vertices[3].texture = Vector2(0.0f, 1.0f);

	vertices[4].position = Vector3(waterRadius, 0.0f, waterRadius);  
	vertices[4].texture = Vector2(1.0f, 0.0f);

	vertices[5].position = Vector3(waterRadius, 0.0f, -waterRadius);  
	vertices[5].texture = Vector2(1.0f, 1.0f);

	
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;
	indices[4] = 4;
	indices[5] = 5;

	
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer)))
	{
		return false;
	}

	
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	
	if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer)))
	{
		return false;
	}

	
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}


void Water::ShutdownBuffers()
{
	
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}


void Water::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}