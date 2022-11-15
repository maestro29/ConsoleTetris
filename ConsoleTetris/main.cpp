#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>


const int WALL_START_X = 50;	// 벽 좌측상단 시작점 X좌표
const int WALL_START_Y = 6;		// 벽 좌측상단 시작점 Y좌표

const int WALL_LENGTH_X = 2;	// 벽 가로 길이
const int WALL_LENGTH_Y = 1;	// 벽 세로 길이

const int MAX_BLOCK_LENGTH = 4;	// 블럭 최대 길이
const int BLOCK_TYPE_COUNT = 7;	// 블럭 종류 개수
const int MAX_ROW = 20;			// 맵 크기(행 개수)
const int MAX_COL = 5;			// 맵 크기(열 개수)

enum Key { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113, SPACE = 32,		// 키보드
			W = 119, A = 97, S = 115, D = 100, X = 120};
enum Speed { SPEED1 = 1000, SPEED2 = 500, MAX_SPEED = 10 };								// 스피드
enum Object { EMPTY = 0, WALL = -1 };													// 물체
enum Color {
	BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, GOLD = 6, DEFAULT = 7,			// 색상
	LIGHTGREEN = 10, SKYBLUE = 11, LIGHTRED = 12, YELLOW = 14, WHITE = 15
};

ULONGLONG prev_time;	// 과거 시점(프레임 계산용)

bool gameIsRunning;		// 게임 구동 여부
bool comboAvailable;	// 콤보 가능 여부

int map[MAX_ROW + 1][MAX_COL] = { EMPTY };	// 맵

// 종류별 블럭 정의
int block_T[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
int block_I[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0} };
int block_O[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_J[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0} };
int block_L[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0} };
int block_S[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_Z[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0} };

int autoDropSpeed;	// 자동 하강 스피드

int currentBlockPosRow;	// 현재 블럭 위치 (행)
int currentBlockPosCol;	// 현재 블럭 위치 (열)

int currentBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// 현재 블럭
int currentBlockLength;	// 현재 블럭 길이
int currentBlockColor;	// 현재 블럭 색상
int nextBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// 다음 블럭
int nextBlockLength;	// 다음 블럭 길이
int nextBlockColor;		// 다음 블럭 색상

int lineClearAtOnce;	// 한 번에 클리어 한 라인 개수
int currentCombo;		// 현재 콤보 개수
int maxCombo;			// 최대 콤보 개수
int totalScore;			// 점수 총합

void gotoXY(short x, short y);			// X,Y로 커서 이동
void gotoInsideXY(short x, short y);	// 맵 내부 X,Y로 커서 이동

void createNextBlock();			// 다음 블럭 생성
void updateCurrentBlock();		// 현재 블럭 갱신
void drawBlock();				// 블럭 그리기
void eraseBlock();				// 블럭 지우기
bool moveBlock(int r, int c);	// 블럭 이동
void copyBlock(int src[][MAX_BLOCK_LENGTH], int dest[][MAX_BLOCK_LENGTH]);	// 블럭 복사

void setTextColor(int color);		// 글씨 색상 설정
void setNextBlockColor(int color);	// 다음 블럭 색상 설정

void moveDown();	// 아래로 한 칸 이동
void moveLeft();	// 왼쪽으로 한 칸 이동
void moveRight();	// 오른쪽으로 한 칸 이동

bool isMovable(int r, int c);	// r행, c열 만큼 이동 가능한 지 검사
bool isRotatable();				// 회전 가능한 지 검사
bool isLineFull(int row);		// 라인이 다 채워졌는지 검사

void rotateBlock();	// 블럭 회전
void putBlock();	// 블럭을 맵에 놓기

void autoDrop();	// 블럭 자동 하강
void redrawMap();	// 맵 내부 다시 그리기

void hideCursor();		// 커서 숨기기
void initGame();		// 게임 초기화
void gameOver();		// 게임 종료
void drawBackground();	// 배경화면 그리기
void updateScore();		// 점수 갱신
void keyProcess();		// 키 처리
void run();				// 게임 구동

int main()
{
	run();
}


