#pragma once
#include "pch.h"
using namespace std;
using namespace DirectX;

class QuadTree
{

private:

	struct VertexType
	{
		Vector3 position;
		Vector2 texture;
		Vector3 normal;
		float walkable = 0.0f;
	};

	struct VectorType
	{
		float x, y, z;
		float walkable = 0.0f;
	};

	struct NodeType
	{
		float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer* vertexBuffer, * indexBuffer;
		NodeType* nodes[4];
		VectorType* vertexArray;
		int* mainTextureIndex;
	};

public:
	QuadTree();
	QuadTree(const QuadTree&);
	~QuadTree();

	bool Initialize(Terrain*, ID3D11Device*);
	void Shutdown();
	//void Render(Frustum*, ID3D11DeviceContext*, TerrainShader*);
	void ReinitializeBuffers(Terrain*, ID3D11Device* device);
	int GetDrawCount();
	bool GetHeightAtPosition(float, float, float&, bool& canWalk);

private:
	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(NodeType*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);
	bool IsTriangleContained(int, float, float, float);
	void ResetNodeBuffers(NodeType*, ID3D11Device*);
	void ReleaseNode(NodeType*);
	//void RenderNode(NodeType*, Frustum*, ID3D11DeviceContext*, TerrainShader*);
	void FindNode(NodeType*, float, float, float&, bool& canWalk);
	bool CheckHeightOfTriangle(float, float, float&, float[3], float[3], float[3]);


private:
	int m_triangleCount, m_drawCount;

	VertexType* m_vertexList;

	NodeType* m_parentNode;
};

