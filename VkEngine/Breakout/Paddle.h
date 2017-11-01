#ifndef	PADDLE_H
#define PADDLE_H

#include "GameObject.h"
#include "../_Common/Vulkan/vku.h"

class Paddle : public GameObject
{
public:
	Math3D::Vec2 position;
	Math3D::Vec2 size;

	Paddle()
	{
		position = { 0.0f, 0.0f };
		size = { 0.0f, 0.0f };
	}
	Paddle(Math3D::Vec2 _position, Math3D::Vec2 _size)
	{
		position = _position;
		size = _size;
	}
};

#endif