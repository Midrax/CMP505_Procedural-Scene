/**
* Stores ambient, diffuse, specular colour, specular power. Also stores direction and position.
* Source code from CMP505 Lab modified by lecturer Matthew Bett, 2019
*/

#ifndef _LIGHT_H_
#define _LIGHT_H_

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Light
{

public:
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	Light();
	~Light();

	// Setters
	void setAmbientColour(float red, float green, float blue, float alpha);		///< Set ambient colour RGBA
	void setDiffuseColour(float red, float green, float blue, float alpha);		///< Set diffuse colour RGBA
	void setDirection(float x, float y, float z);								///< Set light direction (for directional lights)
	void setSpecularColour(float red, float green, float blue, float alpha);	///< set specular colour RGBA
	void setSpecularPower(float power);											///< Set specular power
	void setPosition(float x, float y, float z);								///< Set light position (for point lights)
	void setLookAt(float x, float y, float z);									///< Set light lookAt (near deprecation)

	// Getters
	Vector4 getAmbientColour();		///< Get ambient colour, returns float4
	Vector4 getDiffuseColour();		///< Get diffuse colour, returns float4
	Vector3 getDirection();			///< Get light direction, returns float3
	Vector4 getSpecularColour();	///< Get specular colour, returns float4
	float getSpecularPower();		///< Get specular power, returns float
	Vector3 getPosition();			///< Get light position, returns float3


protected:
	Vector4 m_ambientColour;
	Vector4 m_diffuseColour;
	Vector3 m_direction;
	Vector4 m_specularColour;
	float m_specularPower;
	Vector3 m_position;
	Vector4 m_lookAt;
};

#endif