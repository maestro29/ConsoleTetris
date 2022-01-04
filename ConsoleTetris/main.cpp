#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <algorithm>
#include <time.h>

struct Pos
{
	int row, col;
	Pos()
	{
		row = col = 0;
	};
	Pos(int row_, int col_)
	{
		row = row_;
		col = col_;
	}
} currentPos;

const int MAX_BLOCK_LENGTH = 4;
const int BLOCK_TYPE_COUNT = 7;
const int MAX_ROW = 20;
const int MAX_COL = 10;

enum Key { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113, SPACE = 32 };
enum Speed { SPEED1 = 1000, SPEED10 = 10 };
enum Object {EMPTY = 0, WALL = -1};

ULONGLONG prev_time;

int map[MAX_ROW+1][MAX_COL] = { EMPTY };

int block_T[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
int block_I[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0} };
int block_O[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_J[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0} };
int block_L[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0} };
int block_S[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_Z[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0} };

int autoDrop_term = SPEED1;

int currentBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };
int currentBlockLength;


void gotoxy(short x, short y);

void createBlock();
void drawBlock();
void eraseBlock();
bool moveBlock(int r, int c);
void copyBlock(int* src, int* dest);

void moveDown();
void moveLeft();
void moveRight();

bool isMovable(Pos pos);
bool isRotatable();
bool isLineFull(int row);

void rotateBlock();
void putBlock();

void autoDrop(ULONGLONG& prev_time, int term);
void redrawMap();

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
	int v = rand() % BLOCK_TYPE_COUNT;
	switch (v)
	{
	case 0:  copyBlock(*block_T, *currentBlock); currentBlockLength = 3; break;
	case 1:  copyBlock(*block_I, *currentBlock); currentBlockLength = 4; break;
	case 2:  copyBlock(*block_O, *currentBlock); currentBlockLength = 2; break;
	case 3:  copyBlock(*block_J, *currentBlock); currentBlockLength = 3; break;
	case 4:  copyBlock(*block_L, *currentBlock); currentBlockLength = 3; break;
	case 5:  copyBlock(*block_S, *currentBlock); currentBlockLength = 3; break;
	default: copyBlock(*block_Z, *currentBlock); currentBlockLength = 3; break;
	}
}

void drawBlock() {
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] == 1)
			{
				gotoxy((currentPos.col + j) * 2, currentPos.row + i);
				printf("■");
			}
		}
	}
}
void eraseBlock() {
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] == 1)
			{
				gotoxy((currentPos.col + j) * 2, currentPos.row + i);
				printf("  ");
			}
		}
	}
}

bool moveBlock(int r, int c)
{
	Pos newPos = Pos(currentPos.row + r, currentPos.col + c);

	if (isMovable(newPos) == false)
		return false;

	eraseBlock();
	currentPos = newPos;
	drawBlock();
	return true;
}

void copyBlock(int* src, int* dest)
{
	std::copy(src, src + (MAX_BLOCK_LENGTH * MAX_BLOCK_LENGTH), dest);
}

void moveDown()
{
	prev_time = GetTickCount64(); // 블럭 하강 시, 프레임 카운트 무조건 초기화

	// 블럭이 끝까지 내려왔을 경우
	if (moveBlock(1, 0) == false)
	{
		putBlock(); // 블럭을 맵에 투사한다.

		// 가득 채워 진 라인이 있는지 검사
		for (int i = 0; i < MAX_ROW; i++)
		{
			if (isLineFull(i) == true)
			{
				for (int x = i; x > 0; x--)
				{
					for (int y = 0; y < MAX_COL; y++)
					{
						map[x][y] = map[x - 1][y];
					}
				}
				redrawMap();
			}
		}

		autoDrop_term = SPEED1;
		currentPos = Pos(0, 3);
		createBlock();
		drawBlock();
	}
}

void moveLeft()
{
	moveBlock(0, -1);
}

void moveRight()
{
	moveBlock(0, 1);
}

bool isMovable(Pos pos)
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] == 1 &&
				((pos.col + j < 0 || pos.col + j > MAX_COL - 1) ||
					(map[pos.row+i][pos.col+j] != EMPTY)))
				return false;
		}
	}
	return true;
}

bool isRotatable()
{
	if (currentPos.col < 0 || currentPos.col + currentBlockLength - 1 > MAX_COL - 1)
		return false;

	for (int i = 0; i < currentBlockLength; i++) {
		for (int j = 0; j < currentBlockLength; j++) {
			if (map[currentPos.row + i][currentPos.col + j] != EMPTY)
				return false;
		}
	}
	return true;
}

bool isLineFull(int row)
{
	for (int j = 0; j < MAX_COL; j++)
	{
		if (map[row][j] == EMPTY)
			return false;
	}
	return true;
}

void rotateBlock()
{
	if (isRotatable() == false)
		return;

	int tempBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };
	copyBlock(*currentBlock, *tempBlock);

	eraseBlock();
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			currentBlock[i][j] = tempBlock[currentBlockLength-1-j][i];
		}
	}
	drawBlock();
}

void putBlock()
{
	for (int i = 0; i < currentBlockLength; i++) {
		for (int j = 0; j < currentBlockLength; j++) {
			if (currentBlock[i][j] == 1)
				map[currentPos.row + i][currentPos.col + j] = currentBlock[i][j];
		}
	}
}

void autoDrop(ULONGLONG& prev_time, int term) {

	ULONGLONG cur_time = GetTickCount64();

	if (cur_time - prev_time > term)
	{
		moveDown();
		prev_time = cur_time;
	}
}

void redrawMap()
{
	for (int i = 0; i < MAX_ROW; i++)
	{
		for (int j = 0; j < MAX_COL; j++)
		{
			if (map[i][j] == 0)
			{
				gotoxy(j * 2, i);
				printf("  ");
			}
			else if (map[i][j] == 1)
			{
				gotoxy(j * 2, i);
				printf("■");
			}
		}
	}
}


void hideCursor() {
	CONSOLE_CURSOR_INFO cursorInfo;

	cursorInfo.bVisible = 0;
	cursorInfo.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void initGame()
{
	srand((unsigned int)time(0));
	prev_time = GetTickCount64();
	autoDrop_term = SPEED1;

	hideCursor();
	createBlock();
	moveBlock(0, 3);

	for (int i = 0; i < MAX_COL; i++)
		map[MAX_ROW][i] = WALL;
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
		case SPACE:
			autoDrop_term = SPEED10;
			break;
		}
	}
}

void run()
{
	initGame();

	while (1)
	{
		keyProcess();
		autoDrop(prev_time, autoDrop_term);
	}
}