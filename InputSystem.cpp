#include "pch.h"
#include "InputSystem.h"

using namespace DirectX;

void InputSystem::Initialize(HWND window)
{
	// Initialize Keyboard and Mouse
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);

	m_GameInput.forward = false;
	m_GameInput.back = false;
	m_GameInput.right = false;
	m_GameInput.left = false;
	m_GameInput.upArrow = false;
	m_GameInput.downArrow = false;
	m_GameInput.rightArrow = false;
	m_GameInput.leftArrow = false;
	m_GameInput.q_key = false;
	m_GameInput.e_key = false;
	m_GameInput.space = false;
	m_GameInput.left_shift = false;
	m_GameInput.ctrl = false;
	m_GameInput.escape = false;

	// Initialize Controller
	m_GameInput.squareButton = false;
	m_GameInput.crossButton = false;
	m_GameInput.circleButton = false;
	m_GameInput.triangleButton = false;
	m_GameInput.L1_Button = false;
	m_GameInput.R1_Button = false;
	m_GameInput.L2_Button = false;
	m_GameInput.R2_Button = false;
	m_GameInput.selectButton = false;
	m_GameInput.startButton = false;
	m_GameInput.L3_Button = false;
	m_GameInput.R3_Button = false;
	m_GameInput.pad_Button = false;
	m_GameInput.leftStick_X = 0;
	m_GameInput.leftStick_Y = 0;
	m_GameInput.rightStick_X = 0;
	m_GameInput.rightStick_Y = 0;
	m_GameInput.leftTrigger_pressure = 0;
	m_GameInput.rightTrigger_pressure = 0;
	m_GameInput.dPad_X = 0;
	m_GameInput.dPad_Y = 0;
}

void InputSystem::Update() 
{ 
	ControllerUpdate();
	
	auto kb = m_keyboard->GetState();
	if (kb.W)
	{
		m_GameInput.forward = true;
	}
	else
		m_GameInput.forward = false;

	if (kb.S)
	{
		m_GameInput.back = true;
	}
	else
		m_GameInput.back = false;

	if (kb.D)
	{
		m_GameInput.right = true;
	}
	else
		m_GameInput.right = false;

	if (kb.A)
	{
		m_GameInput.left = true;
	}
	else
		m_GameInput.left = false;

	if (kb.Up)
	{
		m_GameInput.upArrow = true;
	}
	else
		m_GameInput.upArrow = false;

	if (kb.Down)
	{
		m_GameInput.downArrow = true;
	}
	else
		m_GameInput.downArrow = false;

	if (kb.Right)
	{
		m_GameInput.rightArrow = true;
	}
	else
		m_GameInput.rightArrow = false;

	if (kb.Left)
	{
		m_GameInput.leftArrow = true;
	}
	else
		m_GameInput.leftArrow = false;
	
	if (kb.Q)
	{
		m_GameInput.q_key = true;
	}
	else
		m_GameInput.q_key = false;

	if (kb.E)
	{
		m_GameInput.e_key = true;
	}
	else
		m_GameInput.e_key = false;

	if (kb.Space)
	{
		m_GameInput.space = true;
	}
	else
		m_GameInput.space = false;

	if (kb.LeftShift)
	{
		m_GameInput.left_shift = true;
	}
	else
		m_GameInput.left_shift = false;

	if (kb.LeftControl)
	{
		m_GameInput.ctrl = true;
	}
	else
		m_GameInput.ctrl = false;

	if (kb.Escape)
	{
		m_GameInput.escape = true;
	}
	else
		m_GameInput.escape = false;

}


void InputSystem::ControllerUpdate()
{
	sf::Joystick::update();
	if (sf::Joystick::isConnected(0))
	{
		if (sf::Joystick::isButtonPressed(0, 0))
		{
			m_GameInput.squareButton = true;
		}
		else
			m_GameInput.squareButton = false;
		if (sf::Joystick::isButtonPressed(0, 1))
		{
			m_GameInput.crossButton = true;
		}
		else
			m_GameInput.crossButton = false;
		if (sf::Joystick::isButtonPressed(0, 2))
		{
			m_GameInput.circleButton = true;
		}
		else
			m_GameInput.circleButton = false;
		if (sf::Joystick::isButtonPressed(0, 3))
		{
			m_GameInput.triangleButton = true;
		}
		else
			m_GameInput.triangleButton = false;
		if (sf::Joystick::isButtonPressed(0, 4))
		{
			m_GameInput.L1_Button = true;
		}
		else
			m_GameInput.L1_Button = false;
		if (sf::Joystick::isButtonPressed(0, 5))
		{
			m_GameInput.R1_Button = true;
		}
		else
			m_GameInput.R1_Button = false;
		if (sf::Joystick::isButtonPressed(0, 6))
		{
			m_GameInput.L2_Button = true;
		}
		else
			m_GameInput.L2_Button = false;
		if (sf::Joystick::isButtonPressed(0, 7))
		{
			m_GameInput.R2_Button = true;
		}
		else
			m_GameInput.R2_Button = false;
		if (sf::Joystick::isButtonPressed(0, 8))
		{
			m_GameInput.selectButton = true;
		}
		else
			m_GameInput.selectButton = false;
		if (sf::Joystick::isButtonPressed(0, 9))
		{
			m_GameInput.startButton = true;
		}
		else
			m_GameInput.startButton = false;
		if (sf::Joystick::isButtonPressed(0, 10))
		{
			m_GameInput.L3_Button = true;
		}
		else
			m_GameInput.L3_Button = false;
		if (sf::Joystick::isButtonPressed(0, 11))
		{
			m_GameInput.R3_Button = true;
		}
		else
			m_GameInput.R3_Button = false;
		if (sf::Joystick::isButtonPressed(0, 13))
		{
			m_GameInput.pad_Button = true;
		}
		else
			m_GameInput.pad_Button = false;

		// Pointing Right with 0 < X < 100
		// Pointing Left with -100 < X < 0

		m_GameInput.leftStick_X = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
		m_GameInput.leftStick_Y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
		m_GameInput.rightStick_X = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z);
		m_GameInput.rightStick_Y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R);
		m_GameInput.leftTrigger_pressure = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U);
		m_GameInput.rightTrigger_pressure = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V);
		m_GameInput.dPad_X = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
		m_GameInput.dPad_Y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
	}
}

InputCommands InputSystem::getGameInput()
{
	return m_GameInput;
}
