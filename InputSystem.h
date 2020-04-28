#pragma once
#include "pch.h"
#include <SFML/Window.hpp>

struct InputCommands
{
	bool forward;
	bool back;
	bool right;
	bool left;
	bool upArrow;
	bool downArrow;
	bool rightArrow;
	bool leftArrow;
	bool q_key;
	bool e_key;
	bool space;
	bool left_shift;
	bool ctrl;
	bool escape;
	bool alt;
	bool tab;

	bool squareButton;
	bool crossButton;
	bool circleButton;
	bool triangleButton;
	bool L1_Button;
	bool R1_Button;
	bool L2_Button;
	bool R2_Button;
	bool selectButton;
	bool startButton;
	bool L3_Button;
	bool R3_Button;
	bool pad_Button;
	float leftStick_X;
	float leftStick_Y;
	float rightStick_X;
	float rightStick_Y;
	float leftTrigger_pressure;
	float rightTrigger_pressure;
	float dPad_X;
	float dPad_Y;
};

class InputSystem
{
	public:
		std::unique_ptr<DirectX::Mouse> mouse;
		DirectX::Mouse::ButtonStateTracker mouseButtons;
		DirectX::Keyboard::KeyboardStateTracker keys;

		void Initialize(HWND window);

		void Update();
		void ControllerUpdate();

		void Reset();

		InputCommands getGameInput();
	private:
		bool m_quitApp;
		InputCommands m_GameInput;
		std::unique_ptr<DirectX::Keyboard> m_keyboard;
};

