#include "pch.h"
#include "Terrain.h"
#include <time.h>

Terrain::Terrain()
{
	m_terrainGeneratedToggle = false;
	for (int i = 0; i < 512; i++) perm[i] = p[i & 255];

	m_VPoints = 0;
	m_VRegions = 0;
}


Terrain::~Terrain()
{
}

bool Terrain::Initialize(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	int index;
	float height = 0.0;
	bool result;

	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	m_frequency = m_terrainWidth / 20;
	m_amplitude = 10.0f;
	m_wavelength = 0.1f;
	m_dungeonDepth = 4;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	m_newHeightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	if (!m_newHeightMap)
	{
		return false;
	}

	// Initialise the data in the height map (flat).
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

		}
	}

	//even though we are generating a flat terrain, we still need to normalise it. 
	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	if (!result)
	{
		return false;
	}

	CalculateTextureCoordinates();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	return true;
}

void Terrain::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

bool Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	DirectX::SimpleMath::Vector3* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new DirectX::SimpleMath::Vector3[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
	if (!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i + 1);
			index3 = ((j + 1) * m_terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;

			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;

			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

void Terrain::Shutdown()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = 0;
	}

	if (m_parentNode)
	{
		ReleaseNode(m_parentNode);
		delete m_parentNode;
		m_parentNode = 0;
	}

	return;
}

bool Terrain::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index, i, j;
	int index1, index2, index3, index4; //geometric indices. 

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	m_vertices = new VertexType[m_vertexCount];
	if (!m_vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = (m_terrainHeight * j) + i;          // Bottom left.
			index2 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
			index3 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
			index4 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.

			float tilingValue = m_terrainWidth / 10;
			// if i is even
			if (j % 2 == 0) {
				if (i % 2 == 0)
				{
					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right.
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;
				}
				else
				{
					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;
				}
			}
			else
			{
				if (i % 2 == 0)
				{
					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;

				}
				else
				{


					// Upper left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].x / tilingValue, m_heightMap[index3].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom left. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].x / tilingValue, m_heightMap[index1].z / tilingValue);
					indices[index] = index;
					index++;

					// Upper right. 
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].x / tilingValue, m_heightMap[index4].z / tilingValue);
					indices[index] = index;
					index++;

					// Bottom right.
					m_vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
					m_vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
					m_vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].x / tilingValue, m_heightMap[index2].z / tilingValue);
					indices[index] = index;
					index++;
				}
			}

		}
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// QuadTree
	int vertexCount;
	float centerX, centerZ, width;
	// Get the number of vertices in the terrain vertex array.
	vertexCount = GetVertexCount();
	// Store the total triangle count for the vertex list.
	m_triangleCount = vertexCount / 3;
	// Create a vertex array to hold all of the terrain vertices.
	m_vertexList = m_vertices;
	// Calculate the center x,z and the width of the mesh.
	Vector3 meshDimension = CalculateMeshDimensions(vertexCount);
	centerX = meshDimension.x;
	centerZ = meshDimension.y;
	width = meshDimension.z;
	// Create the parent node for the quad tree.
	m_parentNode = new NodeType;
	if (!m_parentNode)
	{
		return false;
	}
	// Recursively build the quad tree based on the vertex list data and mesh dimensions.
	CreateTreeNode(m_parentNode, centerX, centerZ, width, device);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] m_vertices;
	m_vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void Terrain::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void Terrain::CalculateTextureCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;

	int TEXTURE_REPEAT = 32;
	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = m_terrainWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	int index;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			// Store the texture coordinate in the height map.
			m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
			m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

			tuCoordinate += incrementValue;
			tuCount++;
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}

		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	return;
}

bool Terrain::GenerateHeightMap(ID3D11Device* device)
{
	bool result;

	int index;
	float height = 0.0;

	m_frequency = (6.283 / m_terrainHeight) / m_wavelength; //we want a wavelength of 1 to be a single wave over the whole terrain.  A single wave is 2 pi which is about 6.283
	for (int i = 0; i < 30; i++) Faulting();
	RandomHeightMap();
	SmoothenHeightMap(device, 2);
	NoiseHeightMap();
	SmoothenHeightMap(device, 4);
	VoronoiDungeon();

	result = CalculateNormals();
	if (!result)
	{
		return false;
	}
	CalculateTextureCoordinates();

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}

void Terrain::RandomHeightMap()
{

	srand(time(NULL));
	int index;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].y += randomFloat(-0.4f, 0.4f);
		}
	}
}

void Terrain::RandomHeightMap(ID3D11Device* device)
{
	RandomHeightMap();
	CalculateNormals();
	CalculateTextureCoordinates();
	InitializeBuffers(device);
}

void Terrain::NoiseHeightMap()
{
	int index;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y += (float)simplexNoise((double)i * m_wavelength, (double)j * m_wavelength) * m_amplitude;
			m_heightMap[index].z = (float)j;
		}
	}
}

void Terrain::NoiseHeightMap(ID3D11Device* device)
{
	NoiseHeightMap();
	CalculateNormals();
	CalculateTextureCoordinates();
	InitializeBuffers(device);
}

