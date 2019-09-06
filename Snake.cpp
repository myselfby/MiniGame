#include<windows.h>
#include<list>
#include<time.h>
#define BLOCKSIZE 10
LPCWSTR lpTitle = L"Snake";                  // 标题栏文本
LPCWSTR lpWindowClass = L"Snake";            // 主窗口类名

struct Food
{
	int time;
	POINT position;
};
POINT snakePosition;
POINT snakeVelocity;
std::list<POINT> snakePoints;
std::list<Food> foodList;
int rowNum = 40;
int colNum = 40;
int Speed = 2;
int maxFoodNum = 1;
int maxFoodTime = -1;
int gameAreaLeft = BLOCKSIZE;
int gameAreaTop = BLOCKSIZE;

void ResetGame();
bool GetValidFoodPosition(POINT& position);
void Render(HDC hdc);
void CALLBACK SnakeTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK SnakeWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	wcex.lpfnWndProc = SnakeWindowProc;
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
	snakePosition.x = colNum / 2;
	snakePosition.y = rowNum / 2;
	snakeVelocity.x = snakeVelocity.y = 0;
	foodList.clear();
	snakePoints.clear();
	snakePoints.push_back(snakePosition);
	srand(time(0));
}
bool GetValidFoodPosition(POINT& position)
{
	bool bValidFood = false;
	while (!bValidFood)
	{
		position.x = rand() % colNum;
		position.y = rand() % rowNum;
		bValidFood = true;
		for (auto iter = foodList.begin(); iter != foodList.end(); iter++)
		{
			if (iter->position.x == position.x || iter->position.y == position.y)
			{
				bValidFood = false;
				break;
			}
		}
		if (bValidFood)
		{
			for (auto iter = snakePoints.begin(); iter != snakePoints.end(); iter++)
			{
				if (iter->x == position.x || iter->y == position.y)
				{
					bValidFood = false;
					break;
				}
			}
		}
	}
	return bValidFood;
}
void CALLBACK SnakeTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (snakeVelocity.x == 0 && snakeVelocity.y == 0)
	{
		return;
	}
	snakePosition.x += snakeVelocity.x;
	snakePosition.y += snakeVelocity.y;
	if (snakePosition.x >= colNum
		|| snakePosition.x < 0
		|| snakePosition.y >= rowNum
		|| snakePosition.y < 0)
	{
		ResetGame();
	}
	else
	{
		bool bEat = false;
		std::list<Food>::iterator iter = foodList.begin();
		for (iter = foodList.begin(); iter != foodList.end();)
		{
			auto tmpIter = iter;
			if (maxFoodTime > 0 && iter->time > maxFoodTime)
			{
				tmpIter++;
				foodList.erase(iter);
				iter = tmpIter;
			}
			else if (snakePosition.x == iter->position.x && snakePosition.y == iter->position.y)
			{
				bEat = true;
				tmpIter++;
				foodList.erase(iter);
				iter = tmpIter;
			}
			else
			{
				iter->time += dwTime;
				iter++;
			}
		}
		if (foodList.size() < maxFoodNum)
		{
			POINT pt;
			if (GetValidFoodPosition(pt))
			{
				Food food;
				food.position = pt;
				food.time = 0;
				foodList.push_back(food);
			}
		}
		snakePoints.push_front(snakePosition);
		if (!bEat)
		{
			snakePoints.pop_back();
		}
	}
	::InvalidateRect(hWnd, nullptr, TRUE);
}

void Render(HDC hdc)
{
	HPEN hGrayPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hGrayPen);
	HBRUSH hGrayBrush = CreateSolidBrush(RGB(100, 100,100));
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hGrayBrush);
	{//DrawWall
		int left = gameAreaLeft - BLOCKSIZE;
		int top = gameAreaTop - BLOCKSIZE;
		int right = gameAreaLeft + (colNum + 1)*BLOCKSIZE;
		int bottom = gameAreaTop;
		Rectangle(hdc, left, top, right, bottom);

		top = gameAreaTop + rowNum * BLOCKSIZE;
		bottom = top + BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);

		left = gameAreaLeft - BLOCKSIZE;
		top = gameAreaTop;
		right = gameAreaLeft;
		bottom = top + rowNum * BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);

		left = gameAreaLeft + colNum * BLOCKSIZE;
		right = left + BLOCKSIZE;
		Rectangle(hdc, left, top, right, bottom);
	}
	HPEN hBlackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	::SelectObject(hdc, hBlackPen);
	HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
	::SelectObject(hdc, hBlackBrush);
	//DrawSnakeAndFood
	for (auto iter = snakePoints.begin(); iter != snakePoints.end(); iter++)
	{
		int left = gameAreaLeft + iter->x*BLOCKSIZE;
		int top  = gameAreaTop + iter->y*BLOCKSIZE;
		Rectangle(hdc, left, top, left+ BLOCKSIZE, top + BLOCKSIZE);
	}
	for (auto iter = foodList.begin(); iter != foodList.end(); iter++)
	{
		POINT& pt = iter->position;
		int left = gameAreaLeft + pt.x*BLOCKSIZE;
		int top = gameAreaTop + pt.y*BLOCKSIZE;
		Rectangle(hdc, left, top, left+ BLOCKSIZE, top + BLOCKSIZE);
	}

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hGrayBrush);
	DeleteObject(hGrayPen);
	DeleteObject(hBlackBrush);
	DeleteObject(hBlackPen);
}
LRESULT CALLBACK SnakeWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		ResetGame();
		SetTimer(hWnd, 100, 1000/Speed, (TIMERPROC)SnakeTimerProc);
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
			snakeVelocity.x = -1;
			snakeVelocity.y = 0;
			break;
		case VK_RIGHT:
			snakeVelocity.x = 1;
			snakeVelocity.y = 0;
			break;
		case VK_UP:
			snakeVelocity.x = 0;
			snakeVelocity.y = -1;
			break;
		case VK_DOWN:
			snakeVelocity.x = 0;
			snakeVelocity.y = 1;
			break;
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