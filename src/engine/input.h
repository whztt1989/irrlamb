/*************************************************************************************
*	irrlamb - http://irrlamb.googlecode.com
*	Copyright (C) 2013  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include <all.h>
#pragma once

// Classes
class _Input : public irr::IEventReceiver  {

	enum MouseButtonType {
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		MOUSE_COUNT,
	};

	public:

		_Input();
		bool OnEvent(const irr::SEvent &Event);

		void ResetInputState();
		void InitializeJoysticks();
		bool GetKeyState(int Key) const { return Keys[Key]; }
		bool GetMouseState(int Button) const { return MouseButtons[Button]; }

		void SetMouseLocked(bool Value);
		bool GetMouseLocked() const { return MouseLocked; }

		int GetMouseX() const { return MouseX; }
		int GetMouseY() const { return MouseY; }

		bool IsJoystickEnabled() const { return JoystickEnabled; }
		const irr::SEvent::SJoystickEvent &GetJoystickState();
		float GetAxis(int Axis);

		const char *GetKeyName(int Key);

	private:

		void SetKeyState(int Key, bool State) { Keys[Key] = State; }
		void SetMouseState(int Button, bool State) { MouseButtons[Button] = State; }

		// Input
		bool Keys[irr::KEY_KEY_CODES_COUNT], MouseButtons[MOUSE_COUNT];

		// Joystick
		irr::core::array<irr::SJoystickInfo> Joysticks;
		irr::SEvent::SJoystickEvent JoystickState;
		irr::u32 LastJoystickButtonState;
		float DeadZone;

		// States
		bool MouseLocked;
		bool JoystickEnabled;
		int MouseX, MouseY;
};

// Singletons
extern _Input Input;
