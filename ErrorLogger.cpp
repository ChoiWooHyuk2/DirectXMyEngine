#include "ErrorLogger.h"
#include <comdef.h>

void ErrorLogger::Log(std::string message)
{
	std::string error_message = "[오류]  " + message;
	MessageBoxA(NULL, error_message.c_str(), "오류", MB_ICONERROR);
}

void ErrorLogger::Log(HRESULT hr, std::string message)
{
	_com_error error(hr);
	std::wstring error_message = L"[오류]  " + StringConverter::StringToWide(message) + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"오류", MB_ICONERROR);
}

void ErrorLogger::Log(HRESULT hr, std::wstring message)
{
	_com_error error(hr);
	std::wstring error_message = L"[오류]  " + message + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"오류", MB_ICONERROR);
}
