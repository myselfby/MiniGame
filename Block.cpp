#include<windows.h>
#include<time.h>
#define BLOCKSIZE 16
#define COLNUM 20
#define ROWNUM 30
LPCWSTR lpTitle = L"Block";                  // 标题栏文本
LPCWSTR lpWindowClass = L"Block";            // 主窗口类名

enum EBlockType
{
	BT_RECT,
	BT_LINE,
	BT_T,
	BT_Z,
	BT_RZ,
	BT_7,
	BT_R7,
	BT_MAX
};
EBlockType currentBlockType = EBlockType::BT_RECT;
POINT blockPosition;
POINT blockPoints[4];
bool blockFlags[ROWNUM][COLNUM];
int Speed = 5;
int gameAreaLeft = BLOCKSIZE;
int gameAreaTop = BLOCKSIZE;
bool bStopGame = true;

void ResetGame();
void StartGame();
void CreateNewBlock();
bool CanRotate();
void ToNextRotator();
bool StepDown();
void RemoveLines();
bool StepLeftRight(int step);
void Render(HDC hdc);

void CALLBACK BlockTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK BlockWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = BlockWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = lpWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINLOGO));
	RegisterClassExW(&wcex);
	HWND hWnd = CreateWindowW(lpWindowClass, lpTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

void ResetGame()
{
	bStopGame = true;
	currentBlockType = EBlockType::BT_RECT;
	blockPosition.x = COLNUM / 2;
	blockPosition.y = 0;

	for (int i = 0; i < 4; i++)
	{
		blockPoints[i].x = i;
		blockPoints[i].y = 0;
	}
	for (int row = 0; row < ROWNUM; row++)
	{
		for (int col = 0; col < COLNUM; col++)
		{
			blockFlags[row][col] = false;
		}
	}
}

void StartGame()
{
	srand(time(0));
	CreateNewBlock();
	bStopGame = false;
}
void CreateNewBlock()
{
	blockPosition.x = COLNUM / 2 - 1;
	blockPosition.y = 0;
	EBlockType type = (EBlockType)(rand() % BT_MAX);
	currentBlockType = type;
	switch (type)
	{
	case BT_RECT:
	{
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 2 ? i - 1 : i - 3;
			blockPoints[i].y = i < 2 ? 0 : 1;
		}
	}
	break;
	case BT_LINE:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i - 2;
			blockPoints[i].y = 0;
		}
		break;
	case BT_T:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 3 ? i - 1 : 0;
			blockPoints[i].y = i < 3 ? -1 : 0;
		}
		break;
	case BT_Z:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 2 ? i - 1 : i - 2;
			blockPoints[i].y = i < 2 ? -1 : 0;
		}
		break;
	case BT_RZ:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 2 ? i : i - 3;
			blockPoints[i].y = i < 2 ? -1 : 0;
		}
		break;
	case BT_7:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 2 ? i-1 : 0;
			blockPoints[i].y = i < 2 ? -1 : i - 2;
		}
		break;
	case BT_R7:
		for (int i = 0; i < 4; i++)
		{
			blockPoints[i].x = i < 2 ? i : 0;
			blockPoints[i].y = i < 2 ? -1 : i - 2;
		}
		break;
	case BT_MAX:
		break;
	default:
		break;
	}
	//blockPoints->x += blockPoints[0].x;
	blockPosition.y -= blockPoints[0].y;
}
bool CanRotate()
{
	if (currentBlockType == EBlockType::BT_RECT)
	{
		return false;
	}
	for (int i = 0; i < 4; i++)
	{
		int tmpx = -blockPoints[i].y + blockPosition.x;
		int tmpy = blockPoints[i].x + blockPosition.y;
		if (blockFlags[tmpy][tmpx]
			|| tmpx >= COLNUM || tmpx < 0
			|| tmpy >= ROWNUM || tmpy < 0)
		{
			return false;
		}
	}
	return true;
}
void ToNextRotator()
{
	if (currentBlockType == EBlockType::BT_RECT)
	{
		return;
	}
	//(x,y)=>(-y,x)
	for (int i = 0; i < 4; i++)
	{
		int tmp = blockPoints[i].x;
		blockPoints[i].x = -blockPoints[i].y;
		blockPoints[i].y = tmp;
	}
}
bool StepDown()
{
	for (int i = 0; i < 4; i++)
	{
		int top = blockPosition.y + blockPoints[i].y;
		int left = blockPosition.x + blockPoints[i].x;
		int nextTop = top + 1;
		if (blockFlags[nextTop][left] || nextTop >= ROWNUM)
		{
			return false;
		}
	}
	blockPosition.y += 1;
	return true;
}
bool StepLeftRight(int step)
{
	for (int i = 0; i < 4; i++)
	{
		int top = blockPosition.y + blockPoints[i].y;
		int left = blockPosition.x + blockPoints[i].x;
		int nextLeft = left + step;
		if (nextLeft >= COLNUM || nextLeft < 0 || blockFlags[top][nextLeft])
		{
			return false;
		}
	}
	blockPosition.x += step;
	return true;
}
void RemoveLines()
{
	for (int row = 0; row < ROWNUM; row++)
	{
		bool bFull = true;
		for (int col = 0; col < COLNUM; col++)
		{
			if (!blockFlags[row][col])
			{
				bFull = false;
				break;
			}
		}
		if (bFull)
		{
			for (int r = row; r >= 0; r--)
			{
				for (int c = 0; c < COLNUM; c++)
				{
					blockFlags[r][c] = r==0?false:blockFlags[r - 1][c];
				}
			}
		}
	}
}
void CALLBACK BlockTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (bStopGame)
	{
		return;
	}
	bool canDown = StepDown();
	if (!canDown)
	{
		if (blockPosition.y <= 0)
		{
			ResetGame();
		}
		else
		{

			for (int i = 0; i < 4; i++)
			{
				int row = blockPosition.y + blockPoints[i].y;
				int col = blockPosition.x + blockPoints[i].x;
				blockFlags[row][col] = true;
			}
			RemoveLines();
			CreateNewBlock();
		}
	}
	::InvalidateRect(hWnd, nullptr, TRUE);
}

