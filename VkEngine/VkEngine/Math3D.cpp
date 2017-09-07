#include "Math3D.h"

#include <math.h>

/// Vec2
Math3D::Vec2::Vec2()
{
	x = 0.0f;
	y = 0.0f;
}
Math3D::Vec2::Vec2(float _x, float _y)
{
	x = _x;
	y = _y;
}

/// Vec3
Math3D::Vec3::Vec3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}
Math3D::Vec3::Vec3(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

Math3D::Vec3& Math3D::Vec3::operator-(const Vec3 _source)
{
	x -= _source.x;
	y -= _source.y;
	z -= _source.z;

	return *this;
}

// Other
float Math3D::Lenght(const Vec3 _vector)
{
	return sqrtf((_vector.x * _vector.x) + (_vector.y * _vector.y) + (_vector.z * _vector.z));
}
Math3D::Vec3 Math3D::Normalize(const Vec3 _vector)
{
	float length = Lenght(_vector);
	return Vec3(_vector.x / length, _vector.y / length, _vector.z / length);
}
Math3D::Vec3 Math3D::Cross(const Vec3 _vectorA, const Vec3 _vectorB)
{
	return Vec3(
		_vectorA.y * _vectorB.z - _vectorB.y * _vectorA.z,
		_vectorA.z * _vectorB.x - _vectorB.z * _vectorA.x,
		_vectorA.x * _vectorB.y - _vectorB.x * _vectorA.y);
}
float Math3D::Dot(const Vec3 _vectorA, const Vec3 _vectorB)
{
	return _vectorA.x * _vectorB.x + _vectorA.y * _vectorB.y + _vectorA.z * _vectorB.z;
}

/// Quat
Math3D::Quat::Quat()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
}
Math3D::Quat::Quat(float _x, float _y, float _z, float _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

// Get
Math3D::Quat Math3D::GetQuatIdentity()
{
	Math3D::Quat quat;

	quat.x = 0.0f;
	quat.y = 0.0f;
	quat.z = 0.0f;
	quat.w = 1.0f;

	return quat;
}

/// Mat4
Math3D::Mat4::Mat4()
{
	xx = 0.0f;
	xy = 0.0f;
	xz = 0.0f;
	xw = 0.0f;

	yx = 0.0f;
	yy = 0.0f;
	yz = 0.0f;
	yw = 0.0f;

	zx = 0.0f;
	zy = 0.0f;
	zz = 0.0f;
	zw = 0.0f;

	wx = 0.0f;
	wy = 0.0f;
	wz = 0.0f;
	ww = 0.0f;
}
Math3D::Mat4::Mat4(float _xx, float _xy, float _xz, float _xw, float _yx, float _yy, float _yz, float _yw, float _zx, float _zy, float _zz, float _zw, float _wx, float _wy, float _wz, float _ww)
{
	xx = _xx;
	xy = _xy;
	xz = _xz;
	xw = _xw;

	yx = _yx;
	yy = _yy;
	yz = _yz;
	yw = _yw;

	zx = _zx;
	zy = _zy;
	zz = _zz;
	zw = _zw;

	wx = _wx;
	wy = _wy;
	wz = _wz;
	ww = _ww;
}

// Operator
void Math3D::Mat4::operator+=(const Mat4& _source)
{
	xx += _source.xx;
	xy += _source.xy;
	xz += _source.xz;
	xw += _source.xw;

	yx += _source.yx;
	yy += _source.yy;
	yz += _source.yz;
	yw += _source.yw;

	zx += _source.zx;
	zy += _source.zy;
	zz += _source.zz;
	zw += _source.zw;

	wx += _source.wx;
	wy += _source.wy;
	wz += _source.wz;
	ww += _source.ww;
}
void Math3D::Mat4::operator-=(const Mat4& _source)
{
	xx -= _source.xx;
	xy -= _source.xy;
	xz -= _source.xz;
	xw -= _source.xw;

	yx -= _source.yx;
	yy -= _source.yy;
	yz -= _source.yz;
	yw -= _source.yw;

	zx -= _source.zx;
	zy -= _source.zy;
	zz -= _source.zz;
	zw -= _source.zw;

	wx -= _source.wx;
	wy -= _source.wy;
	wz -= _source.wz;
	ww -= _source.ww;
}
void Math3D::Mat4::operator*=(const float _source)
{
	xx *= _source;
	xy *= _source;
	xz *= _source;
	xw *= _source;

	yx *= _source;
	yy *= _source;
	yz *= _source;
	yw *= _source;

	zx *= _source;
	zy *= _source;
	zz *= _source;
	zw *= _source;

	wx *= _source;
	wy *= _source;
	wz *= _source;
	ww *= _source;
}
void Math3D::Mat4::operator*=(const Mat4& _source)
{
	xx = xx*_source.xx + xy*_source.yx + xz*_source.zx + xw*_source.wx;
	xy = yx*_source.xx + yy*_source.yx + yz*_source.zx + yw*_source.wx;
	xz = zx*_source.xx + zy*_source.yx + zz*_source.zx + zw*_source.wx;
	xw = wx*_source.xx + wy*_source.yx + wz*_source.zx + ww*_source.wx;

	yx = xx*_source.xy + xy*_source.yy + xz*_source.zy + xw*_source.wy;
	yy = yx*_source.xy + yy*_source.yy + yz*_source.zy + yw*_source.wy;
	yz = zx*_source.xy + zy*_source.yy + zz*_source.zy + zw*_source.wy;
	yw = wx*_source.xy + wy*_source.yy + wz*_source.zy + ww*_source.wy;

	zx = xx*_source.xz + xy*_source.yz + xz*_source.zz + xw*_source.wz;
	zy = yx*_source.xz + yy*_source.yz + yz*_source.zz + yw*_source.wz;
	zz = zx*_source.xz + zy*_source.yz + zz*_source.zz + zw*_source.wz;
	zw = wx*_source.xz + wy*_source.yz + wz*_source.zz + ww*_source.wz;

	wx = xx*_source.xw + xy*_source.yw + xz*_source.zw + xw*_source.ww;
	wy = yx*_source.xw + yy*_source.yw + yz*_source.zw + yw*_source.ww;
	wz = zx*_source.xw + zy*_source.yw + zz*_source.zw + zw*_source.ww;
	ww = wx*_source.xw + wy*_source.yw + wz*_source.zw + ww*_source.ww;
}
Math3D::Mat4 Math3D::Mat4::operator*(const Mat4& _source)
{
	Mat4 result;

	result.xx = xx*_source.xx + xy*_source.yx + xz*_source.zx + xw*_source.wx;
	result.xy = yx*_source.xx + yy*_source.yx + yz*_source.zx + yw*_source.wx;
	result.xz = zx*_source.xx + zy*_source.yx + zz*_source.zx + zw*_source.wx;
	result.xw = wx*_source.xx + wy*_source.yx + wz*_source.zx + ww*_source.wx;

	result.yx = xx*_source.xy + xy*_source.yy + xz*_source.zy + xw*_source.wy;
	result.yy = yx*_source.xy + yy*_source.yy + yz*_source.zy + yw*_source.wy;
	result.yz = zx*_source.xy + zy*_source.yy + zz*_source.zy + zw*_source.wy;
	result.yw = wx*_source.xy + wy*_source.yy + wz*_source.zy + ww*_source.wy;

	result.zx = xx*_source.xz + xy*_source.yz + xz*_source.zz + xw*_source.wz;
	result.zy = yx*_source.xz + yy*_source.yz + yz*_source.zz + yw*_source.wz;
	result.zz = zx*_source.xz + zy*_source.yz + zz*_source.zz + zw*_source.wz;
	result.zw = wx*_source.xz + wy*_source.yz + wz*_source.zz + ww*_source.wz;

	result.wx = xx*_source.xw + xy*_source.yw + xz*_source.zw + xw*_source.ww;
	result.wy = yx*_source.xw + yy*_source.yw + yz*_source.zw + yw*_source.ww;
	result.wz = zx*_source.xw + zy*_source.yw + zz*_source.zw + zw*_source.ww;
	result.ww = wx*_source.xw + wy*_source.yw + wz*_source.zw + ww*_source.ww;

	return result;
}

// Get
Math3D::Mat4 Math3D::GetMat4Identity()
{
	Math3D::Mat4 mat;

	mat.xx = 1.0f;	mat.xy = 0.0f;	mat.xz = 0.0f;	mat.xw = 0.0f;
	mat.yx = 0.0f;	mat.yy = 1.0f;	mat.yz = 0.0f;	mat.yw = 0.0f;
	mat.zx = 0.0f;	mat.zy = 0.0f;	mat.zz = 1.0f;	mat.zw = 0.0f;
	mat.wx = 0.0f;	mat.wy = 0.0f;	mat.wz = 0.0f;	mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetTranslateMatrix(Vec3 _translate)
{
	Math3D::Mat4 mat;

	mat.xx = 1.0f; mat.xy = 0.0f; mat.xz = 0.0f; mat.xw = _translate.x;
	mat.yx = 0.0f; mat.yy = 1.0f; mat.yz = 0.0f; mat.yw = _translate.y;
	mat.zx = 0.0f; mat.zy = 0.0f; mat.zz = 1.0f; mat.zw = _translate.z;
	mat.wx = 0.0f; mat.wy = 0.0f; mat.wz = 0.0f; mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetRotateXMatrix(float _x)
{
	Math3D::Mat4 mat;

	mat.xx = 1.0f; mat.xy = 0.0f;		mat.xz = 0.0f;		mat.xw = 0.0f;
	mat.yx = 0.0f; mat.yy = cosf(_x);	mat.yz = -sinf(_x);	mat.yw = 0.0f;
	mat.zx = 0.0f; mat.zy = sinf(_x);	mat.zz = cosf(_x);	mat.zw = 0.0f;
	mat.wx = 0.0f; mat.wy = 0.0f;		mat.wz = 0.0f;		mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetRotateYMatrix(float _y)
{
	Math3D::Mat4 mat;

	mat.xx = cosf(_y);	mat.xy = 0.0f; mat.xz = sinf(_y);	mat.xw = 0.0f;
	mat.yx = 0.0f;		mat.yy = 1.0f; mat.yz = 0.0f;		mat.yw = 0.0f;
	mat.zx = -sinf(_y);	mat.zy = 0.0f; mat.zz = cosf(_y);	mat.zw = 0.0f;
	mat.wx = 0.0f;		mat.wy = 0.0f; mat.wz = 0.0f;		mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetRotateZMatrix(float _z)
{
	Math3D::Mat4 mat;

	mat.xx = cosf(_z);	mat.xy = -sinf(_z);	mat.xz = 0.0f; mat.xw = 0.0f;
	mat.yx = sinf(_z);	mat.yy = cosf(_z);	mat.yz = 0.0f; mat.yw = 0.0f;
	mat.zx = 0.0f;		mat.zy = 0.0f;		mat.zz = 1.0f; mat.zw = 0.0f;
	mat.wx = 0.0f;		mat.wy = 0.0f;		mat.wz = 0.0f; mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetRotateMatrix(Vec3 _rotate)
{
	return Mat4();
}
Math3D::Mat4 Math3D::GetScaleMatrix(Vec3 _scale)
{
	Math3D::Mat4 mat;

	mat.xx = _scale.x;	mat.xy = 0.0f;		mat.xz = 0.0f;		mat.xw = 0.0f;
	mat.yx = 0.0f;		mat.yy = _scale.y;	mat.yz = 0.0f;		mat.yw = 0.0f;
	mat.zx = 0.0f;		mat.zy = 0.0f;		mat.zz = _scale.z;	mat.zw = 0.0f;
	mat.wx = 0.0f;		mat.wy = 0.0f;		mat.wz = 0.0f;		mat.ww = 1.0f;

	return mat;
}
Math3D::Mat4 Math3D::GetPerspectiveMatrix(const float _fov, const float _aspect, const float _near, const float _far)
{
	Math3D::Mat4 mat;

	float aspect = _aspect;
	if (aspect < 0.0f)
		aspect *= -1;

	const float tanHalfFov = tanf(_fov / 2.0f);
	mat.xx = 1.0f / (aspect * tanHalfFov);
	mat.yy = 1.0f / tanHalfFov;
	mat.zw = -1.0f;

	mat.zz = _far / (_near - _far);
	mat.wz = -(_far * _near) / (_far - _near);

	return mat;
}
Math3D::Mat4 Math3D::GetLookAt(Vec3 _origin, Vec3 _target, Vec3 _up)
{
	Math3D::Mat4 mat;

	const Vec3 front(Math3D::Normalize(_target - _origin));
	const Vec3 side(Math3D::Normalize(Cross(front, _up)));
	const Vec3 up(Cross(side, front));

	mat.xx = side.x;
	mat.yx = side.y;
	mat.zx = side.z;
	mat.xy = up.x;
	mat.yy = up.y;
	mat.zy = up.z;
	mat.xz = -front.x;
	mat.yz = -front.y;
	mat.zz = -front.z;
	mat.wx = -Dot(side, _origin);
	mat.wy = -Dot(up, _origin);
	mat.wz = Dot(front, _origin);
	mat.ww = 1.0f;

	return mat;
}

// Extract
Math3D::Vec3 Math3D::ExtractPosition(Mat4 _matrix)
{
	return Vec3(_matrix.xw, _matrix.yw, _matrix.zw);
}
Math3D::Vec3 Math3D::ExtractFront(Mat4 _matrix)
{
	return Vec3(_matrix.xz, _matrix.yz, _matrix.zz);
}
Math3D::Vec3 Math3D::ExtractRight(Mat4 _matrix)
{
	return Vec3(_matrix.xx, _matrix.yx, _matrix.zx);
}
Math3D::Vec3 Math3D::ExtractUp(Mat4 _matrix)
{
	return Vec3(_matrix.xy, _matrix.yy, _matrix.zy);
}
Math3D::Vec3 Math3D::ExtractScale(Mat4 _matrix)
{
	float lenghtX = Lenght({ _matrix.xx, _matrix.yx, _matrix.zx });
	float lenghtY = Lenght({ _matrix.xy, _matrix.yy, _matrix.zy });
	float lenghtZ = Lenght({ _matrix.xz, _matrix.yz, _matrix.zz });

	return Vec3(lenghtX, lenghtY, lenghtZ);
}
