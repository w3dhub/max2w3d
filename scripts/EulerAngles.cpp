#include "General.h"
#include "EulerAngles.h"
#include "Matrix3D.h"

void EulerAngles::FromMatrix3D(const Matrix3D& matrix, uint32_t flags)
{
	const static uint32_t s_IndexLutA[4] = { 0, 1, 2, 0 };
	const static uint32_t s_IndexLutB[4] = { 1, 2, 0, 1 };

	const uint32_t flag0 = (flags >> 0) & 1;
	const uint32_t flag1 = (flags >> 1) & 1;
	const uint32_t flag2 = (flags >> 2) & 1;

	const uint32_t index = (flags >> 3) & 3;

	const uint32_t index0 = s_IndexLutA[index];

	const uint32_t index1 = s_IndexLutB[index0 + flag2];
	const uint32_t index2 = s_IndexLutB[1 + index0 - flag2];

	const double epsilon = 0.0000019;

	if (flag1 == 1)
	{
		const double x = matrix[index0][index1];
		const double y = matrix[index0][index2];

		const double length = WWMath::Sqrt(x * x + y * y);
		if (length > epsilon)
		{
			X = atan2(x, y);
			Y = atan2(length, double(matrix[index0][index0]));
			Z = atan2(double(matrix[index1][index0]), -double(matrix[index2][index0]));
		}
		else
		{
			X = atan2(-double(matrix[index1][index2]), double(matrix[index1][index1]));
			Y = atan2(length, double(matrix[index0][index0]));
			Z = 0.0;
		}
	}
	else
	{
		const double x = matrix[index1][index0];
		const double y = matrix[index0][index0];
		const double length = WWMath::Sqrt(x * x + y * y);

		if (length > epsilon)
		{
			X = atan2(double(matrix[index2][index1]), double(matrix[index2][index2]));
			Y = atan2(-double(matrix[index2][index0]), length);
			Z = atan2(x, y);
		}
		else
		{
			X = atan2(-double(matrix[index1][index2]), double(matrix[index1][index1]));
			Y = atan2(-double(matrix[index2][index0]), length);
			Z = 0.0f;
		}
	}
	if (flag2 == 1)
	{
		X = -X;
		Y = -Y;
		Z = -Z;
	}
	if (flag0 == 1)
	{
		double temp = Z;
		Z = X;
		X = temp;
	}
	if (flags == DefaultFlags)
	{
		double newX = X + M_PI;
		double newY = M_PI - Y;
		double newZ = Z + M_PI;

		if (newX > M_PI) {
			newX = newX - (M_PI * 2.0);
		}
		if (newY > M_PI) {
			newY = newY - (M_PI * 2.0);
		}
		if (newZ > M_PI) {
			newZ = newZ - (M_PI * 2.0);
		}

		const double lengthSquared =
			X * X + Y * Y + Z * Z;

		const double newLengthSquared =
			newX * newX + newY * newY + newZ * newZ;

		if (lengthSquared > newLengthSquared) {
			X = newX;
			Y = newY;
			Z = newZ;
		}
	}
}
