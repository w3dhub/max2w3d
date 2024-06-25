#ifndef TT_INCLUDE__VECTOR2_H
#define TT_INCLUDE__VECTOR2_H
#include "wwmath.h"
#include "vector2i.h"

class Vector2 {
public:
	union {
		float X;
		float U;
	};
	union {
		float Y;
		float V;
	};
	TT_INLINE Vector2(float x, float y)
	{
		X = x;
		Y = y;
	}
	TT_INLINE Vector2(const Vector2 &v)
	{
		X = v.X;
		Y = v.Y;
	}
	TT_INLINE Vector2()
	{
		X = Y = 0.0f;
	}
	TT_INLINE explicit Vector2(const float vector[2])
	{
		TT_ASSERT(vector != nullptr);
		X = vector[0];
		Y = vector[1];
	}
	TT_INLINE Vector2 &operator= (const Vector2 & v)
	{
		X = v[0];
		Y = v[1];
		return *this;
	}
	TT_INLINE void Set(float x, float y)
	{
		X = x;
		Y = y;
	}
	TT_INLINE void Set(const Vector2 & v)
	{
		X = v.X;
		Y = v.Y;
	}
	TT_INLINE float &operator[](int i)
	{
		return (&X)[i];
	}
	TT_INLINE const float &operator[](int i) const
	{
		return (&X)[i];
	}
	TT_INLINE void Normalize(void)
	{
		float len2 = WWMATH_FLOAT_TINY + Length2();
		float oolen = WWMath::Inv_Sqrt(len2);
		X *= oolen;
		Y *= oolen;
	}
	TT_INLINE float Length(void) const
	{
		return (float)WWMath::Sqrt(Length2());
	}
	TT_INLINE float Length2(void) const
	{
		return (X * X + Y * Y);
	}
	TT_INLINE Vector2 operator-() const
	{
		return Vector2(-X,-Y);
	}
	TT_INLINE Vector2 operator+() const
	{
		return *this;
	}
	TT_INLINE Vector2& operator+= (const Vector2 &v)
	{
		X += v.X;
		Y += v.Y;
		return *this;
	}
	TT_INLINE Vector2& operator-= (const Vector2 & v)
	{
		X -= v.X;
		Y -= v.Y;
		return *this;
	}
	TT_INLINE Vector2& operator*= (float k)
	{
		X = (float)(X*k);
		Y = (float)(Y*k);
		return *this;
	}
	TT_INLINE Vector2& operator/= (float k)
	{
		k=1.0f/k;
		X*=k;
		Y*=k;
		return *this;
	}
	TT_INLINE static float Dot_Product(const Vector2 &a,const Vector2 &b)
	{
		return a * b;
	}
	TT_INLINE static float Perp_Dot_Product(const Vector2 &a,const Vector2 &b)
	{
		return a.X * -b.Y + a.Y * b.X;
	}
	TT_INLINE void Rotate(float theta)
	{
		Rotate(sinf(theta), cosf(theta));
	}
	TT_INLINE void Rotate(float s, float c)
	{
		float new_x = X * c + Y * -s;
		float new_y = X * s + Y * c;
		X = new_x;
		Y = new_y;
	}
	TT_INLINE bool Rotate_Towards_Vector(Vector2 &target, float max_theta, bool & positive_turn)
	{
		return Rotate_Towards_Vector(target, sinf(max_theta), cosf(max_theta), positive_turn);
	}
	TT_INLINE bool Rotate_Towards_Vector(Vector2 &target, float max_s, float max_c, bool & positive_turn)
	{
		positive_turn = Vector2::Perp_Dot_Product(target, *this) > 0.0f;
		if (Vector2::Dot_Product(*this, target) >= max_c)
		{
			Set(target);
			return true;
		}
		else
		{
			if (positive_turn)
			{
				Rotate(max_s, max_c);
			}
			else
			{
				Rotate(-max_s, max_c);
			}
		}
		return false;
	}
	TT_INLINE static Vector2 Rotate_Around_Reference(const Vector2& point, const Vector2& reference, float theta) {
		Vector2 result = point - reference;

		float resX = result.X * WWMath::Cos(theta) - result.Y * WWMath::Sin(theta);
		float resY = result.X * WWMath::Sin(theta) + result.Y * WWMath::Cos(theta);

		result.X = resX + reference.X;
		result.Y = resY + reference.Y;

		return result;
	}
	TT_INLINE void Update_Min(const Vector2 & a)
	{
		if (a.X < X)
		{
			X = a.X;
		}
		if (a.Y < Y)
		{
			Y = a.Y;
		}
	}
	TT_INLINE void Update_Max(const Vector2 & a)
	{
		if (a.X > X)
		{
			X = a.X;
		}
		if (a.Y > Y)
		{
			Y = a.Y;
		}
	}
	TT_INLINE void Scale(float a, float b)
	{
		X *= a;
		Y *= b;
	}
	TT_INLINE void Scale(const Vector2 & a)
	{
		X *= a.X;
		Y *= a.Y;
	}
	TT_INLINE void Unscale(const Vector2& a)
	{
		X /= a.X;
		Y /= a.Y;
	}
	TT_INLINE static float Distance(const Vector2 &p1, const Vector2 &p2)
	{
		Vector2 temp = p1 - p2;
		return (temp.Length());
	}
	TT_INLINE static void Lerp(const Vector2 &a,const Vector2 &b,float t,Vector2 * set_result)
	{
		set_result->X = (a.X + (b.X - a.X) * t);
		set_result->Y = (a.Y + (b.Y - a.Y) * t);
	}
	TT_INLINE void Floor()
	{
		X = floorf(X);
		Y = floorf(Y);
	};
    TT_INLINE void Snap_To_Units(float u)
    {
        X = (int)(X / u + 0.5f) * u;
        Y = (int)(Y / u + 0.5f) * u;
    }

