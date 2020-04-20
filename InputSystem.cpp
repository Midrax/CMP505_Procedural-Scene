#include "pch.h"
#include "InputSystem.h"

using namespace DirectX;

void InputSystem::Initialize(HWND window)
{

}

void InputSystem::OnSuspending()
{

}

void InputSystem::OnDeactivated()
{

}

void InputSystem::OnResuming()
{

}

void InputSystem::OnActivated()
{

}

void InputSystem::Update() 
{ 
	sf::Joystick::update();
	if (sf::Joystick::isConnected(0))
	{
		if (sf::Joystick::isButtonPressed(0, 0))
		{
			// SQUARE/X
		}
		if (sf::Joystick::isButtonPressed(0, 1))
		{
			// CROSS/A
		}
		if (sf::Joystick::isButtonPressed(0, 2))
		{
			// CIRCLE/B
		}
		if (sf::Joystick::isButtonPressed(0, 3))
		{
			// TRIANGLE/Y
		}
		if (sf::Joystick::isButtonPressed(0, 4))
		{
			// L1/LB
		}
		if (sf::Joystick::isButtonPressed(0, 5))
		{
			// R1/RB
		}
		if (sf::Joystick::isButtonPressed(0, 6))
		{
			// L2/LT
		}
		if (sf::Joystick::isButtonPressed(0, 7))
		{
			// R2/RT
		}
		if (sf::Joystick::isButtonPressed(0, 8))
		{
			// SELECT
		}
		if (sf::Joystick::isButtonPressed(0, 9))
		{
			// START
		}
		if (sf::Joystick::isButtonPressed(0, 10))
		{
			// L3
		}
		if (sf::Joystick::isButtonPressed(0, 11))
		{
			// R3
		}
		if (sf::Joystick::isButtonPressed(0, 13))
		{
			// PAD
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) != 0)
		{
			// Left Stick Pointing Right with 0 < X < 100
			// Left Stick Pointing Left with -100 < X < 0
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) != 0)
		{
			// Left Stick Pointing Down with 0 < X < 100
			// Left Stick Pointing Up with -100 < X < 0
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z) != 0)
		{
			// Right Stick Pointing Right with 0 < X < 100
			// Right Stick Pointing Left with -100 < X < 0
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R) != 0)
		{
			// Right Stick Pointing Down with 0 < X < 100
			// Right Stick Pointing Up with -100 < X < 0
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U) != 0)
		{
			// Left Trigger Pressure!
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V) != 0)
		{
			// Right Trigger Pressure!
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U) != 0)
		{
			// Left Trigger Pressure!
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) != 0)
		{
			// DPAD Pointing Right with 0 < X < 100
			// DPAD Pointing Left with -100 < X < 0
		}
		if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) != 0)
		{
			// DPAD Pointing Right with 0 < X < 100
			// DPAD Pointing Left with -100 < X < 0
		}
	}
}
