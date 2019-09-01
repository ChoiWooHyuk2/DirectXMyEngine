#pragma once
#include "KeyboardEvent.h"
#include <queue>

class KeyboardClass
{
public:
	bool KeyIsPressed(const unsigned char keycode);
	bool KeyBufferIsEmpty();
	bool CharBufferIsEmpty();
	KeyboardEvent ReadKey();
	unsigned char ReadChar();
	void OnKeyPressed(const unsigned char key);
	void OnKeyReleased(const unsigned char key);
	void OnChar(const unsigned char key);
	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	bool IsKeysAutoRepeat();
	bool IsCharsAutoRepeat();
private:
	bool autoRepeatKeys = false;
	bool autoRepeatChars = false;
	bool KeyStates[256];
	std::queue<KeyboardEvent> KeyBuffer;
	std::queue<unsigned char> charBuffer;
public:
	KeyboardClass();
};

