////////////////////////////////////////////////////////////////////////////////
// Filename: water_vs.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix reflectionMatrix;
};

cbuffer CamNormBuffer
{
	float3 cameraPosition;
	float padding1;
	float2 normalMapTiling;
	float2 padding2;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 reflectionPosition : TEXCOORD0;
    float4 refractionPosition : TEXCOORD1;
	float3 viewDirection : TEXCOORD2;
    float2 tex1 : TEXCOORD3;
    float2 tex2 : TEXCOORD4;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType WaterVertexShader(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
	matrix viewProjectWorld;
	float4 worldPosition;

    
	// 적절한 행렬 계산을 위해 위치 벡터를 4 단위로 변경합니다.
    input.position.w = 1.0f;

	// 월드, 뷰 및 투영 행렬에 대한 정점의 위치를 ​​계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// 반사 투영 세계 행렬을 생성합니다.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

	// reflectProjectWorld 행렬에 대한 입력 위치를 계산합니다.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);

	// 굴절을위한 뷰 투영 세계 행렬을 생성합니다.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);
   
	// viewProjectWorld 행렬에 대해 입력 위치를 계산합니다.
    output.refractionPosition = mul(input.position, viewProjectWorld);
	
	 // 세계의 정점 위치를 계산합니다.
    worldPosition = mul(input.position, worldMatrix);

    // 카메라의 위치와 세계의 정점 위치를 기준으로 보기 방향을 결정합니다.
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

    // 뷰 방향 벡터를 표준화합니다.
    output.viewDirection = normalize(output.viewDirection);

	// 수질 쿼드 위에 수차 법 맵을 여러 번 타일링하기 위해 두 개의 다른 텍스처 샘플 좌표를 생성합니다.
    output.tex1 = input.tex / normalMapTiling.x;
    output.tex2 = input.tex / normalMapTiling.y;

	return output;
}