void gotoXY(short x, short y)
{
	x = x + WALL_START_X; // 맵 좌측 상단부터 x만큼 떨어진 곳
	y = y + WALL_START_Y; // 맵 좌측 상단부터 y만큼 떨어진 곳
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x,y });	// 커서 이동
}

void gotoInsideXY(short x, short y)
{
	gotoXY(x + WALL_LENGTH_X, y + WALL_LENGTH_Y);	// 벽 길이만큼 더 들어가서 이동
}

void createNextBlock()
{
	if (gameIsRunning == false)
	{ // 이미 게임이 끝났으면 새 블럭은 생성하지 않는다.
		return;
	}

	int v = rand() % BLOCK_TYPE_COUNT;	// 무작위 숫자 생성
	switch (v) // 숫자별 블럭 생성
	{
	case 0:  copyBlock(block_T, nextBlock); nextBlockLength = 3; setNextBlockColor(MAGENTA);  break;
	case 1:  copyBlock(block_I, nextBlock); nextBlockLength = 4; setNextBlockColor(CYAN);     break;
	case 2:  copyBlock(block_O, nextBlock); nextBlockLength = 2; setNextBlockColor(YELLOW);   break;
	case 3:  copyBlock(block_J, nextBlock); nextBlockLength = 3; setNextBlockColor(BLUE);     break;
	case 4:  copyBlock(block_L, nextBlock); nextBlockLength = 3; setNextBlockColor(GOLD);     break;
	case 5:  copyBlock(block_S, nextBlock); nextBlockLength = 3; setNextBlockColor(GREEN);    break;
	default: copyBlock(block_Z, nextBlock); nextBlockLength = 3; setNextBlockColor(RED);      break;
	}

	setTextColor(nextBlockColor);	// 글씨 색상을 다음 블럭 색상으로 설정
	// 다음 블럭 그리기
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			gotoXY((MAX_COL + 4 + j) * 2, i + 4);	// 다음 블럭 표시 위치로 이동
			if (nextBlock[i][j] != EMPTY)
				printf("■");
			else
				printf("  ");
		}
	}
	setTextColor(currentBlockColor);	// 글씨 색상을 현재 블럭 색상으로 설정
}

void updateCurrentBlock()
{
	copyBlock(nextBlock, currentBlock);	// 다음 블럭을 현재 블럭으로 복사
	currentBlockPosRow = 0;				// 현재 블럭 위치를 상단 정중앙으로 설정
	currentBlockPosCol = MAX_COL / 2 - 1;
	currentBlockLength = nextBlockLength;	// 다음 블럭 길이를 현재 블럭으로
	currentBlockColor = nextBlockColor;		// 다음 블럭 색상을 현재 블럭 색상으로
	setTextColor(currentBlockColor);	// 글씨 색상을 현재 블럭 색상으로 설정
	drawBlock();	// 블럭 그리기

	// 생성된 자리에 다른 블럭이 있는지 확인
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{
				if (map[currentBlockPosRow + i][currentBlockPosCol + j] != EMPTY)
				{
					gameOver();	// 다른 블럭이 있다면 게임 오버
				}
			}
		}
	}
}

void drawBlock()
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{	// 현재 블럭 그리기
				gotoInsideXY((currentBlockPosCol + j) * 2, currentBlockPosRow + i);
				printf("■");
			}
		}
	}
}
void eraseBlock()
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{	// 현재 블럭 지우기
				gotoInsideXY((currentBlockPosCol + j) * 2, currentBlockPosRow + i);
				printf("  ");
			}
		}
	}
}

bool moveBlock(int r, int c)
{
	int newRow = currentBlockPosRow + r;
	int newCol = currentBlockPosCol + c;

	if (isMovable(newRow, newCol) == false)	// 새로운 위치로 블럭을 이동할 수 없다면 false 리턴
		return false;

	eraseBlock(); // 현재 위치 블럭 지우고
	currentBlockPosRow = newRow;	// 위치 옮긴 후
	currentBlockPosCol = newCol;
	drawBlock();	// 블럭 그리기
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
		{	// 맵에 배치된 블럭들이 각자의 색상값을 가지도록 설정
			nextBlock[i][j] = nextBlock[i][j] * color;
		}
	}
	nextBlockColor = color;
}

