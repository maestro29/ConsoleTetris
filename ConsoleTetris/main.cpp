#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <algorithm>


int row = 0;
int col = 0;

enum KEY { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113 };
enum OBJECT {EMPTY = 0};

int map[20][10] = { EMPTY };

int block_T[3][3] = { {0,1,0},
					 {1,1,1},
					 {0,0,0} };
int block_T_length = 3;

int currentBlock[3][3] = { 0 };


void gotoxy(short x, short y);
void createBlock();
void drawBlock();
void eraseBlock();
void moveBlock(int drow, int dcol);
void moveDown();
void moveLeft();
void moveRight();
bool isMovable(int drow, int dcol);
bool isRotatable();
void rotateBlock();
void putBlock();
void autoDrop(ULONGLONG& prev_time, int term);
void hideCursor();
void initGame();
void keyProcess();
void run();

int main()
{
	run();
}


void gotoxy(short x, short y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}

void createBlock()
{
	std::copy(&block_T[0][0], &block_T[0][0]+9, &currentBlock[0][0]);
}

void drawBlock() {
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (currentBlock[i][j] == 1)
			{
				gotoxy((col + j) * 2, row + i);
				printf("бс");
			}
		}
	}
}
void eraseBlock() {
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (currentBlock[i][j] == 1)
			{
				gotoxy((col + j) * 2, row + i);
				printf("  ");
			}
		}
	}
}

void moveBlock(int drow, int dcol)
{
	if (isMovable(drow, dcol) == false) {
		if (drow == 1 && dcol == 0)
		{
			putBlock();
			row = 0;
			col = 3;
			createBlock();
			drawBlock();
		}
		return;
	}

	eraseBlock();
	row += drow;
	col += dcol;
	drawBlock();
}

void moveDown()
{
	moveBlock(1, 0);
}

void moveLeft()
{
	moveBlock(0, -1);
}

void moveRight()
{
	moveBlock(0, 1);
}

bool isMovable(int drow, int dcol)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (currentBlock[i][j] == 1 &&
				((col + j + dcol < 0 || col + j + dcol > 9) ||
					(map[row+i+drow][col+j+dcol] != EMPTY)))
				return false;
		}
	}
	return true;
}

bool isRotatable()
{
	if (col < 0 || col + block_T_length - 1 > 9)
		return false;

	for (int i = 0; i < block_T_length; i++) {
		for (int j = 0; j < block_T_length; j++) {
			if (map[row + i][col + j] != EMPTY)
				return false;
		}
	}
	return true;
}

void rotateBlock()
{
	if (isRotatable() == false)
		return;

	int tempBlock[3][3] = { 0 };
	std::copy(&currentBlock[0][0], &currentBlock[0][0] + 9, &tempBlock[0][0]);
	eraseBlock();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			currentBlock[i][j] = tempBlock[2-j][i];
		}
	}
	drawBlock();
}

void putBlock()
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (currentBlock[i][j] == 1)
				map[row + i][col + j] = currentBlock[i][j];
		}
	}
}

void autoDrop(ULONGLONG& prev_time, int term) {

	ULONGLONG cur_time = GetTickCount64();

	if (cur_time - prev_time > term)
	{
		moveBlock(1, 0);
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
	createBlock();
	moveBlock(0, 3);
	for (int i = 0; i < 10; i++)
		map[19][i] = 1;
}

void keyProcess()
{
	if (_kbhit()) {
		int key = _getch();
		switch (key) {
		case DOWN:
			moveDown();
			break;
		case UP:
			rotateBlock();
			break;
		case LEFT:
			moveLeft();
			break;
		case RIGHT:
			moveRight();
			break;
		}
	}
}

void run()
{
	ULONGLONG prev_time = GetTickCount64();

	initGame();

	while (1)
	{
		keyProcess();
		autoDrop(prev_time, 1000);
	}
}