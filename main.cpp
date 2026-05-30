#include <iostream>
#include "Game.h"
#include <thread>
#include <csignal>

Game* global_game = nullptr;

void signal_handler(int)
{
	if (global_game)
	{
		delete global_game;
		global_game = nullptr;
	}
	Terminal3D::terminate();
	exit(0);
}

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	//window initialization
	Terminal3D::init(454,325,float(4)/2.1, 'Q');
	std::cout << "\033[1;37m";

	Game* game = new Game(12);
	global_game = game;

	std::thread generation_thread([&] { game->generate_chunks(); });
	generation_thread.detach();

	std::thread physics_thread([&] { game->run_physics(); });
	physics_thread.detach();

	game->render_game();

	delete game;
	global_game = nullptr;
	Terminal3D::terminate();
}

