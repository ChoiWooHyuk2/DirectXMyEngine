#pragma once
#include "ErrorLogger.h"

class WindowContainer;

class RenderWindow
{
public:
	bool Initialize(WindowContainer* pWindowContainer,HINSTANCE hInstace, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	HWND GetHWND() const;
	~RenderWindow();
private:
	void RegisterWindowClass();
	HWND handle = NULL;									//윈도우 핸드
	HINSTANCE hInstance = NULL;							// 어플리케이션 핸들 인스턴스 
	std::string window_title = "";
	std::wstring window_title_wide = L"";              //윈도우 타이틀
	std::string window_class = "";
	std::wstring window_class_wide = L"";             //윈도우 클래스 이름
	int width = 0;
	int height = 0;
};

