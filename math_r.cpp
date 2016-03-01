
struct Vec2
{
	float X;
	float Y;
};

struct Vec3
{
	float X;
	float Y;
	float Z;
};

union Matrix2x2
{
	float values[4];
	struct
	{
		float a11;
		float a12;
		float a21;
		float a22;
	};
};

union Matrix3x3
{
	float values[9];
	struct
	{
		float a11;
		float a12;
		float a13;
		float a21;
		float a22;
		float a23;
		float a31;
		float a32;
		float a33;
	};
};

struct Edge
{
	int edge1;
	int edge2;
};

inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
	Vec2 result = { lhs.X + rhs.X, lhs.Y + rhs.Y };
	return result;
}

inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
	Vec2 result = { lhs.X - rhs.X, lhs.Y - rhs.Y };
	return result;
}

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
	Vec3 result = { lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z};
	return result;
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
	Vec3 result = { lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z};
	return result;
}

inline Vec2 operator*(const Matrix2x2& lhs, const Vec2& rhs)
{
	Vec2 result = { 0 };
	result.X = lhs.a11 * rhs.X + lhs.a12 * rhs.Y;
	result.Y = lhs.a21 * rhs.X + lhs.a22 * rhs.Y;
	return result;
}

inline Vec3 operator*(const Matrix3x3& lhs, const Vec3& rhs)
{
	Vec3 result = { 0 };
	result.X = lhs.a11 * rhs.X + lhs.a12 * rhs.Y + lhs.a13 * rhs.Z;
	result.Y = lhs.a21 * rhs.X + lhs.a22 * rhs.Y + lhs.a23 * rhs.Z;
	result.Z = lhs.a31 * rhs.X + lhs.a32 * rhs.Y + lhs.a33 * rhs.Z;
	return result;
}

inline Vec3 operator*(const Vec3& lhs, const float& rhs)
{
	Vec3 result = { lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs };
	return result;
}

inline Matrix2x2 operator*(const Matrix2x2& lhs, const Matrix2x2& rhs)
{
	Matrix2x2 result = { 0 };
	result.a11 = lhs.a11 * rhs.a11 + lhs.a12 * rhs.a21;
	result.a12 = lhs.a11 * rhs.a12 + lhs.a12 * rhs.a22;
	result.a21 = lhs.a21 * rhs.a11 + lhs.a22 * rhs.a21;
	result.a22 = lhs.a21 * rhs.a12 + lhs.a22 * rhs.a22;
	return result;
}

inline Matrix3x3 operator*(const Matrix3x3 lhs, const Matrix3x3 rhs)
{
	Matrix3x3 result = { 0 };
	result.a11 = lhs.a11 * rhs.a11 + lhs.a12 * rhs.a21 + lhs.a13 * rhs.a31;
	result.a12 = lhs.a11 * rhs.a12 + lhs.a12 * rhs.a22 + lhs.a13 * rhs.a32;
	result.a13 = lhs.a11 * rhs.a13 + lhs.a12 * rhs.a23 + lhs.a13 * rhs.a33;
	result.a21 = lhs.a21 * rhs.a11 + lhs.a22 * rhs.a21 + lhs.a23 * rhs.a31;
	result.a22 = lhs.a21 * rhs.a12 + lhs.a22 * rhs.a22 + lhs.a23 * rhs.a32;
	result.a23 = lhs.a21 * rhs.a13 + lhs.a22 * rhs.a23 + lhs.a23 * rhs.a33;
	result.a31 = lhs.a31 * rhs.a11 + lhs.a32 * rhs.a21 + lhs.a33 * rhs.a31;
	result.a32 = lhs.a31 * rhs.a12 + lhs.a32 * rhs.a22 + lhs.a33 * rhs.a32;
	result.a33 = lhs.a31 * rhs.a13 + lhs.a32 * rhs.a23 + lhs.a33 * rhs.a33;
	return result;
}

inline Vec3 operator*(const float& lhs, const Vec3& rhs)
{
	return rhs * lhs;
}
