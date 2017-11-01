#ifndef	BLOCK_H
#define BLOCK_H

#include "GameObject.h"
#include "../_Common/Vulkan/vku.h"
#include <stdint.h>

class Block : public GameObject
{
public:
	Math3D::Vec2 position;
	Math3D::Vec2 size;

	uint32_t health;

	Block()
	{
		position = { 0.0f, 0.0f };
			size = { 0.0f, 0.0f };
		health = 0;
	}
	Block(Math3D::Vec2 _position, Math3D::Vec2 _size, uint32_t _health)
	{
		position = _position;
		size = _size;
		health = _health;
	}
};

#endif