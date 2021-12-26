#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <algorithm>


int row = 0;
int col = 0;

enum KEY { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113 };

int map[20][10] = { 0 };

int block_T[3][3] = { {0,1,0},
					 {1,1,1},
					 {0,0,0} };

int currentBlock[3][3] = { 0 };


void gotoxy(short x, short y);
void createBlock();
void drawBlock();
void eraseBlock();
void moveBlock(int drow, int dcol);
bool isMovable(int drow, int dcol);
void rotateBlock();
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

	/*gotoxy(x, y);
	printf("бсбсбс");
	gotoxy(x, y + 1);
	printf("    бс");*/
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

	/*gotoxy(x, y);
	printf("      ");
	gotoxy(x, y + 1);
	printf("      ");*/
}

void moveBlock(int drow, int dcol)
{
	if (isMovable(drow, dcol) == false)
		return;
	eraseBlock();
	row += drow;
	col += dcol;
	drawBlock();
}

bool isMovable(int drow, int dcol)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (currentBlock[i][j] == 1 &&
				(col + j + dcol < 0 || col + j + dcol > 9))
				return false;
		}
	}
	return true;
}

void rotateBlock()
{
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
}

void keyProcess()
{
	if (_kbhit()) {
		int key = _getch();
		switch (key) {
		case DOWN:
			moveBlock(1, 0);
			break;
		case UP:
			rotateBlock();
			break;
		case LEFT:
			moveBlock(0, -1);
			break;
		case RIGHT:
			moveBlock(0, 1);
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