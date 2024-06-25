#pragma once
class Matrix3D;
class EulerAngles
{
	double        X = 0;
	double        Y = 0;
	double        Z = 0;
	static const uint32_t DefaultFlags = 0x15;
public:
	void FromMatrix3D(const Matrix3D& matrix, uint32_t flags = DefaultFlags);
	double operator[](int index) { return (&X)[index]; }
};
