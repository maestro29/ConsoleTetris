#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>


const int WALL_START_X = 50;	// �� ������� ������ X��ǥ
const int WALL_START_Y = 6;		// �� ������� ������ Y��ǥ

const int WALL_LENGTH_X = 2;	// �� ���� ����
const int WALL_LENGTH_Y = 1;	// �� ���� ����

const int MAX_BLOCK_LENGTH = 4;	// �� �ִ� ����
const int BLOCK_TYPE_COUNT = 7;	// �� ���� ����
const int MAX_ROW = 20;			// �� ũ��(�� ����)
const int MAX_COL = 5;			// �� ũ��(�� ����)

enum Key { UP = 72, LEFT = 75, RIGHT = 77, DOWN = 80, P = 112, Q = 113, SPACE = 32,		// Ű����
			W = 119, A = 97, S = 115, D = 100, X = 120};
enum Speed { SPEED1 = 1000, SPEED2 = 500, MAX_SPEED = 10 };								// ���ǵ�
enum Object { EMPTY = 0, WALL = -1 };													// ��ü
enum Color {
	BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, GOLD = 6, DEFAULT = 7,			// ����
	LIGHTGREEN = 10, SKYBLUE = 11, LIGHTRED = 12, YELLOW = 14, WHITE = 15
};

ULONGLONG prev_time;	// ���� ����(������ ����)

bool gameIsRunning;		// ���� ���� ����
bool comboAvailable;	// �޺� ���� ����

int map[MAX_ROW + 1][MAX_COL] = { EMPTY };	// ��

// ������ �� ����
int block_T[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
int block_I[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0} };
int block_O[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_J[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0} };
int block_L[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0} };
int block_S[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
int block_Z[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { {1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0} };

int autoDropSpeed;	// �ڵ� �ϰ� ���ǵ�

int currentBlockPosRow;	// ���� �� ��ġ (��)
int currentBlockPosCol;	// ���� �� ��ġ (��)

int currentBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// ���� ��
int currentBlockLength;	// ���� �� ����
int currentBlockColor;	// ���� �� ����
int nextBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// ���� ��
int nextBlockLength;	// ���� �� ����
int nextBlockColor;		// ���� �� ����

int lineClearAtOnce;	// �� ���� Ŭ���� �� ���� ����
int currentCombo;		// ���� �޺� ����
int maxCombo;			// �ִ� �޺� ����
int totalScore;			// ���� ����

void gotoXY(short x, short y);			// X,Y�� Ŀ�� �̵�
void gotoInsideXY(short x, short y);	// �� ���� X,Y�� Ŀ�� �̵�

void createNextBlock();			// ���� �� ����
void updateCurrentBlock();		// ���� �� ����
void drawBlock();				// �� �׸���
void eraseBlock();				// �� �����
bool moveBlock(int r, int c);	// �� �̵�
void copyBlock(int src[][MAX_BLOCK_LENGTH], int dest[][MAX_BLOCK_LENGTH]);	// �� ����

void setTextColor(int color);		// �۾� ���� ����
void setNextBlockColor(int color);	// ���� �� ���� ����

void moveDown();	// �Ʒ��� �� ĭ �̵�
void moveLeft();	// �������� �� ĭ �̵�
void moveRight();	// ���������� �� ĭ �̵�

bool isMovable(int r, int c);	// r��, c�� ��ŭ �̵� ������ �� �˻�
bool isRotatable();				// ȸ�� ������ �� �˻�
bool isLineFull(int row);		// ������ �� ä�������� �˻�

void rotateBlock();	// �� ȸ��
void putBlock();	// ���� �ʿ� ����

void autoDrop();	// �� �ڵ� �ϰ�
void redrawMap();	// �� ���� �ٽ� �׸���

void hideCursor();		// Ŀ�� �����
void initGame();		// ���� �ʱ�ȭ
void gameOver();		// ���� ����
void drawBackground();	// ���ȭ�� �׸���
void updateScore();		// ���� ����
void keyProcess();		// Ű ó��
void run();				// ���� ����

int main()
{
	run();
}


void gotoXY(short x, short y)
{
	x = x + WALL_START_X; // �� ���� ��ܺ��� x��ŭ ������ ��
	y = y + WALL_START_Y; // �� ���� ��ܺ��� y��ŭ ������ ��
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x,y });	// Ŀ�� �̵�
}