	friend Vector2 operator* (const Vector2 &a,float k);
	friend Vector2 operator* (float k,const Vector2 &a);
	friend Vector2 operator/ (const Vector2 &a,float k);
	friend Vector2 operator+ (const Vector2 &a,const Vector2 &b);
	friend Vector2 operator- (const Vector2 &a,const Vector2 &b);
	friend float operator* (const Vector2 &a,const Vector2 &b);
	friend bool operator== (const Vector2 &a,const Vector2 &b);
	friend bool operator!= (const Vector2 &a,const Vector2 &b);

	friend Vector2 operator/(const Vector2& a, const Vector2& b) { return Vector2(a.X / b.X, a.Y / b.Y); }
	friend Vector2 operator/(const Vector2& a, const Vector2i& b) { return Vector2(a.X / b.I, a.Y / b.J); }
};
TT_INLINE Vector2 operator* (const Vector2 &a,float k)
{
	return Vector2(a[0] * k,a[1] * k);
}
TT_INLINE Vector2 operator* (float k, const Vector2 &a)
{
	return Vector2(a[0] * k,a[1] * k);
}
TT_INLINE Vector2 operator/ (const Vector2 &a,float k)
{
	return Vector2(a[0] * (1.0f/k),a[1] * (1.0f/k));
}
TT_INLINE Vector2 operator+ (const Vector2 &a,const Vector2 &b)
{
	return Vector2(a.X + b.X,a.Y + b.Y);
}
TT_INLINE Vector2 operator- (const Vector2 &a,const Vector2 &b)
{
	return Vector2(a.X - b.X,a.Y - b.Y);
}
TT_INLINE float operator* (const Vector2 &a,const Vector2 &b)
{
	return a.X * b.X + a.Y * b.Y;
}
TT_INLINE bool operator== (const Vector2 &a,const Vector2 &b)
{
	return (a.X == b.X) | (a.Y == b.Y);
}
TT_INLINE bool operator!= (const Vector2 &a,const Vector2 &b)
{
	return (a.X != b.X) | (a.Y != b.Y);
}
TT_INLINE Vector2 Normalize(const Vector2 &vec)
{
		float len2 = WWMATH_FLOAT_TINY + vec.Length2();
		return vec * WWMath::Inv_Sqrt(len2);
}
TT_INLINE void Swap(Vector2 & a,Vector2 & b)
{
	Vector2 tmp(a);
	a = b;
	b = tmp;
}
TT_INLINE float Distance(float x1, float y1, float x2, float y2)
{
	float x_diff = x1 - x2;
	float y_diff = y1 - y2;
	return (WWMath::Sqrt((x_diff * x_diff) + (y_diff * y_diff)));
}
TT_INLINE bool Almost_Equal(const Vector2 &a, const Vector2 &b)
{
	return ((WWMath::Floats_Almost_Equal(a.X, b.X))
			&& (WWMath::Floats_Almost_Equal(a.Y, b.Y)));
}

TT_INLINE bool Almost_Equal(const Vector2 &a, const Vector2 &b, float epsilon)
{
	return ((WWMath::Floats_Almost_Equal(a.X, b.X, epsilon))
		&& (WWMath::Floats_Almost_Equal(a.Y, b.Y, epsilon)));
}
#endif
