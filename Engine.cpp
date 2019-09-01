#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = timer.GetMilisecondsElapsed();
	timer.Restart();

	while (!Keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = Keyboard.ReadChar();
	}

	while (!Keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = Keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EvnetBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->gfx.camera.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01f, 0);
			}
		}
	}

	const float cameraSpeed = 0.006f;

	if (Keyboard.KeyIsPressed('W'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * dt);
	}
	if (Keyboard.KeyIsPressed('S'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * dt);
	}
	if (Keyboard.KeyIsPressed('A'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * dt);
	}
	if (Keyboard.KeyIsPressed('D'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * dt);
	}
	if (Keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
	}
	if (Keyboard.KeyIsPressed(VK_SHIFT))
	{
		this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
	}



}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}