void gotoInsideXY(short x, short y)
{
	gotoXY(x + WALL_LENGTH_X, y + WALL_LENGTH_Y);	// �� ���̸�ŭ �� ���� �̵�
}

void createNextBlock()
{
	if (gameIsRunning == false)
	{ // �̹� ������ �������� �� ���� �������� �ʴ´�.
		return;
	}

	int v = rand() % BLOCK_TYPE_COUNT;	// ������ ���� ����
	switch (v) // ���ں� �� ����
	{
	case 0:  copyBlock(block_T, nextBlock); nextBlockLength = 3; setNextBlockColor(MAGENTA);  break;
	case 1:  copyBlock(block_I, nextBlock); nextBlockLength = 4; setNextBlockColor(CYAN);     break;
	case 2:  copyBlock(block_O, nextBlock); nextBlockLength = 2; setNextBlockColor(YELLOW);   break;
	case 3:  copyBlock(block_J, nextBlock); nextBlockLength = 3; setNextBlockColor(BLUE);     break;
	case 4:  copyBlock(block_L, nextBlock); nextBlockLength = 3; setNextBlockColor(GOLD);     break;
	case 5:  copyBlock(block_S, nextBlock); nextBlockLength = 3; setNextBlockColor(GREEN);    break;
	default: copyBlock(block_Z, nextBlock); nextBlockLength = 3; setNextBlockColor(RED);      break;
	}

	setTextColor(nextBlockColor);	// �۾� ������ ���� �� �������� ����
	// ���� �� �׸���
	for (int i = 0; i < MAX_BLOCK_LENGTH; i++)
	{
		for (int j = 0; j < MAX_BLOCK_LENGTH; j++)
		{
			gotoXY((MAX_COL + 4 + j) * 2, i + 4);	// ���� �� ǥ�� ��ġ�� �̵�
			if (nextBlock[i][j] != EMPTY)
				printf("��");
			else
				printf("  ");
		}
	}
	setTextColor(currentBlockColor);	// �۾� ������ ���� �� �������� ����
}

