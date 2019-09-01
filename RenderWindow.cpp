#include "WindowContainer.h"

bool RenderWindow::Initialize(WindowContainer* pWindowContainer,HINSTANCE hInstace, std::string window_title, std::string window_class, int width, int height)
{
	this->hInstance = hInstace;
	this->width = width;
	this->height = height;
	this->window_title = window_title;
	this->window_title_wide = StringConverter::StringToWide(this->window_title);
	this->window_class = window_class;
	this->window_class_wide = StringConverter::StringToWide(this->window_class);

	this->RegisterWindowClass();

	int centerScreenX = GetSystemMetrics(SM_CXSCREEN) / 2 - this->width / 2;
	int centerScreenY = GetSystemMetrics(SM_CYSCREEN) / 2 - this->height / 2;

	RECT wr; //윈도우 렉트앵글
	wr.left = centerScreenX;
	wr.top = centerScreenY;
	wr.right = wr.left + this->width;
	wr.bottom = wr.top + this->height;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	this->handle = CreateWindowEx(0,
		this->window_class_wide.c_str(),  //클래스 이름
		this->window_title_wide.c_str(),  //타이틀
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, // 윈도우 스타일
		wr.left, //윈도우 x 포지션
		wr.top, //윈도우 y 포지션
		wr.right - wr.left, //윈도우 너비
		wr.bottom - wr.top, //윈도우 높이
		NULL,
		NULL,
		this->hInstance,
		pWindowContainer
		);

	if (this->handle == NULL)
	{
		ErrorLogger::Log(GetLastError(), " 윈도우EX 생성에 실패했습니다." + this->window_title);
		return false;
	}
	//윈도우 포커스 설정
	ShowWindow(this->handle, SW_SHOW);
	SetForegroundWindow(this->handle);
	SetFocus(this->handle);

	return true;
}

bool RenderWindow::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); //메세지 초기화

	while (PeekMessage(&msg, this->handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL)
	{
		if (!IsWindow(this->handle))
		{
			this->handle = NULL;
			UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
			return false;
		}
	}

	return true;
}
HWND RenderWindow::GetHWND() const
{
	return this->handle;
}
LRESULT CALLBACK HandleMagRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		return 0;
	}
	default:
	{
		WindowContainer* const pWindow = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	}


	WindowContainer* const pWindow = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

RenderWindow::~RenderWindow()
{
	if (this->handle != NULL)
	{
		UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
		DestroyWindow(handle);
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowContainer* pWindow = reinterpret_cast<WindowContainer*>(pCreate->lpCreateParams);;
		if (pWindow == nullptr)
		{
			ErrorLogger::Log(" 치명적인 오류: 윈도우 컨테이너의 포인터가 NULL 입니다.");
			exit(0);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMagRedirect));
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	
}

void RenderWindow::RegisterWindowClass()
{
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = HandleMessageSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = this->hInstance;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = this->window_class_wide.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);
	
}
