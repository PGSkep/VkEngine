#ifndef	MATH_3D_H
#define MATH_3D_H

namespace Math3D
{
	struct Vec2
	{
		float x, y;
		Vec2();
		Vec2(float _x, float _y);
	};

	struct Vec3
	{
		float x, y, z;
		Vec3();
		Vec3(float _x, float _y, float _z);

		Vec3& operator-(const Vec3 _source);
	};
	// Other
	float Lenght(const Vec3 _vector);
	Vec3 Normalize(const Vec3 _vector);
	Vec3 Cross(const Vec3 _vectorA, const Vec3 _vectorB);
	float Dot(const Vec3 _vectorA, const Vec3 _vectorB);

	struct Quat
	{
		float x, y, z, w;
		Quat();
		Quat(float _x, float _y, float _z, float _w);
	};
	// Get
	Quat GetQuatIdentity();

	struct Mat4
	{
		float	xx, xy, xz, xw,
			yx, yy, yz, yw,
			zx, zy, zz, zw,
			wx, wy, wz, ww;

		Mat4();
		Mat4(float _xx, float _xy, float _xz, float _xw,
			float _yx, float _yy, float _yz, float _yw,
			float _zx, float _zy, float _zz, float _zw,
			float _wx, float _wy, float _wz, float _ww);

		void operator+=(const Mat4& _source);
		void operator-=(const Mat4& _source);

		void operator*=(const float _source);
		void operator*=(const Mat4& _source);

		Mat4 operator*(const Mat4& _source);
	};
	// Get
	Mat4 GetMat4Identity();
	Mat4 GetTranslateMatrix(Vec3 _translate);
	Mat4 GetRotateXMatrix(float _x);
	Mat4 GetRotateYMatrix(float _y);
	Mat4 GetRotateZMatrix(float _z);
	Mat4 GetRotateMatrix(Vec3 _rotate);
	Mat4 GetScaleMatrix(Vec3 _scale);
	Mat4 GetPerspectiveMatrix(float _fov, float _aspect, float _near, float _far);
	Mat4 GetLookAt(Vec3 _eye, Vec3 _center, Vec3 _up);

	// Extract
	Vec3 ExtractPosition(Mat4 _matrix);
	Vec3 ExtractFront(Mat4 _matrix);
	Vec3 ExtractRight(Mat4 _matrix);
	Vec3 ExtractUp(Mat4 _matrix);
	Vec3 ExtractScale(Mat4 _matrix);
}

#endif