void updateCurrentBlock()
{
	copyBlock(nextBlock, currentBlock);	// ���� ���� ���� ������ ����
	currentBlockPosRow = 0;				// ���� �� ��ġ�� ��� ���߾����� ����
	currentBlockPosCol = MAX_COL / 2 - 1;
	currentBlockLength = nextBlockLength;	// ���� �� ���̸� ���� ������
	currentBlockColor = nextBlockColor;		// ���� �� ������ ���� �� ��������
	setTextColor(currentBlockColor);	// �۾� ������ ���� �� �������� ����
	drawBlock();	// �� �׸���

	// ������ �ڸ��� �ٸ� ���� �ִ��� Ȯ��
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{
				if (map[currentBlockPosRow + i][currentBlockPosCol + j] != EMPTY)
				{
					gameOver();	// �ٸ� ���� �ִٸ� ���� ����
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
			{	// ���� �� �׸���
				gotoInsideXY((currentBlockPosCol + j) * 2, currentBlockPosRow + i);
				printf("��");
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
			{	// ���� �� �����
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

	if (isMovable(newRow, newCol) == false)	// ���ο� ��ġ�� ���� �̵��� �� ���ٸ� false ����
		return false;

	eraseBlock(); // ���� ��ġ �� �����
	currentBlockPosRow = newRow;	// ��ġ �ű� ��
	currentBlockPosCol = newCol;
	drawBlock();	// �� �׸���
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
		{	// �ʿ� ��ġ�� ������ ������ ������ �������� ����
			nextBlock[i][j] = nextBlock[i][j] * color;
		}
	}
	nextBlockColor = color;
}

void moveDown()
{
	prev_time = GetTickCount64(); // �� �ϰ� ��, ������ ī��Ʈ �ʱ�ȭ
	lineClearAtOnce = 0;	// �� ���� Ŭ���� �� ���� ���� �ʱ�ȭ

	bool blockReachedTheEnd = !moveBlock(1, 0); // ���� �� ĭ �ϰ��Ѵ�.

	// ���� ������ �������� ���
	if (blockReachedTheEnd)
	{
		putBlock(); // ���� �ʿ� ���´�.

		// ���� ä�� �� ������ �ִ��� �˻�
		for (int i = 0; i < MAX_ROW; i++)
		{
			if (isLineFull(i) == true)	// ���� ������ ���� á�ٸ�
			{
				// �� ���κ��� ���� �׿� �ִ� ������ �� �� �Ʒ��� ������.
				for (int x = i; x > 0; x--)
				{
					for (int y = 0; y < MAX_COL; y++)
					{
						map[x][y] = map[x - 1][y];
					}
				}
				lineClearAtOnce++;	// ���� Ŭ���� ���� ����
			}
		}

		if (lineClearAtOnce > 0) // ���� ���� Ŭ��� �ߴٸ�
		{
			if (comboAvailable == true)	// ���� �޺��� ������ ���¶��
			{
				currentCombo++;	// �޺� ���� ����

				// �ִ� �޺� ���� ����
				if (currentCombo > maxCombo)
				{
					maxCombo = currentCombo;
				}
			}
			else
			{	// �޺��� �������� ���� ���¶�� �޺� ���� ���� Ȱ��ȭ
				comboAvailable = true;
			}
		}
		else
		{	// ���� Ŭ��� ���� �ʾҴٸ� ���� �޺� ���� �ʱ�ȭ
			comboAvailable = false;
			currentCombo = 0;
		}

		redrawMap();
		updateScore();

		autoDropSpeed = SPEED1;	// �ϵ���(����)���� ���� �ִ�ġ�� �� �ӵ��� �����Ѵ�.
		updateCurrentBlock();	// ���� ���� ���� ������ ��ü
		createNextBlock();	// ���� �� ����
	}
}

void moveLeft()
{
	moveBlock(0, -1);	// �� ��ġ 1�� ����
}

void moveRight()
{
	moveBlock(0, 1);	// �� ��ġ 1�� ����
}

bool isMovable(int r, int c)
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY &&
				((c + j < 0 || c + j > MAX_COL - 1) ||	// ��, �� ���� ����ų�
					(map[r + i][c + j] != EMPTY)))		// �ٸ� ���� �ִٸ�
			{
				return false;	// �̵� �Ұ�
			}
		}
	}
	return true;
}

bool isRotatable()
{
	if (currentBlockPosCol < 0 || currentBlockPosCol + currentBlockLength > MAX_COL)
	{	// ȸ������ �� ��, �� ���� ����ٸ� ȸ�� �Ұ�
		return false;
	}

	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (map[currentBlockPosRow + i][currentBlockPosCol + j] != EMPTY)
			{	// ȸ������ �� �ٸ� ���� �ִٸ� ȸ�� �Ұ�
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
		{	// ���ο� �� ĭ�� �ִٸ� false ����
			return false;
		}
	}
	return true;
}

void rotateBlock()
{
	if (isRotatable() == false)
	{	// ȸ�� �Ұ����̸� ����������.
		return;
	}

	int tempBlock[MAX_BLOCK_LENGTH][MAX_BLOCK_LENGTH] = { 0 };	// �ӽ� �� ����
	copyBlock(currentBlock, tempBlock);	// ���� ���� �ӽ� ���� ����

	eraseBlock();	// ���� �� �����
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{	// ���� ���� �ð�������� 90�� ȸ���Ѵ�.
			currentBlock[i][j] = tempBlock[currentBlockLength - 1 - j][i];
		}
	}
	drawBlock();	// �ٽ� �׸���
}

void putBlock()
{
	for (int i = 0; i < currentBlockLength; i++)
	{
		for (int j = 0; j < currentBlockLength; j++)
		{
			if (currentBlock[i][j] != EMPTY)
			{	// ���� ��ġ�� ���� ���´�. (������Ų��)
				map[currentBlockPosRow + i][currentBlockPosCol + j] = currentBlock[i][j];
			}
		}
	}
}