void moveDown()
{
	prev_time = GetTickCount64(); // 블럭 하강 시, 프레임 카운트 초기화
	lineClearAtOnce = 0;	// 한 번에 클리어 한 라인 개수 초기화

	bool blockReachedTheEnd = !moveBlock(1, 0); // 블럭을 한 칸 하강한다.

	// 블럭이 끝까지 내려왔을 경우
	if (blockReachedTheEnd)
	{
		putBlock(); // 블럭을 맵에 놓는다.

		// 가득 채워 진 라인이 있는지 검사
		for (int i = 0; i < MAX_ROW; i++)
		{
			if (isLineFull(i) == true)	// 만약 라인이 가득 찼다면
			{
				// 그 라인보다 위에 쌓여 있는 블럭들을 한 줄 아래로 내린다.
				for (int x = i; x > 0; x--)
				{
					for (int y = 0; y < MAX_COL; y++)
					{
						map[x][y] = map[x - 1][y];
					}
				}
				lineClearAtOnce++;	// 라인 클리어 개수 증가
			}
		}

		if (lineClearAtOnce > 0) // 만약 라인 클리어를 했다면
		{
			if (comboAvailable == true)	// 만약 콤보가 가능한 상태라면
			{
				currentCombo++;	// 콤보 개수 증가

				// 최대 콤보 개수 갱신
				if (currentCombo > maxCombo)
				{
					maxCombo = currentCombo;
				}
			}
			else
			{	// 콤보가 가능하지 않은 상태라면 콤보 가능 여부 활성화
				comboAvailable = true;
			}
		}
		else
		{	// 라인 클리어를 하지 않았다면 현재 콤보 개수 초기화
			comboAvailable = false;
			currentCombo = 0;
		}

		redrawMap();
		updateScore();

		autoDropSpeed = SPEED1;	// 하드드롭(낙하)으로 인해 최대치가 된 속도를 원복한다.
		updateCurrentBlock();	// 현재 블럭을 다음 블럭으로 교체
		createNextBlock();	// 다음 블럭 생성
	}
}

void moveLeft()
{
	moveBlock(0, -1);	// 블럭 위치 1열 후퇴
}

void moveRight()
{
	moveBlock(0, 1);	// 블럭 위치 1열 증가
}

bool isMovable(int r, int c)
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY &&
				((c + j < 0 || c + j > MAX_COL - 1) ||	// 좌, 우 벽을 벗어나거나
					(map[r + i][c + j] != EMPTY)))		// 다른 블럭이 있다면
			{
				return false;	// 이동 불가
			}
		}
	}
	return true;
}

bool isRotatable()
{
	if (currentBlockPosCol < 0 || currentBlockPosCol + currentBlockLength > MAX_COL)
	{	// 회전했을 때 좌, 우 벽을 벗어난다면 회전 불가
		return false;
	}

	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (map[currentBlockPosRow + i][currentBlockPosCol + j] != EMPTY)
			{	// 회전했을 때 다른 블럭이 있다면 회전 불가
				return false;
			}
		}
	}
	return true;
}

bool isLineFull(int row)
{
	for (int j = 0; j < MAX_COL; j++)
	{
		if (map[row][j] == EMPTY)
		{	// 라인에 빈 칸이 있다면 false 리턴
			return false;
		}
	}
	return true;
}

void rotateBlock()
{
	if (isRotatable() == false)
	{	// 회전 불가능이면 빠져나간다.
		return;
	}

	int tempBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// 임시 블럭 생성
	copyBlock(currentBlock, tempBlock);	// 현재 블럭을 임시 블럭에 복사

	eraseBlock();	// 현재 블럭 지우고
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{	// 현재 블럭을 시계방향으로 90도 회전한다.
			currentBlock[i][j] = tempBlock[currentBlockLength - 1 - j][i];
		}
	}
	drawBlock();	// 다시 그리기
}

void putBlock()
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{	// 현재 위치에 블럭을 놓는다. (고정시킨다)
				map[currentBlockPosRow + i][currentBlockPosCol + j] = currentBlock[i][j];
			}
		}
	}
}

