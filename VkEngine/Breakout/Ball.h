#ifndef	BALL_H
#define BALL_H

#include "GameObject.h"
#include "../_Common/Vulkan/vku.h"

class Ball : public GameObject
{
public:
	Math3D::Vec2 position;
	float radius;

	Math3D::Vec2 velocity;

	Ball()
	{
		position = { 0.0f, 0.0f };
		radius = 0.0f;
		velocity = { 0.0f, 0.0f };
	}
	Ball(Math3D::Vec2 _position, float _radius, Math3D::Vec2 _velocity)
	{
		position = _position;
		radius = _radius;
		velocity = _velocity;
	}
};

#endif