void autoDrop() {

	ULONGLONG cur_time = GetTickCount64(); // ���� ����

	if (cur_time - prev_time > autoDropSpeed)
	{	// ���� ������ ���� ������ ���̰�, speed���� Ŀ���� �� �ڵ� �ϰ�
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
				printf("��");
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
	srand((unsigned int)time(0));	// ���� �õ尪 ����
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
	{	// ���� ���� �Ʒ��κ��� ������ ó��
		map[MAX_ROW][i] = WALL;
	}
}

void gameOver()
{
	gameIsRunning = false;
	gotoXY(MAX_COL - 3, MAX_ROW / 2);
	setTextColor(WHITE);
	printf(" Game Over"); // G�� �������� ���� ������ �տ� �� ĭ ���� �߰�
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
}

void drawBackground()
{
	gotoXY(MAX_COL - 4, -2);
	setTextColor(LIGHTRED);
	printf("T E T R I S");

	setTextColor(WHITE);

	// ���θ� �� �׸��� (����)
	for (int i = 0; i < MAX_COL + 2; i++)
	{
		gotoXY(i * 2, 0);
		printf("��");
		gotoXY(i * 2, MAX_ROW + 1);
		printf("��");
	}

	// ���θ� �� �׸��� (����)
	for (int i = 0; i < MAX_ROW + 1; i++)
	{
		gotoXY(0, i);
		printf("��\n");
		gotoXY((MAX_COL * 2) + 2, i);
		printf("��\n");
	}

	// ���� ��Ȳ�� �� �׸��� (����)
	for (int i = 0; i < 9; i++)
	{
		gotoXY(i * 2 - 16, 0);
		printf("��");
		gotoXY(i * 2 - 16, 7);
		printf("��");
		gotoXY(i * 2 - 16, 21);
		printf("��");
	}

	// ���� ��Ȳ�� �� �׸��� (����)
	for (int i = 0; i < 21; i++)
	{
		gotoXY(-16, i);
		printf("��");
		gotoXY(0, i);
		printf("��");
	}

	// ������ ��Ȳ�� �� �׸��� (����)
	for (int i = 0; i < 9; i++)
	{
		gotoXY((i + MAX_COL + 1) * 2, 0);
		printf("��");
		gotoXY((i + MAX_COL + 1) * 2, 9);
		printf("��");
		gotoXY((i + MAX_COL + 1) * 2, 21);
		printf("��");
	}
	// ������ ��Ȳ�� �� �׸��� (����)
	for (int i = 0; i < 21; i++)
	{
		gotoXY((MAX_COL + 1) * 2, i);
		printf("��");
		gotoXY((MAX_COL + 9) * 2, i);
		printf("��");
	}

	setTextColor(SKYBLUE);
	gotoXY((MAX_COL + 4) * 2 + 1, 2);
	printf("NEXT");

	gotoXY(-13, 9);
	printf("�� : ����");
	gotoXY(-13, 11);
	printf("�� : ������");
	gotoXY(-13, 13);
	printf("�� : ȸ��");
	gotoXY(-13, 15);
	printf("�� : ������");
	gotoXY(-13, 17);
	printf("SPACE : ����");
	gotoXY(-13, 19);
	printf("Q : ����");
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
		printf("%d �� Ŭ����", lineClearAtOnce);
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
		printf("%d �޺� !!", currentCombo);
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
		printf("+%d��", score);
	}

	setTextColor(LIGHTGREEN);
	gotoXY(-11, 2);
	printf("STAGE 1");

	totalScore += score;
	setTextColor(YELLOW);
	gotoXY(-11, 4);
	printf("%d ��", totalScore);

	gotoXY((MAX_COL + 3) * 2 - 1, 18);
	switch (maxCombo)
	{
	case 0: setTextColor(WHITE); break;
	case 1: setTextColor(GOLD); break;
	case 2: setTextColor(YELLOW); break;
	case 3: setTextColor(LIGHTRED); break;
	default:setTextColor(RED); break;
	}
	printf("�ִ��޺� : %d", maxCombo);
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
		Sleep(1);	// CPU ������ ����� �����ϱ� ���Ͽ� �߰�
	}
}