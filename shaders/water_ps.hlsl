////////////////////////////////////////////////////////////////////////////////
// Filename: water_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
SamplerState SampleType;
Texture2D refractionTexture : register(t0);
Texture2D reflectionTexture : register(t1);
Texture2D normalTexture : register(t2);

cbuffer WaterBuffer
{
	float4 refractionTint;
	float3 lightDirection;
	float waterTranslation;
	float reflectRefractScale;
	float specularShininess;
	float2 padding;
};


//////////////
// TYPEDEFS //
//////////////
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
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 WaterPixelShader(PixelInputType input) : SV_TARGET
{
	float4 normalMap1;
	float4 normalMap2;
	float3 normal1;
	float3 normal2;
	float3 normal;
	float2 refractTexCoord;
	float2 reflectTexCoord;
	float4 reflectionColor;
	float4 refractionColor;
	float3 heightView;
	float r;
	float fresnelFactor;
	float4 color;
	float3 reflection;
	float specular;


	input.tex1.y += waterTranslation;
	input.tex2.y += waterTranslation;

	normalMap1 = normalTexture.Sample(SampleType, input.tex1);
	normalMap2 = normalTexture.Sample(SampleType, input.tex2);
	
	normal1 = (normalMap1.rgb * 2.0f) - 1.0f;
	normal2 = (normalMap2.rgb * 2.0f) - 1.0f;

	normal = normalize(normal1 + normal2);
		
	refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;
	
	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
		
	reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
	refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);
	
	reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

	refractionColor = saturate(refractionColor * refractionTint);

	heightView.x = input.viewDirection.y;
	heightView.y = input.viewDirection.y;
	heightView.z = input.viewDirection.y;

	r = (1.2f - 1.0f) / (1.2f + 1.0f);
	fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normal, heightView), 2)));

	color = lerp(reflectionColor, refractionColor, fresnelFactor);

	reflection = -reflect(normalize(lightDirection), normal);
	
	specular = dot(normalize(reflection), normalize(input.viewDirection));

	if(specular > 0.0f)
	{
		specular = pow(specular, specularShininess);

		color = saturate(color + specular);
	}
	return color;
}