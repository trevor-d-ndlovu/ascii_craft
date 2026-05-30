#include <iostream>
#include "Game.h"
#include <thread>



int main()
{
	//window initialization
	Terminal3D::init(454,325,float(4)/2.1, 'Q');
	std::cout << "\033[1;37m";//white
	//Terminal3D::init(230, 230, float(4) / 2.5, 'Q');
	//Terminal3D::init(200, 160, float(4) / 2.1, 'Q');

	Game game(12);

	std::thread generation_thread([&] { game.generate_chunks(); });
	generation_thread.detach();

	std::thread physics_thread([&] { game.run_physics(); });
	physics_thread.detach();

	game.render_game();

	Terminal3D::terminate();
}

