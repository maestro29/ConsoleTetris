#include <stdio.h>
#include <Windows.h>
#include <conio.h>

short x = 0;
short y = 0;

enum KEY { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113 };

void gotoxy(short x, short y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}

void drawSquare(short x, short y) {
	gotoxy(x, y);
	printf("бсбсбс");
	gotoxy(x, y+1);
	printf("    бс");
}
void eraseSquare(short x, short y) {
	gotoxy(x, y);
	printf("      ");
	gotoxy(x, y + 1);
	printf("      ");
}

void move(int dx, int dy) {
	eraseSquare(x, y);
	x += dx;
	y += dy;
	drawSquare(x, y);
}

void autoDrop(ULONGLONG& prev_time, int term) {
	
	ULONGLONG cur_time = GetTickCount64();

	if (cur_time - prev_time > term)
	{
		move(0, 1);
		prev_time = cur_time;
	}
}

void hideCursor() {
	CONSOLE_CURSOR_INFO cursorInfo;

	cursorInfo.bVisible = 0;
	cursorInfo.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void initGame() {

	hideCursor();

	x = 10;
	y = 3;
	move(x, y);
}

void keyProcess()
{
	if (_kbhit()) {
		int key = _getch();
		switch (key) {
		case DOWN:
			move(0, 1);
			break;
		case UP:
			move(0, -1);
			break;
		case LEFT:
			move(-2, 0);
			break;
		case RIGHT:
			move(2, 0);
			break;
		}
	}
}

void run()
{
	ULONGLONG prev_time = 0;

	initGame();

	while (1)
	{
		keyProcess();

		autoDrop(prev_time, 1000);
	}
}


int main()
{
	run();
}