void Terrain::Faulting()
{
	int x1, y1, x2, y2;
	float m, b;
	x1 = (int)m_terrainWidth * 0.1f + (int)(rand() % (int)(m_terrainWidth * 0.8f));
	y1 = (rand() % 2 == 0) ? m_terrainHeight - 1 : 0;
	int random1 = 0;
	while (random1 == 0 || x2 == x1) {

		random1 = randomFloat(-5.f,5.f);
		x2 = x1 + random1;		//random between -15 and 15
	}
	int random2 = 0;
	while (random2 == 0)
	{
		random2 = randomFloat(-5.f, 5.f);
	}
	y2 = y1 + random2;		//random between -15 and 15

	m = ((float)(y2 - y1)) / ((float)(x2 - x1));
	b = (float)y1 - (((float)x1) * m);

	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = 10.0f;
	float r = random * diff;
	random1 = (int)(-5.0f + r);
	float H1 = randomFloat(-5.f, 5.f);
	float H2 = H1 * 0.5f;
	int index;
	bool eq;
	int i, j;
	//Faulting
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainWidth * j) + i;

			eq = (float)j > ((float)i * m) + b;
			if (eq)
				m_heightMap[index].y += H1;

			m_heightMap[index].y -= H1;
		}
	}
}

void Terrain::Faulting(ID3D11Device* device)
{
	Faulting();
	CalculateNormals();
	CalculateTextureCoordinates();
	InitializeBuffers(device);
}

void Terrain::AddVoronoiPointAt(int IndexInArray, int RegionIndex) {

	// Creating a voronoi seed and the definition of a voronoi region for each voronoi seed
	VoronoiPoint* v = new VoronoiPoint;
	VoronoiRegion* r = new VoronoiRegion;
	v->x = m_heightMap[IndexInArray].x;
	v->y = m_heightMap[IndexInArray].y;
	v->z = m_heightMap[IndexInArray].z;
	v->index = IndexInArray;
	v->height = RegionIndex;
	if (RegionIndex % 2 == 0)
		v->height = -v->height;
	v->RegionIndex = RegionIndex;
	r->vPoint = v;
	m_VPoints->push_back(v);
	m_VRegions->push_back(r);
	m_VRegions->at(RegionIndex)->maxDist = 0.0f;
	v = 0;
	r = 0;
}
void Terrain::VoronoiDungeon()
{
	srand(time(NULL));
	int nOfRooms = randomFloat(140, 200);
	//creating voronoi Regions with parameters
	VoronoiRegions(nOfRooms, nOfRooms / 10);
	
	int index = m_rooms.at(0)->vPoint->index;
	float h = 0.50f;
	m_heightMap[index].y += h;
	m_heightMap[index + 1].y += h;
	m_heightMap[index - 1].y += h;
	m_heightMap[index + m_terrainWidth].y += h;
	m_heightMap[index - m_terrainWidth].y += h;
	m_heightMap[index + m_terrainWidth + 1].y += h;
	m_heightMap[index + m_terrainWidth - 1].y += h;
	m_heightMap[index - m_terrainWidth + 1].y += h;
	m_heightMap[index - m_terrainWidth - 1].y += h;
}
void Terrain::VoronoiDungeon(ID3D11Device* device)
{
	VoronoiDungeon();
	CalculateNormals();
	CalculateTextureCoordinates();
	InitializeBuffers(device);
}
void Terrain::VoronoiRegions(int numOfPoints = 200, int numOfRooms = 20) 
{
	ReleaseVoronoi();

	m_VPoints = new vector<VoronoiPoint*>;
	m_VRegions = new vector<VoronoiRegion*>;

	bool resetMesh = false;
	bool randomizePoints = false;
	bool showFullDiagram = false;

	// Create a grid of x*x points on the height map to be the seeds of the voronoi region. 
	if (!randomizePoints) {
		int k = 0;
		int pointsInCols = (int)sqrt((float)numOfPoints);
		int inBetweens = m_terrainWidth / pointsInCols;

		for (int j = 0; j < pointsInCols; j++) {
			for (int i = 0; i < pointsInCols; i++) {
				int nj = (inBetweens / 2);
				int index = ((j * inBetweens * m_terrainWidth)) + ((i * inBetweens)) + nj + (nj * m_terrainWidth);	// offset the center of the grid so that nothing is on the edges;
				index += (int)randomFloat(-nj, nj);						//randomizing the locations
				index += (int)randomFloat(-nj, nj) * m_terrainWidth;	//randomizing the locations

				AddVoronoiPointAt(index, k);
				k++;
			}
		}

		numOfPoints = k;
	}
	else {					// randomizing the points instead of creating a grid. (Only for testing)
		for (int k = 0; k < numOfPoints; k++) {
			int i = 0, j = 0, index = 0;

			j = (int)randomFloat(0, m_terrainHeight - 1);
			i = (int)randomFloat(0, m_terrainWidth - 1);
			index = (j * m_terrainWidth) + i;

			AddVoronoiPointAt(index, k);
		}
	}

	//get Voronoi data for each index of the height map
	for (int j = 0; j < m_terrainHeight; j++) {
		for (int i = 0; i < m_terrainWidth; i++)
		{
			int minIndex;
			int index = (j * m_terrainWidth) + i;
			float minDist = 100000.0f;
			for (int k = 0; k < numOfPoints; k++) {
				float dist = sqrt(pow(m_VPoints->at(k)->x - m_heightMap[index].x, 2) + pow(m_VPoints->at(k)->z - m_heightMap[index].z, 2));
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = k;
				}
			}
			VoronoiData* vd = new VoronoiData;
			vd->VorPoint = m_VPoints->at(minIndex);
			vd->dist = minDist;
			m_heightMap[index].VorData = vd;
			vd = 0;

			//adding point to the region
			int RegionIndex = m_VPoints->at(minIndex)->RegionIndex;
			/*if (!m_VRegions->at(RegionIndex)->VRegionIndices) {
			m_VRegions->at(RegionIndex)->VRegionIndices = new vector<int>;
			}*/
			m_VRegions->at(RegionIndex)->VRegionIndices.push_back(index);
			//checking if the current distance is greater than the max dist (minDist is the current distance from the point)
			if (minDist > m_VRegions->at(RegionIndex)->maxDist) {
				m_VRegions->at(RegionIndex)->maxDist = minDist;
			}

		}
	}

	//if the mesh needs to be reset (Clearing all voronoi regions already present on the map)
	if (resetMesh)
	{
		for (int j = 0; j < m_terrainHeight; j++) {
			for (int i = 0; i < m_terrainWidth; i++) {
				int index = (j * m_terrainWidth) + i;
				m_heightMap[index].y = 0.0f;
				m_heightMap[index].walkable = 0.0f;
			}
		}
	}


	// Get n unique rooms from the generated voronoi regions
	int* n = new int[numOfRooms];
	int numOfRows = sqrt(numOfPoints);
	for (int i = 0; i < numOfRooms; i++) {
		n[i] = (int)randomFloat(numOfRows + 1, (float)numOfPoints - numOfRows - 1);

		if ((n[i] % numOfRows == 0) || (n[i] % numOfRows == numOfRows - 1)) //Left Right Border
		{
			i--;
			continue;
		}
		for (int j = i - 1; j >= 0; j--)
		{
			//check for adjacent room
			if (n[i] == n[j] || n[i] + 1 == n[j] || n[i] - 1 == n[j] || n[i] + numOfRows == n[j] || n[i] - numOfRows == n[j] || n[i] - numOfRows + 1 == n[j] || n[i] - numOfRows - 1 == n[j] || n[i] + 1 + numOfRows == n[j] || n[i] - 1 + numOfRows == n[j])
			{
				i--;
				break;
			}
		}
	}

	//pushing all the unique rooms into a new vector.
	for (int planes = 0; planes < numOfRooms; planes++) {
		m_rooms.push_back(m_VRegions->at(n[planes]));
	}

	//settingHeight so that the dungeon is indented / show all the voronoi regions if needed
	if (showFullDiagram) {
		for (int j = 0; j < m_terrainHeight; j++) {
			for (int i = 0; i < m_terrainWidth; i++) {

				int index = (j * m_terrainWidth) + i;
				int regionIndex = m_heightMap[index].VorData->VorPoint->RegionIndex;

				m_heightMap[index].y = 0;
				m_heightMap[index].y = m_heightMap[index].VorData->VorPoint->RegionIndex;
			}
		}
	}
	else {
		for (int planes = 0; planes < numOfRooms; planes++) {
			int s = m_VRegions->at(n[planes])->VRegionIndices.size();
			for (int i = 0; i < s; i++) {
				int index = m_VRegions->at(n[planes])->VRegionIndices.at(i);
				if (m_heightMap[index].walkable == 0.0f) {
					m_heightMap[index].y += m_dungeonDepth;
					m_heightMap[index].walkable = GetWalkableValue(m_heightMap[index].x, m_heightMap[index].z);
				}
			}
		}
	}
	//ReleaseVornoi();
	DelaunayTriangles();
	return;
}

