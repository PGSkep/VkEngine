#ifndef	COLLISION_H
#define COLLISION_H

#include "Math3D.h"

namespace Collision2D
{
	struct Line
	{
		Math3D::Vec2 start;
		Math3D::Vec2 end;
	};

	struct AABB
	{
		float left, right, top, bottom;
	};

	struct Circle
	{
		Math3D::Vec2 position;
		float radius;
	};

	bool Intersection(AABB _aabb1, AABB _aabb2)
	{
		if (_aabb1.right < _aabb2.left) return false;
		if (_aabb1.left > _aabb2.right) return false;
		if (_aabb1.bottom < _aabb2.top) return false;
		if (_aabb1.top > _aabb2.bottom) return false;

		return true;
	}
	bool Intersection(AABB _aabb, Math3D::Vec2 _point)
	{
		return false;
	}
	bool Intersection(AABB _aabb, Circle _circle)
	{
		return false;
	}

	static Math3D::Vec2 ClosestPoint(AABB _aabb, Math3D::Vec2 _point)
	{
		{
			Math3D::Vec2 closestPoint = _point;

			// ensures point is inside
			if (closestPoint.x < _aabb.left)
				closestPoint.x = _aabb.left;
			if (closestPoint.x > _aabb.right)
				closestPoint.x = _aabb.right;

			if (closestPoint.y < _aabb.top)
				closestPoint.y = _aabb.top;
			if (closestPoint.y > _aabb.bottom)
				closestPoint.y = _aabb.bottom;

			return closestPoint;
		}
	}
	static bool CollisionTestPointCircle(Math3D::Vec2 _point, Circle _circle)
	{
		float distance = Math3D::Vec2::Distance(_circle.position, _point);

		if (_circle.radius >= distance)
			return true;

		return false;
	}
	static bool CollisionTestAABBCircle(AABB _aabb, Circle _circle)
	{
		Math3D::Vec2 closestPoint = Collision2D::ClosestPoint(_aabb, _circle.position);

		return CollisionTestPointCircle(closestPoint, _circle);
	}
}

#endif