void Render(HDC hdc)
{
	HPEN hGrayPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hGrayPen);
	HBRUSH hGrayBrush = CreateSolidBrush(RGB(100, 100, 100));
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hGrayBrush);
	{//DrawWall
		int left = gameAreaLeft - BLOCKSIZE;
		int top = gameAreaTop - BLOCKSIZE;
		int right = gameAreaLeft + (COLNUM + 1) * BLOCKSIZE;
		int bottom = gameAreaTop;
		Rectangle(hdc, left, top, right, bottom);

		top = gameAreaTop + ROWNUM * BLOCKSIZE;
		bottom = top + BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);

		left = gameAreaLeft - BLOCKSIZE;
		top = gameAreaTop;
		right = gameAreaLeft;
		bottom = top + ROWNUM * BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);

		left = gameAreaLeft + COLNUM * BLOCKSIZE;
		right = left + BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);
	}
	HPEN hBlackPen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
	//::SelectObject(hdc, hBlackPen);
	HBRUSH hBlackBrush = CreateSolidBrush(RGB(20, 20, 20));
	::SelectObject(hdc, hBlackBrush);
	//DrawBlock
	if (!bStopGame)
	{
		for (int row = 0; row < ROWNUM; row++)
		{
			for (int col = 0; col < COLNUM; col++)
			{
				if (blockFlags[row][col])
				{
					int left = gameAreaLeft + col * BLOCKSIZE;
					int top = gameAreaTop + row * BLOCKSIZE;
					Rectangle(hdc, left, top, left + BLOCKSIZE, top + BLOCKSIZE);
				}
			}
		}
		for (int i = 0; i < 4; i++)
		{
			int left = gameAreaLeft + (blockPosition.x + blockPoints[i].x) * BLOCKSIZE;
			int top = gameAreaTop + (blockPosition.y + blockPoints[i].y) * BLOCKSIZE;
			Rectangle(hdc, left, top, left + BLOCKSIZE, top + BLOCKSIZE);
		}
	}
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hGrayBrush);
	DeleteObject(hGrayPen);
	DeleteObject(hBlackBrush);
	DeleteObject(hBlackPen);
}
LRESULT CALLBACK BlockWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		ResetGame();
		SetTimer(hWnd, 100, 1000 / Speed, (TIMERPROC)BlockTimerProc);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		break;
	}
	case WM_LBUTTONUP:
	{
		break;
	}
	case WM_MOUSEMOVE:
	{
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		Render(hdc);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_LEFT:
			StepLeftRight(-1);
			break;
		case VK_RIGHT:
			StepLeftRight(1);
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			StepDown();
			break;
		case VK_SPACE:
		{
			if (CanRotate())
			{
				ToNextRotator();
			}
			break;
		}
		case VK_BACK:
		{
			ResetGame();
			break;
		}
		case 0x0d:
		{
			if (bStopGame)
			{
				StartGame();
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
