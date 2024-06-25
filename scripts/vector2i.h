#pragma once
class Vector2i
{

public:
	Vector2i() : I(0), J(0)
	{
	}

	Vector2i(int i, int j)
	{
		I = i; J = j;
	}
	void Set(int i, int j)
	{
		I = i;
		J = j;
	}
	bool operator==(const Vector2i &v) const {
		return I == v.I && J == v.J;
	}
	bool operator!=(const Vector2i &v) const {
		return !(*this == v);
	}
	int I;
	int J;
};
