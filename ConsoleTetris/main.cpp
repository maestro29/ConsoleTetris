#include <stdio.h>
#include <Windows.h>
#include <conio.h>
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
const int WALL_START_Y = 6;

const int WALL_LENGTH_X = 2;
const int WALL_LENGTH_Y = 1;

const int MAX_BLOCK_LENGTH = 4;
const int BLOCK_TYPE_COUNT = 7;
const int MAX_ROW = 20;
const int MAX_COL = 10;

enum Key { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113, SPACE = 32 };
enum Speed { SPEED1 = 1000, SPEED2 = 500, MAX_SPEED = 10 };
enum Object {EMPTY = 0, WALL = -1};
enum Color {BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, GOLD = 6, DEFAULT = 7,
			LIGHTGREEN = 10, SKYBLUE = 11, LIGHTRED = 12, YELLOW = 14, WHITE = 15};

ULONGLONG prev_time;

bool gameIsRunning;
bool comboAvailable;

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
int currentBlockColor;
int nextBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };
int nextBlockLength;
int nextBlockColor;

int lineClearAtOnce;
int currentCombo;
int maxCombo;
int totalScore;

void gotoXY(short x, short y);
void gotoInsideXY(short x, short y);

void createNextBlock();
void updateCurrentBlock();
void drawBlock();
void eraseBlock();
bool moveBlock(int r, int c);
void copyBlock(int src[][MAX_BLOCK_LENGTH], int dest[][MAX_BLOCK_LENGTH]);

void setTextColor(int color);
void setNextBlockColor(int color);

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
void updateScore();
void keyProcess();
void run();

int main()
{
	run();
}


void gotoXY(short x, short y) {
	x += WALL_START_X;
	y += WALL_START_Y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}
void gotoInsideXY(short x, short y) {
	gotoXY(x + WALL_LENGTH_X, y + WALL_LENGTH_Y);
}

void createNextBlock()
{
	int v = rand() % BLOCK_TYPE_COUNT;
	switch (v)
	{
	case 0:  copyBlock(block_T, nextBlock); nextBlockLength = 3; setNextBlockColor(MAGENTA);  break;
	case 1:  copyBlock(block_I, nextBlock); nextBlockLength = 4; setNextBlockColor(CYAN);     break;
	case 2:  copyBlock(block_O, nextBlock); nextBlockLength = 2; setNextBlockColor(YELLOW);   break;
	case 3:  copyBlock(block_J, nextBlock); nextBlockLength = 3; setNextBlockColor(BLUE);     break;
	case 4:  copyBlock(block_L, nextBlock); nextBlockLength = 3; setNextBlockColor(GOLD);     break;
	case 5:  copyBlock(block_S, nextBlock); nextBlockLength = 3; setNextBlockColor(GREEN);    break;
	default: copyBlock(block_Z, nextBlock); nextBlockLength = 3; setNextBlockColor(RED);      break;
	}

	setTextColor(nextBlockColor);
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			gotoXY((MAX_COL + 4 + j) * 2, i + 4);
			if (nextBlock[i][j] != EMPTY)
				printf("■");
			else
				printf("  ");
		}
	}
	setTextColor(currentBlockColor);
}

void updateCurrentBlock()
{
	copyBlock(nextBlock, currentBlock);
	currentBlockLength = nextBlockLength;
	currentBlockColor = nextBlockColor;
	setTextColor(currentBlockColor);
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
				gotoInsideXY((currentPos.col + j) * 2, currentPos.row + i);
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
				gotoInsideXY((currentPos.col + j) * 2, currentPos.row + i);
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

void copyBlock(int src[][MAX_BLOCK_LENGTH], int dest[][MAX_BLOCK_LENGTH])
{
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			dest[i][j] = src[i][j];
		}
	}
}

void setTextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void setNextBlockColor(int color)
{
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			nextBlock[i][j] = nextBlock[i][j] * color;
		}
	}
	nextBlockColor = color;
}

void moveDown()
{
	prev_time = GetTickCount64(); // 블럭 하강 시, 프레임 카운트 무조건 초기화
	lineClearAtOnce = 0;

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
				lineClearAtOnce++;
			}
		}

		if (lineClearAtOnce > 0)
		{
			if (comboAvailable == true)
			{
				currentCombo++;
				if (currentCombo > maxCombo)
					maxCombo = currentCombo;
			}
			else
			{
				comboAvailable = true;
			}
		}
		else
		{
			comboAvailable = false;
			currentCombo = 0;
		}
		redrawMap();
		updateScore();

		autoDropSpeed = SPEED1;
		updateCurrentBlock();
		createNextBlock();
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
	copyBlock(currentBlock, tempBlock);

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
			gotoInsideXY(j * 2, i);
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
	autoDropSpeed = SPEED1;

	lineClearAtOnce = 0;
	currentCombo = 0;
	maxCombo = 0;
	totalScore = 0;
	comboAvailable = false;

	drawBackground();
	updateScore();
	hideCursor();
	createNextBlock();
	updateCurrentBlock();
	createNextBlock();
	gameIsRunning = true;

	for (int i = 0; i < MAX_COL; i++)
		map[MAX_ROW][i] = WALL;
}

