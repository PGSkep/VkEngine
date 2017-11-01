#include "GameplayBreakout.h"

#include "../_Common/Input.h"
#include "../_Common/Collision.h"

GameplayBreakout::GameplayBreakout()
{
	// ball
	balls.PushBack({ { 0.0f, 0.4f }, 0.025f, { 0.0f, -0.1f } });

	// paddle
	paddles.PushBack({ {0.0f, 0.1f}, {0.1f, 0.025f} });

	// blocks
	for (size_t iY = 0; iY != 4; ++iY)
	{
		for (size_t iX = 0; iX != 9; ++iX)
		{
			blocks.PushBack({ { -0.85f + iX*0.21f, -0.46f + iY*0.12f }, { 0.09f, 0.045f }, 1 });
		}
	}
}
GameplayBreakout::~GameplayBreakout()
{
	blocks.Clear();
	paddles.Clear();
	balls.Clear();
}

void GameplayBreakout::Input(RECT _clientScreenSpace)
{
	int32_t mouseXScreenSpace = Input::GetMousePositionX();
	int32_t mouseYScreenSpace = Input::GetMousePositionY();

	float aspect = (float)(_clientScreenSpace.bottom - _clientScreenSpace.top) / (float)(_clientScreenSpace.right - _clientScreenSpace.left);
	float mouseXClientSpace = -1.0f + (mouseXScreenSpace - _clientScreenSpace.left)/(((float)_clientScreenSpace.right - (float)_clientScreenSpace.left)*0.5f);
	float mouseYClientSpace = (1.0f - (mouseYScreenSpace - _clientScreenSpace.bottom) / (((float)_clientScreenSpace.top - (float)_clientScreenSpace.bottom)*0.5f)) *aspect;

	paddles.GetHead().node->data.position.x = mouseXClientSpace;
}
void GameplayBreakout::Update(float _deltaTime)
{
	// move
	balls.GetHead().node->data.position.x += balls.GetHead().node->data.velocity.x * _deltaTime;
	balls.GetHead().node->data.position.y += balls.GetHead().node->data.velocity.y * _deltaTime;

	// collide
	Ball* ball = &balls.GetHead().node->data;
	Paddle pad = paddles.GetHead().node->data;
	
	Collision2D::AABB aabb = {
		pad.position.x - (1.0f * pad.size.x),// left
		pad.position.x + (1.0f * pad.size.x),// right
		pad.position.y - (1.0f * pad.size.y),// top
		pad.position.y + (1.0f * pad.size.y),// bottom
	};
	Collision2D::Circle circle = {
		{
			ball->position.x,// x
			ball->position.y,// y
		},
		ball->radius
	};

	// collision check
	Math3D::Vec2 closestPoint = Collision2D::ClosestPoint(aabb, circle.position);
	if (Collision2D::CollisionTestAABBCircle(aabb, circle))
	{
		Math3D::Vec2 velocity = { ball->velocity.x, ball->velocity.y };

		Math3D::Vec2 normal;
		Math3D::Vec2 delta = circle.position - closestPoint;

		if (delta.IsZero())
		{

		}
		else
		{
			normal = Math3D::Vec2::Normalize(circle.position - closestPoint);

			ball->velocity = Math3D::Vec2::Reflect(velocity, normal);
		}

		COORD coord = { 0, 0 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		std::cout << "                                              \n";
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		std::cout << ball->velocity.x << "	" << ball->velocity.y;
	}
	
	// collision response
}
void GameplayBreakout::Render()
{
	ListS<Ball>::Iterator ballIter = balls.GetHead();
	while (ballIter.node != nullptr)
	{
		//ballIter.node->data.Render();
		ballIter.Next();
	}

	ListS<Paddle>::Iterator paddleIter = paddles.GetHead();
	while (paddleIter.node != nullptr)
	{
		//paddleIter.node->data.Render();
		paddleIter.Next();
	}

	ListS<Block>::Iterator blockIter = blocks.GetHead();
	while (blockIter.node != nullptr)
	{
		//blockIter.node->data.Render();
		blockIter.Next();
	}
}
