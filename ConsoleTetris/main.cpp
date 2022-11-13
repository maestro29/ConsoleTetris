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

const int WALL_START_X = 50;
const int WALL_START_Y = 4;

const int WALL_LENGTH_X = 2;
const int WALL_LENGTH_Y = 1;

const int MAX_BLOCK_LENGTH = 4;
const int BLOCK_TYPE_COUNT = 7;
const int MAX_ROW = 20;
const int MAX_COL = 10;

enum Key { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113, SPACE = 32 };
enum Speed { SPEED1 = 1000, SPEED2 = 500, MAX_SPEED = 10 };
enum Object {EMPTY = 0, WALL = -1};
enum Color {BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, GOLD = 6, DEFAULT = 7, SKYBLUE = 11, YELLOW = 14, WHITE = 15};

ULONGLONG prev_time;

bool gameIsRunning;

int map[MAX_ROW+1][MAX_COL] = { EMPTY };

int block_T[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
int block_I[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0} };
int block_O[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_J[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0} };
int block_L[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0} };
int block_S[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_Z[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0} };

int autoDropSpeed;

int currentBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };
int currentBlockLength;

void gotoxy(short x, short y);
void gotoxyInside(short x, short y);

void createNewBlock();
void drawBlock();
void eraseBlock();
bool moveBlock(int r, int c);
void copyBlock(int* src, int* dest);

void setTextColor(int color);
void setBlockColor(int color);

void moveDown();
void moveLeft();
void moveRight();

bool isMovable(Pos pos);
bool isRotatable();
bool isLineFull(int row);

void rotateBlock();
bool putBlock();

void autoDrop();
void redrawMap();

void hideCursor();
void initGame();
void gameOver();
void drawBackground();
void keyProcess();
void run();

int main()
{
	run();
}


void gotoxy(short x, short y) {
	x += WALL_START_X;
	y += WALL_START_Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}
void gotoxyInside(short x, short y) {
	gotoxy(x + WALL_LENGTH_X, y + WALL_LENGTH_Y);
}

void createNewBlock()
{
	int v = rand() % BLOCK_TYPE_COUNT;
	switch (v)
	{
	case 0:  copyBlock(*block_T, *currentBlock); currentBlockLength = 3; setBlockColor(MAGENTA);  break;
	case 1:  copyBlock(*block_I, *currentBlock); currentBlockLength = 4; setBlockColor(CYAN);     break;
	case 2:  copyBlock(*block_O, *currentBlock); currentBlockLength = 2; setBlockColor(YELLOW);   break;
	case 3:  copyBlock(*block_J, *currentBlock); currentBlockLength = 3; setBlockColor(BLUE);     break;
	case 4:  copyBlock(*block_L, *currentBlock); currentBlockLength = 3; setBlockColor(GOLD);     break;
	case 5:  copyBlock(*block_S, *currentBlock); currentBlockLength = 3; setBlockColor(GREEN);    break;
	default: copyBlock(*block_Z, *currentBlock); currentBlockLength = 3; setBlockColor(RED);      break;
	}
	currentPos = Pos(0, MAX_COL / 2 - 1);
	drawBlock();
}

void drawBlock() {
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{
				gotoxyInside((currentPos.col + j) * 2, currentPos.row + i);
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
			if (currentBlock[i][j] != EMPTY)
			{
				gotoxyInside((currentPos.col + j) * 2, currentPos.row + i);
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

void setTextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void setBlockColor(int color)
{
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			currentBlock[i][j] = currentBlock[i][j] * color;
		}
	}
	setTextColor(color);
}

void moveDown()
{
	prev_time = GetTickCount64(); // 블럭 하강 시, 프레임 카운트 무조건 초기화

	bool blockReachedTheEnd = !moveBlock(1, 0); // 블럭을 한 칸 하강한다.

	// 블럭이 끝까지 내려왔을 경우
	if (blockReachedTheEnd)
	{
		bool gameIsOver = !putBlock(); // 블럭을 맵에 놓는다.
		if (gameIsOver)
			return;

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

		autoDropSpeed = SPEED2;
		createNewBlock();
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
			if (currentBlock[i][j] != EMPTY &&
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

bool putBlock()
{
	for (int i = 0; i < currentBlockLength; i++) {
		for (int j = 0; j < currentBlockLength; j++) {
			if (currentBlock[i][j] != EMPTY) {
				if (map[currentPos.row + i][currentPos.col + j] != EMPTY) {
					// 놓을 자리에 다른 블럭이 있다면 게임 오버
					gameOver();
					return false;
				}
				else
				{
					map[currentPos.row + i][currentPos.col + j] = currentBlock[i][j];
				}
			}
		}
	}
	return true;
}

void autoDrop() {

	ULONGLONG cur_time = GetTickCount64();

	if (cur_time - prev_time > autoDropSpeed)
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
			gotoxyInside(j * 2, i);
			setTextColor(map[i][j]);

			if (map[i][j] == EMPTY)
			{
				printf("  ");
			}
			else
			{
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
	autoDropSpeed = SPEED2;

	drawBackground();
	hideCursor();
	createNewBlock();
	gameIsRunning = true;

	for (int i = 0; i < MAX_COL; i++)
		map[MAX_ROW][i] = WALL;
}

void gameOver()
{
	gameIsRunning = false;
	gotoxy(MAX_COL - 3, MAX_ROW / 2);
	setTextColor(WHITE);
	printf(" Game Over"); // G가 지워지는 버그 때문에 앞에 한 칸 공백 추가
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
}

void drawBackground()
{
	setTextColor(WHITE);

	// 벽 그리기 (가로)
	for (int i = 0; i < MAX_COL + 2; i++)
	{
		gotoxy((short)(i * 2), 0);
		printf("▨");
		gotoxy((short)(i * 2), MAX_ROW + 1);
		printf("▨");
	}

	// 벽 그리기 (세로)
	for (int i = 0; i < MAX_ROW + 1; i++)
	{
		gotoxy(0, (short)i);
		printf("▨\n");
		gotoxy((MAX_COL * 2) + 2, (short)i);
		printf("▨\n");
	}

	// 현황판 벽 그리기 (가로)
	for (int i = 0; i < 9; i++)
	{
		gotoxy((short)(i * 2 - 18), 0);
		printf("▧");
		gotoxy((short)(i * 2 - 18), 7);
		printf("▧");
		gotoxy((short)(i * 2 - 18), 13);
		printf("▧");
		gotoxy((short)(i * 2 - 18), 21);
		printf("▧");
	}

	// 현황판 벽 그리기 (세로)
	for (int i = 0; i < 21; i++)
	{
		gotoxy(-18, (short)i);
		printf("▧");
		gotoxy(-2, (short)i);
		printf("▧");
	}

	setTextColor(SKYBLUE);
	gotoxy(-15, 15);
	printf("← : 왼쪽");
	gotoxy(-15, 16);
	printf("→ : 오른쪽");
	gotoxy(-15, 17);
	printf("↑ : 회전");
	gotoxy(-15, 18);
	printf("↓ : 밑으로");
	gotoxy(-15, 19);
	printf("SPACE : 낙하");
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
			autoDropSpeed = MAX_SPEED;
			break;
		case Q:
			gameOver();
			break;
		}
	}
}

void run()
{
	initGame();

	while (gameIsRunning)
	{
		keyProcess();
		autoDrop();
	}
}