void gameOver()
{
	gameIsRunning = false;
	gotoXY(MAX_COL - 3, MAX_ROW / 2);
	setTextColor(WHITE);
	printf(" Game Over"); // G가 지워지는 버그 때문에 앞에 한 칸 공백 추가
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
}

void drawBackground()
{
	setTextColor(LIGHTRED);
	gotoXY(MAX_COL - 4, -2);
	printf("T E T R I S");

	setTextColor(WHITE);

	// 벽 그리기 (가로)
	for (int i = 0; i < MAX_COL + 2; i++)
	{
		gotoXY((short)(i * 2), 0);
		printf("▨");
		gotoXY((short)(i * 2), MAX_ROW + 1);
		printf("▨");
	}

	// 벽 그리기 (세로)
	for (int i = 0; i < MAX_ROW + 1; i++)
	{
		gotoXY(0, (short)i);
		printf("▨\n");
		gotoXY((MAX_COL * 2) + 2, (short)i);
		printf("▨\n");
	}

	// 왼쪽 현황판 벽 그리기 (가로)
	for (int i = 0; i < 9; i++)
	{
		gotoXY((short)(i * 2 - 16), 0);
		printf("▧");
		gotoXY((short)(i * 2 - 16), 7);
		printf("▧");
		gotoXY((short)(i * 2 - 16), 21);
		printf("▧");
	}

	// 왼쪽 현황판 벽 그리기 (세로)
	for (int i = 0; i < 21; i++)
	{
		gotoXY(-16, (short)i);
		printf("▧");
		gotoXY(0, (short)i);
		printf("▧");
	}

	// 오른쪽 현황판 벽 그리기 (가로)
	for (int i = 0; i < 9; i++)
	{
		gotoXY((short)((i + MAX_COL + 1) * 2), 0);
		printf("▧");
		gotoXY((short)((i + MAX_COL + 1) * 2), 9);
		printf("▧");
		gotoXY((short)((i + MAX_COL + 1) * 2), 21);
		printf("▧");
	}
	// 오른쪽 현황판 벽 그리기 (세로)
	for (int i = 0; i < 21; i++)
	{
		gotoXY((MAX_COL + 1) * 2, (short)i);
		printf("▧");
		gotoXY((MAX_COL + 9) * 2, (short)i);
		printf("▧");
	}

	setTextColor(SKYBLUE);
	gotoXY((MAX_COL + 4) * 2 + 1, 2);
	printf("NEXT");
	
	gotoXY(-13, 9);
	printf("← : 왼쪽");
	gotoXY(-13, 11);
	printf("→ : 오른쪽");
	gotoXY(-13, 13);
	printf("↑ : 회전");
	gotoXY(-13, 15);
	printf("↓ : 밑으로");
	gotoXY(-13, 17);
	printf("SPACE : 낙하");
	gotoXY(-13, 19);
	printf("Q : 종료");
}

void updateScore()
{
	int score = 0;

	gotoXY((MAX_COL + 3) * 2 - 1, 13);
	if (lineClearAtOnce > 0)
	{
		switch (lineClearAtOnce) {
		case 1: setTextColor(CYAN);		score = 10; break;
		case 2: setTextColor(SKYBLUE);	score = 30; break;
		case 3: setTextColor(GREEN);	score = 60; break;
		default: setTextColor(RED);		score = 100; break;
		}
		printf("%d 줄 클리어", lineClearAtOnce);
	}
	else
	{
		printf("            ");
	}

	gotoXY((MAX_COL + 3) * 2, 15);
	if (currentCombo > 0)
	{
		switch (currentCombo) {
		case 1: setTextColor(GOLD);		score *= 2; break;
		case 2: setTextColor(YELLOW);	score *= 3; break;
		case 3: setTextColor(LIGHTRED); score *= 4; break;
		default: setTextColor(RED);		score *= 5; break;
		}
		printf("%d 콤보 !!", currentCombo);
	}
	else
	{
		printf("          ");
	}

	setTextColor(LIGHTGREEN);
	gotoXY((MAX_COL + 4) * 2, 11);
	printf("          ");
	if (score > 0)
	{
		gotoXY((MAX_COL + 4) * 2, 11);
		printf("+%d점", score);
	}

	setTextColor(LIGHTGREEN);
	gotoXY(-11, 2);
	printf("Stage 1");
	
	totalScore += score;
	setTextColor(YELLOW);
	gotoXY(-11, 4);
	printf("%d 점", totalScore);

	setTextColor(YELLOW);
	gotoXY((MAX_COL + 3) * 2 - 1, 18);
	printf("최대콤보 : %d", maxCombo);
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
		Sleep(1);
	}
}