void autoDrop() {

	ULONGLONG cur_time = GetTickCount64(); // 현재 시점

	if (cur_time - prev_time > autoDropSpeed)
	{	// 현재 시점과 과거 시점의 차이가, speed보다 커지면 블럭 자동 하강
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
	srand((unsigned int)time(0));	// 난수 시드값 설정
	prev_time = GetTickCount64();
	autoDropSpeed = SPEED1;

	lineClearAtOnce = 0;
	currentCombo = 0;
	maxCombo = 0;
	totalScore = 0;
	comboAvailable = false;
	gameIsRunning = true;

	drawBackground();
	updateScore();
	hideCursor();
	createNextBlock();
	updateCurrentBlock();
	createNextBlock();

	for (int i = 0; i < MAX_COL; i++)
	{	// 맵의 가장 아랫부분은 벽으로 처리
		map[MAX_ROW][i] = WALL;
	}
}

void gameOver()
{
	gameIsRunning = false;
	gotoXY(MAX_COL - 3, MAX_ROW / 2);
	setTextColor(WHITE);
	printf(" Game Over"); // G가 지워지는 버그 때문에 앞에 한 칸 공백 추가
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
}

void drawBackground()
{
	gotoXY(MAX_COL - 4, -2);
	setTextColor(LIGHTRED);
	printf("T E T R I S");

	setTextColor(WHITE);

	// 메인맵 벽 그리기 (가로)
	for (int i = 0; i < MAX_COL + 2; i++)
	{
		gotoXY(i * 2, 0);
		printf("▨");
		gotoXY(i * 2, MAX_ROW + 1);
		printf("▨");
	}

	// 메인맵 벽 그리기 (세로)
	for (int i = 0; i < MAX_ROW + 1; i++)
	{
		gotoXY(0, i);
		printf("▨\n");
		gotoXY((MAX_COL * 2) + 2, i);
		printf("▨\n");
	}

	// 왼쪽 현황판 벽 그리기 (가로)
	for (int i = 0; i < 9; i++)
	{
		gotoXY(i * 2 - 16, 0);
		printf("▧");
		gotoXY(i * 2 - 16, 7);
		printf("▧");
		gotoXY(i * 2 - 16, 21);
		printf("▧");
	}

	// 왼쪽 현황판 벽 그리기 (세로)
	for (int i = 0; i < 21; i++)
	{
		gotoXY(-16, i);
		printf("▧");
		gotoXY(0, i);
		printf("▧");
	}

	// 오른쪽 현황판 벽 그리기 (가로)
	for (int i = 0; i < 9; i++)
	{
		gotoXY((i + MAX_COL + 1) * 2, 0);
		printf("▧");
		gotoXY((i + MAX_COL + 1) * 2, 9);
		printf("▧");
		gotoXY((i + MAX_COL + 1) * 2, 21);
		printf("▧");
	}
	// 오른쪽 현황판 벽 그리기 (세로)
	for (int i = 0; i < 21; i++)
	{
		gotoXY((MAX_COL + 1) * 2, i);
		printf("▧");
		gotoXY((MAX_COL + 9) * 2, i);
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
		switch (lineClearAtOnce)
		{
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
		switch (currentCombo)
		{
		case 1: setTextColor(GOLD);		score = score * 2; break;
		case 2: setTextColor(YELLOW);	score = score * 3; break;
		case 3: setTextColor(LIGHTRED); score = score * 4; break;
		default: setTextColor(RED);		score = score * 5; break;
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
	printf("STAGE 1");

	totalScore += score;
	setTextColor(YELLOW);
	gotoXY(-11, 4);
	printf("%d 점", totalScore);

	gotoXY((MAX_COL + 3) * 2 - 1, 18);
	switch (maxCombo)
	{
	case 0: setTextColor(WHITE); break;
	case 1: setTextColor(GOLD); break;
	case 2: setTextColor(YELLOW); break;
	case 3: setTextColor(LIGHTRED); break;
	default:setTextColor(RED); break;
	}
	printf("최대콤보 : %d", maxCombo);
}

void keyProcess()
{
	if (_kbhit())
	{
		int key = _getch();
		switch (key)
		{
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
		Sleep(1);	// CPU 점유율 상승을 방지하기 위하여 추가
	}
}