bool Terrain::SmoothenHeightMap(ID3D11Device* device, float smoothenFactor)
{
	if (smoothenFactor == 0) smoothenFactor = 1.f; // Avoid dividing by zero;

	bool result;

	int index, i, j, count = 0;

	//loop through the terrain and set the heights how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.

	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{

			float sum_y = 0;
			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);
				sum_y += m_heightMap[index].y;
				count++;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + i;

				sum_y += m_heightMap[index].y;
				count++;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 2)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + (i + 1);

				sum_y += m_heightMap[index].y;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (m_terrainHeight - 2)))
			{
				index = ((j + 1) * (m_terrainHeight - 1)) + (i - 1);

				sum_y += m_heightMap[index].y;
				count++;
			}

			if ((i < (m_terrainWidth - 2)) && (j < (m_terrainHeight - 2)))
			{
				index = ((j + 1) * (m_terrainHeight - 1)) + (i + 1);

				sum_y += m_heightMap[index].y;
				count++;
			}

			// Upper right face.
			if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 2)))
			{
				index = ((j + 1) * (m_terrainHeight - 1)) + i;

				sum_y += m_heightMap[index].y;
				count++;
			}

			if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + (i - 1);

				sum_y += m_heightMap[index].y;
				count++;
			}

			if ((i < (m_terrainWidth - 2)) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + (i + 1);

				sum_y += m_heightMap[index].y;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum_y = (sum_y / (float)count);

			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)(m_heightMap[index].y + sum_y)/ smoothenFactor;
			m_heightMap[index].z = (float)j;
		}
	}

	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}

