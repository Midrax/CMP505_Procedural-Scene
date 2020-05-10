// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
};

float4 main(InputType input) : SV_TARGET
{
	float4	textureColor1;
	float4	textureColor2;
	float4	textureColor3;
	float3	lightDir;
    float	lightIntensity;
    float4	color;
	float blendAmount;
	float slope = input.normal.y + 1;
	float threshMin = 0.2f, threshMax = 0.65f;

	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity);
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor1 = shaderTexture1.Sample(SampleType, input.tex);
	textureColor2 = shaderTexture2.Sample(SampleType, input.tex);
	textureColor3 = shaderTexture3.Sample(SampleType, input.tex);

	// Height Texturing
	float variance = 0;
	float yValue = input.position3D.y + 2;

	if (yValue > 0.2) {
		variance = 1;
	}
	else if (yValue < -0.2) {
		variance = 0;
	}
	else {
		variance = (yValue + 0.2) / 0.4;
	}

	float mf = 0.5;
	// Slope Texturing
	if (slope <= threshMin) // If blend = 1 Slope Texture is true;
	{
		blendAmount = 0;
	}
	else if (slope > threshMin && input.normal.y <= threshMax)
	{
		blendAmount = (slope - threshMin)/(threshMax-threshMin);
	}
	else if (slope > threshMax) 
	{
		blendAmount = 1;
	}
	color = color * (((textureColor1 * variance) + (textureColor2 * (1 - variance))) * (1 - blendAmount) + textureColor3 * blendAmount);
    return color;
}

