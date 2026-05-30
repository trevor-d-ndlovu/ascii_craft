#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#endif

enum class Key : int
{
	W = 0, A, S, D,
	Space, C, F, X, G,
	Num1, Num2, Num3, Num4, Num5, Num6, Num7,
	Left, Right, Up, Down,
	Enter, Backspace,
	COUNT
};

class Input
{
	static bool keys_down[(int)Key::COUNT];
	static bool keys_prev[(int)Key::COUNT];
#ifdef _WIN32
	static SHORT win_key_map[(int)Key::COUNT];
#else
	static termios orig_termios;
	static bool terminal_changed;
	static int linux_key_map[(int)Key::COUNT];
#endif
public:
	static void init();
	static void update();
	static void shutdown();
	static bool is_down(Key key) { return keys_down[(int)key]; }
};

bool Input::keys_down[(int)Key::COUNT] = {};
bool Input::keys_prev[(int)Key::COUNT] = {};

#ifdef _WIN32
SHORT Input::win_key_map[(int)Key::COUNT] =
{
	'W', 'A', 'S', 'D',
	VK_SPACE, 'C', 'F', 'X', 'G',
	'1', '2', '3', '4', '5', '6', '7',
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
	VK_RETURN, VK_BACK
};

void Input::init() {}
void Input::update()
{
	for (int i = 0; i < (int)Key::COUNT; i++)
		keys_down[i] = (GetAsyncKeyState(win_key_map[i]) & 0x8000) != 0;
}
void Input::shutdown() {}

#else
termios Input::orig_termios = {};
bool Input::terminal_changed = false;

int Input::linux_key_map[(int)Key::COUNT] =
{
	'w', 'a', 's', 'd',
	' ', 'c', 'f', 'x', 'g',
	'1', '2', '3', '4', '5', '6', '7',
	0, 0, 0, 0,
	'\n', 127
};

void Input::init()
{
	termios raw;
	tcgetattr(STDIN_FILENO, &orig_termios);
	raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	terminal_changed = true;
}

void Input::update()
{
	std::memset(keys_down, 0, sizeof(keys_down));

	struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
	while (poll(&pfd, 1, 0) > 0)
	{
		char buf[8];
		ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
		if (n <= 0) break;

		if (n == 1)
		{
			char c = buf[0];
			for (int i = 0; i < (int)Key::COUNT; i++)
			{
				if (linux_key_map[i] == c)
					keys_down[i] = true;
			}
		}
		else if (n >= 3 && buf[0] == '\x1b' && buf[1] == '[')
		{
			switch (buf[2])
			{
			case 'A': keys_down[(int)Key::Up] = true; break;
			case 'B': keys_down[(int)Key::Down] = true; break;
			case 'C': keys_down[(int)Key::Right] = true; break;
			case 'D': keys_down[(int)Key::Left] = true; break;
			}
		}
	}
}

void Input::shutdown()
{
	if (terminal_changed)
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
#endif