float Terrain::randomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
int Terrain::fastfloor(double x)
{
	return x > 0 ? (int)x : (int)x - 1;
}
double Terrain::dot(int g[], double x, double y) {
	return g[0] * x + g[1] * y;
}
double Terrain::mix(double a, double b, double t) {
	return (1 - t) * a + t * b;
}
double Terrain::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}
double Terrain::simplexNoise(double xin, double yin) {
	double n0, n1, n2; // Noise contributions from the three corners
	// Skew the input space to determine which simplex cell we're in
	double F2 = 0.5 * (sqrt(3.0) - 1.0);
	double s = (xin + yin) * F2; // Hairy factor for 2D
	int i = fastfloor(xin + s);
	int j = fastfloor(yin + s);
	double G2 = (3.0 - sqrt(3.0)) / 6.0;
	double t = (i + j) * G2;
	double X0 = i - t; // Unskew the cell origin back to (x,y) space
	double Y0 = j - t;
	double x0 = xin - X0; // The x,y distances from the cell origin
	double y0 = yin - Y0;
	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if (x0 > y0) { i1 = 1; j1 = 0; } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else { i1 = 0; j1 = 1; } // upper triangle, YX order: (0,0)->(0,1)->(1,1)
	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6
	double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	double y1 = y0 - j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	double y2 = y0 - 1.0 + 2.0 * G2;
	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = perm[ii + perm[jj]] % 12;
	int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
	int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;
	// Calculate the contribution from the three corners
	double t0 = 0.5 - x0 * x0 - y0 * y0;
	if (t0 < 0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}
	double t1 = 0.5 - x1 * x1 - y1 * y1;
	if (t1 < 0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	double t2 = 0.5 - x2 * x2 - y2 * y2;
	if (t2 < 0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 70.0 * (n0 + n1 + n2);
}

float* Terrain::GetWavelength()
{
	return &m_wavelength;
}
float* Terrain::GetAmplitude()
{
	return &m_amplitude;
}

void Terrain::ReleaseVoronoi()
{
	if (m_VPoints) {
		for (std::vector< VoronoiPoint* >::iterator it = m_VPoints->begin(); it != m_VPoints->end(); ++it)
		{
			delete (*it);
		}
		m_VPoints->clear();
		delete m_VPoints;
		m_VPoints = 0;
	}

	if (m_VRegions) {

		for (std::vector< VoronoiRegion* >::iterator it = m_VRegions->begin(); it != m_VRegions->end(); ++it)
		{
			delete (*it);
		}
		m_VRegions->clear();
		delete m_VRegions;
		m_VRegions = 0;
	}

	if (!m_rooms.empty())		m_rooms.clear();

	if (!m_corridors.empty()) {
		for (vector<vector<HeightMapType*>>::iterator i = m_corridors.begin(); i != m_corridors.end(); ++i) {
			(*i).clear();
		}
		m_corridors.clear();
	}

	return;
}
void Terrain::DelaunayTriangles() {
	//If VoronoiRegions exists
	if (m_VRegions) {

		// Adding points to algorithm
		vector<Vec2f> points;
		int nPoints = m_rooms.size();
		for (int i = 0; i < nPoints; i++) {
			points.push_back(Vec2f(m_rooms.at(i)->vPoint->x, m_rooms.at(i)->vPoint->z, i));
		}
		// Using algorithm to get delaunay triangulation
		Delaunay triangulation;
		vector<Triangle> triangles = triangulation.triangulate(points);
		vector<Edge> edges = triangulation.getEdges();

		//obtaining weights of edges as distances between points
		int nEdges = edges.size();
		for (std::vector< Edge >::iterator e = edges.begin(); e != edges.end(); ++e) {
			e->weight = sqrt(pow(e->p2.x - e->p1.x, 2) + pow(e->p2.y - e->p1.y, 2));
		}

		// Sorting the edges array according to weights
		std::sort(edges.begin(), edges.end());

		// Finding minimum spanning tree from graph obtaned in delaunay using Kruskal's Algorithm
		vector<Edge*> minSpanTree;

		// finding Extra corridors to increse circularity of the dungeon
		vector<Edge*> extraCorridors;
		for (std::vector< Edge >::iterator e = edges.begin(); e != edges.end(); ++e)
		{
			bool duplicate = false;
			//check for duplicates
			for (std::vector< Edge* >::iterator m = minSpanTree.begin(); m != minSpanTree.end(); ++m)
			{
				if ((*m)->p1 == (e->p1) && (*m)->p2 == (e->p2) || (*m)->p1 == (e->p2) && (*m)->p2 == (e->p1))
				{
					duplicate = true;
					break;
				}
			}

			//if its not a duplicate
			if (!duplicate) {
				minSpanTree.push_back(e._Ptr);

				if (isCircular(minSpanTree)) {
					minSpanTree.pop_back();
					if (randomFloat(0.0f, 1.0f) < 0.2f)		//20% chance to add a new corridor to increase circularity
						extraCorridors.push_back(e._Ptr);
				}

				if (minSpanTree.size() == nPoints - 1)
					break;		//enough points obtained
			}

		}

		//append the two corridors so that the minimum spanning tree stays in tact along with adding extra corridors
		minSpanTree.insert(minSpanTree.end(), extraCorridors.begin(), extraCorridors.end());

		makeCorridors(minSpanTree);

		//release Data
		points.clear();

		extraCorridors.clear();

		minSpanTree.clear();

		triangles.clear();

		edges.clear();
	}
	return;

}
bool Terrain::isCircular(vector<Edge*>& edges) {
	int nPoints = m_rooms.size();

	// Create adjacenct list
	vector<int>** adj = new vector<int> * [nPoints];
	for (int i = 0; i < nPoints; i++) {
		adj[i] = new vector<int>;
	}

	for (std::vector< Edge* >::iterator e = edges.begin(); e != edges.end(); ++e) {
		for (int i = 0; i < nPoints; i++) {
			if ((*e)->p1.index == i) {
				if (std::find(adj[i]->begin(), adj[i]->end(), (*e)->p2.index) == adj[i]->end())	//element p2 not found in adj list
					adj[i]->push_back((*e)->p2.index);
			}
			else if ((*e)->p2.index == i) {
				if (std::find(adj[i]->begin(), adj[i]->end(), (*e)->p1.index) == adj[i]->end())	//element p1 not found in adj list
					adj[i]->push_back((*e)->p1.index);
			}
		}
	}

	bool* visited = new bool[nPoints];

	for (int i = 0; i < nPoints; i++)
		visited[i] = false;

	//Here the back edge is used to check if the currently added edge creates a circularity in the graph.
	if (isCircular((edges.back())->p1.index, visited, adj, -1)) {
		return true;
	}

	delete[] visited;
	visited = 0;


	//releasing memory before leaving
	for (int i = 0; i < nPoints; i++) {
		adj[i]->clear();

	}
	delete[] adj;
	adj = 0;

	//returning false if no circularity
	return false;
}
bool Terrain::isCircular(int v, bool visited[], vector<int, allocator<int>>** adj, int parent) {

	visited[v] = true;

	for (vector<int>::iterator i = adj[v]->begin(); i != adj[v]->end(); ++i) {
		int ind = (*i);
		if (!visited[ind])
		{
			if (isCircular(ind, visited, adj, v))
				return true;
		}
		else	if (ind != parent)
			return true;
	}



	return false;

}
void Terrain::makeCorridors(const vector<Edge*>& tree) {

	for (int i = 0; i < tree.size(); i++) {
		//getting index of the point and accessing the index in terms for the heightmap
		int p1Index = m_rooms.at(tree.at(i)->p1.index)->vPoint->index;
		int p2Index = m_rooms.at(tree.at(i)->p2.index)->vPoint->index;

		//obtaining x1 nad y1 from the given index
		int x1 = p1Index % (m_terrainHeight), x2 = p2Index % (m_terrainHeight);
		int y1 = p1Index / m_terrainHeight, y2 = p2Index / m_terrainHeight;
		int ycol1 = y1, ycol2 = y2, xcol1 = x1, xcol2 = x2;
		bool yswap = false, xswap = false;
		//checking which one is smaller
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
			/*xcol1 = x2;
			xcol2 = x1;
			*/
			xswap = true;
		}

		if (y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
			/*ycol1 = y2;
			ycol2 = y1;
			*/
			yswap = true;
		}

		vector<HeightMapType*> c1, c2;
		//creating columns
		for (int j = x1; j <= x2; j++)
		{
			//giving it a height using the original position of y2
			for (int k = ycol2 - 2; k < ycol2 + 2; k++) {
				if (m_heightMap[(k * m_terrainHeight) + j].walkable == 0.0f) {
					int index = (k * m_terrainHeight) + j;
					m_heightMap[index].walkable = GetWalkableValue(m_heightMap[index].x, m_heightMap[index].z);
					m_heightMap[index].y += m_dungeonDepth;
				}
			}
			//adding the required points to a vector to access later
			c1.push_back(&m_heightMap[(ycol2 * m_terrainHeight) + j]);
		}
		for (int j = y1; j <= y2; j++)
		{
			//giving it a width using the original position of x1
			for (int k = xcol1 - 2; k < xcol1 + 2; k++) {
				if (m_heightMap[(j * m_terrainHeight) + k].walkable == 0.0f) {
					int index = (j * m_terrainHeight) + k;
					m_heightMap[index].walkable = GetWalkableValue(m_heightMap[index].x, m_heightMap[index].z);
					m_heightMap[index].y += m_dungeonDepth;
				}
			}
			//adding required points to vector to access later
			c2.push_back(&m_heightMap[(j * m_terrainHeight) + xcol1]);
		}

		//collecting both the corridors into a main vector
		m_corridors.push_back(c1);
		m_corridors.push_back(c2);

	}

	return;
}
float Terrain::GetWalkableValue(float i, float j)
{
	float f = (float)simplexNoise((double)i * m_wavelength, (double)j * m_wavelength) * m_amplitude;
	f = max(f * 1.25f, 0.0f); //clamping values with 0
	return f + 1.0f;		//adding 1 so that player can always walk
}

void Terrain::ReinitializeBuffers(ID3D11Device* device)
{
	m_vertexList = m_vertices;
	if (!m_vertexList)
	{
		return;
	}

	ResetNodeBuffers(m_parentNode, device);
	// Release the vertex list since the quad tree now has the vertices in each node.


	delete[] m_vertexList;
	m_vertexList = 0;
	return;
}
void Terrain::ResetNodeBuffers(NodeType* node, ID3D11Device* device)
{
	int numTriangles, i, count, vertexCount, index;
	VertexType* vertices;
	unsigned long* indices;
	bool result;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	count = 0;
	for (i = 0; i < 4; i++) {
		if (node->nodes[i] != 0)
		{
			count++;
			ResetNodeBuffers(node->nodes[i], device);
		}
	}
	if (count > 0)
		return;


	// Calculate the number of vertices.
	vertexCount = node->triangleCount * 3;

	// Create the vertex array.
	vertices = new VertexType[vertexCount];

	// Create the index array.
	indices = new unsigned long[vertexCount];

	// Initialize the index for this new vertex and index array.
	index = 0;

	// Go through all the triangles in the vertex list.
	for (i = 0; i < vertexCount; i++)
	{
		// If the triangle is inside this node then add it to the vertex array.

			// Get the three vertices of this triangle from the vertex list.
		vertices[i].position = m_vertexList[node->mainTextureIndex[i]].position;
		vertices[i].texture = m_vertexList[node->mainTextureIndex[i]].texture;
		vertices[i].normal = m_vertexList[node->mainTextureIndex[i]].normal;
		vertices[i].walkable = m_vertexList[node->mainTextureIndex[i]].walkable;
		indices[i] = i;
		node->vertexArray[i].x = m_vertexList[node->mainTextureIndex[i]].position.x;
		node->vertexArray[i].y = m_vertexList[node->mainTextureIndex[i]].position.y;
		node->vertexArray[i].z = m_vertexList[node->mainTextureIndex[i]].position.z;
		node->vertexArray[i].walkable = m_vertexList[node->mainTextureIndex[i]].walkable;

	}

	//releasing index and vertex buffer before creating new one
	node->vertexBuffer->Release();
	node->vertexBuffer = 0;
	node->indexBuffer->Release();
	node->indexBuffer = 0;



	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->vertexBuffer);

	// Set up the description of the index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &node->indexBuffer);

	// Release the vertex and index arrays now that the data is stored in the buffers in the node.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return;
}
int Terrain::GetDrawCount()
{
	return m_drawCount;
}
void Terrain::ReleaseNode(NodeType* node)
{
	int i;
	// Recursively go down the tree and release the bottom nodes first.
	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i] != 0)
		{
			ReleaseNode(node->nodes[i]);
		}
	}

	// Release the vertex buffer for this node.
	if (node->vertexBuffer)
	{
		node->vertexBuffer->Release();
		node->vertexBuffer = 0;
	}

	// Release the index buffer for this node.
	if (node->indexBuffer)
	{
		node->indexBuffer->Release();
		node->indexBuffer = 0;
	}

	// Release the vertex array for this node.
	if (node->vertexArray)
	{
		delete[] node->vertexArray;
		node->vertexArray = 0;
	}

	if (node->mainTextureIndex)
	{
		delete[] node->mainTextureIndex;
		node->mainTextureIndex = 0;
	}

	// Release the four child nodes.
	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i])
		{
			delete node->nodes[i];
			node->nodes[i] = 0;
		}
	}

	return;
}
Vector3 Terrain::CalculateMeshDimensions(int vertexCount)
{
	int i;
	float maxWidth, maxDepth, minWidth, minDepth, width, depth, maxX, maxZ;


	// Initialize the center position of the mesh to zero.
	float centerX = 0.0f;
	float centerZ = 0.0f;

	// Sum all the vertices in the mesh.
	for (i = 0; i < vertexCount; i++)
	{
		centerX += m_vertexList[i].position.x;
		centerZ += m_vertexList[i].position.z;
	}

	// And then divide it by the number of vertices to find the mid-point of the mesh.
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	// Initialize the maximum and minimum size of the mesh.
	maxWidth = 0.0f;
	maxDepth = 0.0f;

	minWidth = fabsf(m_vertexList[0].position.x - centerX);
	minDepth = fabsf(m_vertexList[0].position.z - centerZ);

	// Go through all the vertices and find the maximum and minimum width and depth of the mesh.
	for (i = 0; i < vertexCount; i++)
	{
		width = fabsf(m_vertexList[i].position.x - centerX);
		depth = fabsf(m_vertexList[i].position.z - centerZ);

		if (width > maxWidth) { maxWidth = width; }
		if (depth > maxDepth) { maxDepth = depth; }
		if (width < minWidth) { minWidth = width; }
		if (depth < minDepth) { minDepth = depth; }
	}

	// Find the absolute maximum value between the min and max depth and width.
	maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	// Calculate the maximum diameter of the mesh.
	float meshWidth = max(maxX, maxZ) * 2.0f;

	return Vector3(centerX, centerZ, meshWidth);
}
void Terrain::CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device)
{
	int numTriangles, i, count, vertexCount, index, vertexIndex;
	float offsetX, offsetZ;
	VertexType* vertices;
	unsigned long* indices;
	bool result;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Store the node position and size.
	node->positionX = positionX;
	node->positionZ = positionZ;
	node->width = width;

	// Initialize the triangle count to zero for the node.
	node->triangleCount = 0;

	// Initialize the vertex and index buffer to null.
	node->vertexBuffer = 0;
	node->indexBuffer = 0;

	// Initialize the vertex array to null.
	node->vertexArray = 0;

	//initialize textureindex
	node->mainTextureIndex = 0;

	// Initialize the children nodes of this node to null.
	node->nodes[0] = 0;
	node->nodes[1] = 0;
	node->nodes[2] = 0;
	node->nodes[3] = 0;

	// Count the number of triangles that are inside this node.
	numTriangles = CountTriangles(positionX, positionZ, width);

	// Case 1: If there are no triangles in this node then return as it is empty and requires no processing.
	if (numTriangles == 0)
	{
		return;
	}

	// Case 2: If there are too many triangles in this node then split it into four equal sized smaller tree nodes.
	if (numTriangles > MAX_TRIANGLES)
	{
		for (i = 0; i < 4; i++)
		{
			// Calculate the position offsets for the new child node.
			offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
			offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

			// See if there are any triangles in the new node.
			count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
			if (count > 0)
			{
				// If there are triangles inside where this new node would be then create the child node.
				node->nodes[i] = new NodeType;

				// Extend the tree starting from this new child node now.
				CreateTreeNode(node->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
			}
		}

		return;
	}

	// Case 3: If this node is not empty and the triangle count for it is less than the max then 
	// this node is at the bottom of the tree so create the list of triangles to store in it.
	node->triangleCount = numTriangles;
	// Calculate the number of vertices.
	vertexCount = numTriangles * 3;

	// Create the vertex array.
	node->vertexArray = new VectorType[vertexCount];

	//create array to store terrain locations index
	node->mainTextureIndex = new int[vertexCount];

	// Create the vertex array.
	vertices = new VertexType[vertexCount];

	// Create the index array.
	indices = new unsigned long[vertexCount];

	// Initialize the index for this new vertex and index array.
	index = 0;

	// Go through all the triangles in the vertex list.
	for (i = 0; i < m_triangleCount; i++)
	{
		// If the triangle is inside this node then add it to the vertex array.
		result = IsTriangleContained(i, positionX, positionZ, width);
		if (result == true)
		{
			// Calculate the index into the terrain vertex list.
			vertexIndex = i * 3;
			// Get the three vertices of this triangle from the vertex list.
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			vertices[index].walkable = m_vertexList[vertexIndex].walkable;
			node->mainTextureIndex[index] = vertexIndex;
			// Also store the vertex position information in the node vertex array.
			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;
			node->vertexArray[index].walkable = m_vertexList[vertexIndex].walkable;

			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			vertices[index].walkable = m_vertexList[vertexIndex].walkable;
			node->mainTextureIndex[index] = vertexIndex;
			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;
			node->vertexArray[index].walkable = m_vertexList[vertexIndex].walkable;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			vertices[index].walkable = m_vertexList[vertexIndex].walkable;
			node->mainTextureIndex[index] = vertexIndex;
			node->vertexArray[index].x = m_vertexList[vertexIndex].position.x;
			node->vertexArray[index].y = m_vertexList[vertexIndex].position.y;
			node->vertexArray[index].z = m_vertexList[vertexIndex].position.z;
			node->vertexArray[index].walkable = m_vertexList[vertexIndex].walkable;
			indices[index] = index;
			index++;
		}
	}

	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->vertexBuffer);

	// Set up the description of the index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &node->indexBuffer);

	// Release the vertex and index arrays now that the data is stored in the buffers in the node.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return;
}

