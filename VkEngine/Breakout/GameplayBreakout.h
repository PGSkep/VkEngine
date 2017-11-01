#ifndef	GAMEPLAY_BREAKOUT_H
#define GAMEPLAY_BREAKOUT_H

#include "Block.h"
#include "Paddle.h"
#include "Ball.h"

#include "../_Common/List.h"

class GameplayBreakout
{
public:

	GameplayBreakout();
	~GameplayBreakout();

	ListS<Block> blocks;
	ListS<Paddle> paddles;
	ListS<Ball> balls;

	void Input(RECT _clientScreenSpace);
	void Update(float _deltaTime);
	void Render();
};

#endif