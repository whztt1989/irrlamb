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
#ifndef INPUT_H
#define INPUT_H

// Libraries
#include "singleton.h"
#include <irrlicht.h>

// Classes
class InputClass : public irr::IEventReceiver  {

	enum MouseButtonType {
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		MOUSE_COUNT,
	};

	public:

		InputClass();
		bool OnEvent(const irr::SEvent &Event);

		void ResetInputState();
		bool GetKeyState(int Key) const { return Keys[Key]; }
		bool GetMouseState(int Button) const { return MouseButtons[Button]; }

		void SetMouseLocked(bool Value);
		bool GetMouseLocked() const { return MouseLocked; }

		int GetMouseX() const { return MouseX; }
		int GetMouseY() const { return MouseY; }

		const char *GetKeyName(int Key);

	private:

		void SetKeyState(int Key, bool State) { Keys[Key] = State; }
		void SetMouseState(int Button, bool State) { MouseButtons[Button] = State; }

		// Input
		bool Keys[irr::KEY_KEY_CODES_COUNT], MouseButtons[MOUSE_COUNT];

		// States
		bool MouseLocked;
		int MouseX, MouseY;
};

// Singletons
typedef SingletonClass<InputClass> Input;

#endif