int Terrain::CountTriangles(float positionX, float positionZ, float width)
{
	int count, i;
	bool result;


	// Initialize the count to zero.
	count = 0;

	// Go through all the triangles in the entire mesh and check which ones should be inside this node.
	for (i = 0; i < m_triangleCount; i++)
	{
		// If the triangle is inside the node then increment the count by one.
		result = IsTriangleContained(i, positionX, positionZ, width);
		if (result == true)
		{
			count++;
		}
	}

	return count;
}
bool Terrain::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	float radius;
	int vertexIndex;
	float x1, z1, x2, z2, x3, z3;
	float minimumX, maximumX, minimumZ, maximumZ;


	// Calculate the radius of this node.
	radius = width / 2.0f;

	// Get the index into the vertex list.
	vertexIndex = index * 3;

	// Get the three vertices of this triangle from the vertex list.
	x1 = m_vertexList[vertexIndex].position.x;
	z1 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	x2 = m_vertexList[vertexIndex].position.x;
	z2 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	x3 = m_vertexList[vertexIndex].position.x;
	z3 = m_vertexList[vertexIndex].position.z;

	// Check to see if the minimum of the x coordinates of the triangle is inside the node.
	minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
	{
		return false;
	}

	// Check to see if the maximum of the x coordinates of the triangle is inside the node.
	maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
	{
		return false;
	}

	// Check to see if the minimum of the z coordinates of the triangle is inside the node.
	minimumZ = min(z1, min(z2, z3));
	if (minimumZ > (positionZ + radius))
	{
		return false;
	}

	// Check to see if the maximum of the z coordinates of the triangle is inside the node.
	maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
	{
		return false;
	}

	return true;
}
bool Terrain::GetHeightAtPosition(float positionX, float positionZ, float& height, bool& canWalk)
{
	float meshMinX, meshMaxX, meshMinZ, meshMaxZ;


	meshMinX = m_parentNode->positionX - (m_parentNode->width / 2.0f);
	meshMaxX = m_parentNode->positionX + (m_parentNode->width / 2.0f);

	meshMinZ = m_parentNode->positionZ - (m_parentNode->width / 2.0f);
	meshMaxZ = m_parentNode->positionZ + (m_parentNode->width / 2.0f);

	////Make sure the coordinates are actually over a polygon.
	//if ((positionX < meshMinX) || (positionX > meshMaxX) || (positionZ < meshMinZ) || (positionZ > meshMaxZ))
	//{
	//	return false;
	//}

	// Find the node which contains the polygon for this position.
	FindNode(m_parentNode, positionX, positionZ, height, canWalk);

	return true;
}
void Terrain::FindNode(NodeType* node, float x, float z, float& height, bool& canWalk)
{
	float xMin, xMax, zMin, zMax;
	int count, i, index;
	float vertex1[3], vertex2[3], vertex3[3];
	bool foundHeight;


	// Calculate the dimensions of this node.
	xMin = node->positionX - (node->width / 2.0f);
	xMax = node->positionX + (node->width / 2.0f);

	zMin = node->positionZ - (node->width / 2.0f);
	zMax = node->positionZ + (node->width / 2.0f);

	// See if the x and z coordinate are in this node, if not then stop traversing this part of the tree.
	if ((x < xMin) || (x > xMax) || (z < zMin) || (z > zMax))
	{
		return;
	}

	// If the coordinates are in this node then check first to see if children nodes exist.
	count = 0;

	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i] != 0)
		{
			count++;
			FindNode(node->nodes[i], x, z, height, canWalk);
		}
	}

	// If there were children nodes then return since the polygon will be in one of the children.
	if (count > 0)
	{
		return;
	}

	// If there were no children then the polygon must be in this node.  Check all the polygons in this node to find 
	// the height of which one the polygon we are looking for.
	for (i = 0; i < node->triangleCount; i++)
	{
		index = i * 3;
		float b1 = 0.0f, b2 = 0.0f, b3 = 0.0f;
		vertex1[0] = node->vertexArray[index].x;
		vertex1[1] = node->vertexArray[index].y;
		vertex1[2] = node->vertexArray[index].z;
		b1 = node->vertexArray[index].walkable;
		index++;

		vertex2[0] = node->vertexArray[index].x;
		vertex2[1] = node->vertexArray[index].y;
		vertex2[2] = node->vertexArray[index].z;
		b2 = node->vertexArray[index].walkable;

		index++;
		vertex3[0] = node->vertexArray[index].x;
		vertex3[1] = node->vertexArray[index].y;
		vertex3[2] = node->vertexArray[index].z;
		b3 = node->vertexArray[index].walkable;

		// Check to see if this is the polygon we are looking for.
		foundHeight = CheckHeightOfTriangle(x, z, height, vertex1, vertex2, vertex3);

		// If this was the triangle then quit the function and the height will be returned to the calling function.
		if (foundHeight)
		{
			if (b1 >= 1.0f && b2 >= 1.0f && b3 >= 1.0f)
				canWalk = true;

			return;
		}
	}

	return;
}
bool Terrain::CheckHeightOfTriangle(float x, float z, float& height, float v0[3], float v1[3], float v2[3])
{
	float startVector[3], directionVector[3], edge1[3], edge2[3], normal[3];
	float Q[3], e1[3], e2[3], e3[3], edgeNormal[3], temp[3];
	float magnitude, D, denominator, numerator, t, determinant;


	// Starting position of the ray that is being cast.
	startVector[0] = x;
	startVector[1] = 0.0f;
	startVector[2] = z;

	// The direction the ray is being cast.
	directionVector[0] = 0.0f;
	directionVector[1] = -1.0f;
	directionVector[2] = 0.0f;

	// Calculate the two edges from the three points given.
	edge1[0] = v1[0] - v0[0];
	edge1[1] = v1[1] - v0[1];
	edge1[2] = v1[2] - v0[2];

	edge2[0] = v2[0] - v0[0];
	edge2[1] = v2[1] - v0[1];
	edge2[2] = v2[2] - v0[2];

	// Calculate the normal of the triangle from the two edges.
	normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
	normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
	normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

	magnitude = (float)sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));
	normal[0] = normal[0] / magnitude;
	normal[1] = normal[1] / magnitude;
	normal[2] = normal[2] / magnitude;

	// Find the distance from the origin to the plane.
	D = ((-normal[0] * v0[0]) + (-normal[1] * v0[1]) + (-normal[2] * v0[2]));

	// Get the denominator of the equation.
	denominator = ((normal[0] * directionVector[0]) + (normal[1] * directionVector[1]) + (normal[2] * directionVector[2]));

	// Make sure the result doesn't get too close to zero to prevent divide by zero.
	if (fabs(denominator) < 0.0001f)
	{
		return false;
	}

	// Get the numerator of the equation.
	numerator = -1.0f * (((normal[0] * startVector[0]) + (normal[1] * startVector[1]) + (normal[2] * startVector[2])) + D);

	// Calculate where we intersect the triangle.
	t = numerator / denominator;

	// Find the intersection vector.
	Q[0] = startVector[0] + (directionVector[0] * t);
	Q[1] = startVector[1] + (directionVector[1] * t);
	Q[2] = startVector[2] + (directionVector[2] * t);

	// Find the three edges of the triangle.
	e1[0] = v1[0] - v0[0];
	e1[1] = v1[1] - v0[1];
	e1[2] = v1[2] - v0[2];

	e2[0] = v2[0] - v1[0];
	e2[1] = v2[1] - v1[1];
	e2[2] = v2[2] - v1[2];

	e3[0] = v0[0] - v2[0];
	e3[1] = v0[1] - v2[1];
	e3[2] = v0[2] - v2[2];

	// Calculate the normal for the first edge.
	edgeNormal[0] = (e1[1] * normal[2]) - (e1[2] * normal[1]);
	edgeNormal[1] = (e1[2] * normal[0]) - (e1[0] * normal[2]);
	edgeNormal[2] = (e1[0] * normal[1]) - (e1[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v0[0];
	temp[1] = Q[1] - v0[1];
	temp[2] = Q[2] - v0[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Calculate the normal for the second edge.
	edgeNormal[0] = (e2[1] * normal[2]) - (e2[2] * normal[1]);
	edgeNormal[1] = (e2[2] * normal[0]) - (e2[0] * normal[2]);
	edgeNormal[2] = (e2[0] * normal[1]) - (e2[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v1[0];
	temp[1] = Q[1] - v1[1];
	temp[2] = Q[2] - v1[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Calculate the normal for the third edge.
	edgeNormal[0] = (e3[1] * normal[2]) - (e3[2] * normal[1]);
	edgeNormal[1] = (e3[2] * normal[0]) - (e3[0] * normal[2]);
	edgeNormal[2] = (e3[0] * normal[1]) - (e3[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v2[0];
	temp[1] = Q[1] - v2[1];
	temp[2] = Q[2] - v2[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Now we have our height.
	height = Q[1];

	return true;
}