#pragma once

#include "wwmath.h"
#include "vector2.h"
#include "vector3.h"
class Vector4 
{
public:
    float X;
    float Y;
    float Z;
    float W;

    // constructors
	Vector4() : X(0), Y(0), Z(0), W(0)
	{
	}
    Vector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) { }
    Vector4(const Vector2& v, float z, float w) : X(v.X), Y(v.Y), Z(z), W(w) { }
    Vector4(const Vector3& v, float w) : X(v.X), Y(v.Y), Z(v.Z), W(w) { }
    explicit Vector4(const float v[4]) { TT_ASSERT(v != nullptr); X = v[0]; Y = v[1]; Z = v[2]; W = v[3]; }

    // conversion operators
    explicit operator Vector2&() { return *(Vector2*)&X; }
    explicit operator Vector3&() { return *(Vector3*)&X; }
    explicit operator const Vector2&() const { return *(Vector2*)&X; }
    explicit operator const Vector3&() const { return *(Vector3*)&X; }

	// assignment operators
	TT_INLINE void Set(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; };

	// array access operators
	TT_INLINE float& operator[](int i) { return (&X)[i]; };
	TT_INLINE const float& operator[](int i) const { return (&X)[i]; };
	TT_INLINE void Normalize(void)
	{
		float len2 = WWMATH_FLOAT_TINY + Length2();
		float oolen = WWMath::Inv_Sqrt(len2);
		X *= oolen;
		Y *= oolen;
		Z *= oolen;
		W *= oolen;
	}
	TT_INLINE float Length(void) const
	{
		return WWMath::Sqrt(Length2());
	}
	TT_INLINE float Length2(void) const
	{
		return X*X + Y*Y + Z*Z + W*W;
	}

	// unary operators
	TT_INLINE Vector4 operator-() const { return(Vector4(-X,-Y,-Z,-W)); } ;

	TT_INLINE Vector4& operator+=(const Vector4& v) { X += v.X; Y += v.Y; Z += v.Z; W += v.W; return *this;	};
	TT_INLINE Vector4& operator-=(const Vector4& v) { X -= v.X; Y -= v.Y; Z -= v.Z; W -= v.W; return *this;	};
	TT_INLINE Vector4& operator*=(float f) { X *= f; Y *= f; Z *= f; W *= f; return *this; };
	TT_INLINE Vector4& operator/=(float f)	{ X /= f; Y /= f; Z /= f; W /= f; return *this; };

	// vector addition/subtraction
	friend Vector4 operator+(const Vector4& a, const Vector4& b);
	friend Vector4 operator-(const Vector4& a, const Vector4& b);

	// scalar multiplication/division
	friend Vector4 operator*(const Vector4 &a, float b);
	friend Vector4 operator*(float a, const Vector4 &b);
	friend Vector4 operator/(const Vector4 &a, float b);

	// equality operators
	friend bool operator== (const Vector4 &a, const Vector4 &b);
	friend bool operator!= (const Vector4 &a, const Vector4 &b);

	friend float operator * (const Vector4 &a,const Vector4 &b);
	TT_INLINE static float Dot_Product(const Vector4 &a,const Vector4 &b)
	{
		return a * b;
	}
	TT_INLINE static Vector4 Lerp(const Vector4 & a, const Vector4 & b, float alpha)
	{
		return Vector4((a.X + (b.X - a.X)*alpha),(a.Y + (b.Y - a.Y)*alpha),(a.Z + (b.Z - a.Z)*alpha),(a.W + (b.W - a.W)*alpha));
	}
	TT_INLINE static void Lerp(const Vector4 & a, const Vector4 & b, float alpha,Vector4 * set_result)
	{
		set_result->X = (a.X + (b.X - a.X)*alpha);
		set_result->Y = (a.Y + (b.Y - a.Y)*alpha);
		set_result->Z = (a.Z + (b.Z - a.Z)*alpha);
		set_result->W = (a.W + (b.W - a.W)*alpha);
	}

	float Min_Element()
	{
		return WWMath::Min(WWMath::Min(X, Y), WWMath::Min(Z, W));
	}
	float Max_Element()
	{
		return WWMath::Max(WWMath::Max(X, Y), WWMath::Max(Z, W));
	}
};

TT_INLINE Vector4 operator+(const Vector4& a, const Vector4& b)
{
	return Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z,	a.W + b.W);
};

TT_INLINE Vector4 operator-(const Vector4& a, const Vector4& b)
{
	return Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z,	a.W - b.W);
};

TT_INLINE Vector4 operator*(const Vector4 &a, float b)
{
	return Vector4(a.X * b, a.Y * b, a.Z * b, a.W * b);
};

TT_INLINE Vector4 operator*(float a, const Vector4 &b)
{
	return b * a;
};

TT_INLINE Vector4 operator/(const Vector4 &a, float k)
{
	return Vector4(a.X / k, a.Y / k, a.Z / k, a.W / k);
};

TT_INLINE bool operator==(const Vector4 &a, const Vector4 &b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z) && (a.W == b.W);
};

TT_INLINE bool operator!=(const Vector4 &a, const Vector4 &b)
{
	return (a.X != b.X) || (a.Y != b.Y) || (a.Z != b.Z) || (a.W != b.W);
};

TT_INLINE float operator * (const Vector4 &a,const Vector4 &b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
};

TT_INLINE Vector4 Normalize(const Vector4 & vec)
{
	float len2 = WWMATH_FLOAT_TINY + vec.Length2();
	return vec * WWMath::Inv_Sqrt(len2);
}

TT_INLINE void Swap(Vector4 & a,Vector4 & b)
{
	Vector4 tmp(a);
	a = b;
	b = tmp;
}

TT_INLINE Vector4 Lerp(const Vector4 & a, const Vector4 & b, float alpha)
{
	return Vector4((a.X + (b.X - a.X) * alpha),(a.Y + (b.Y - a.Y) * alpha),(a.Z + (b.Z - a.Z) * alpha),(a.W + (b.W - a.W) * alpha));
}

TT_INLINE bool Almost_Equal(const Vector4 &a, const Vector4 &b)
{
	return ((WWMath::Floats_Almost_Equal(a.X, b.X))
		&& (WWMath::Floats_Almost_Equal(a.Y, b.Y))
		&& (WWMath::Floats_Almost_Equal(a.Z, b.Z))
		&& (WWMath::Floats_Almost_Equal(a.W, b.W)));
}

TT_INLINE bool Almost_Equal(const Vector4 &a, const Vector4 &b, float epsilon)
{
	return ((WWMath::Floats_Almost_Equal(a.X, b.X, epsilon))
		&& (WWMath::Floats_Almost_Equal(a.Y, b.Y, epsilon))
		&& (WWMath::Floats_Almost_Equal(a.Z, b.Z, epsilon))
		&& (WWMath::Floats_Almost_Equal(a.W, b.W, epsilon)));
}
