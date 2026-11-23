#ifndef H_DVECTOR2
#define H_DVECTOR2

#include <iostream>
#include <cmath>

template <typename T>
class DVector2
{
public:
	
	// Constructors
	DVector2()
	{
		x = 0;
		y = 0;
		index = 0;
	}

	DVector2(T _x, T _y, int _index = 0)
	{
		x = _x;
		y = _y;
		index = _index;
	}

	DVector2(const DVector2& v)
	{
		x = v.x;
		y = v.y;
		index = v.index;
	}

	void set(const DVector2& v)
	{
		x = v.x;
		y = v.y;
		index = v.index;
	}

	// Operations
	T dist2(const DVector2& v)
	{
		T dx = x - v.x;
		T dy = y - v.y;
		return dx * dx + dy * dy;
	}

	float dist(const DVector2& v)
	{
		return sqrtf(dist2(v));
	}

	T x;
	T y;
	int index;
};

template<typename T>
std::ostream& operator << (std::ostream& str, DVector2<T> const& point)
{
	return str << "Point x: " << point.x << " y: " << point.y;
}

template<typename T>
bool operator == (DVector2<T> v1, DVector2<T> v2)
{
	return (v1.x == v2.x) && (v1.y == v2.y);
}

#endif
