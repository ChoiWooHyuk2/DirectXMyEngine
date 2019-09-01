#pragma once
#include "RenderWindow.h"
#include "Input\KeyboardClass.h"
#include "Mouse/MouseClass.h"
#include "Graphics\Graphics.h"
class WindowContainer
{
protected:
	RenderWindow render_window;
	KeyboardClass Keyboard;
	MouseClass mouse;
	Graphics gfx;
public:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	WindowContainer();
	~WindowContainer();


};

