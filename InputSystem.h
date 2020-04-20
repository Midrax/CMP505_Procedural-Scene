#pragma once
#include "pch.h"
#include <SFML/Window.hpp>

struct InputCommands
{
	bool forward;
	bool back;
	bool right;
	bool left;
	bool rotRight;
	bool rotLeft;
	bool generate;
};

class InputSystem
{
	public:
		void Initialize(HWND window);

		void OnSuspending();
		void OnDeactivated();

		void OnResuming(); 
		void OnActivated();

		void Update();
	private:
		bool m_quitApp;
		InputCommands m_GameInput;
};

