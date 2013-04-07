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
#include "input.h"
#include "state.h"
#include "config.h"
#include "globals.h"
#include "graphics.h"
#include "game.h"
#include "namespace.h"
#include "actions.h"
#include "log.h"

#ifdef _WIN32
	#include <windows.h>
#endif

// Event receiver constructor
InputClass::InputClass()
:	MouseLocked(false),
	MouseX(0),
	MouseY(0),
	JoystickEnabled(false) {

	// Set up input
	ResetInputState();
}

// Event receiver for irrlicht
bool InputClass::OnEvent(const SEvent &Event) {
	bool Processed = false;

	if(Game::Instance().GetManagerState() != GameClass::STATE_UPDATE)
		return false;

	switch(Event.EventType) {
		case EET_KEY_INPUT_EVENT:
			Actions.KeyEvent(Event.KeyInput.Key, Event.KeyInput.PressedDown);

			// Send key press events
			if(Event.KeyInput.PressedDown && !GetKeyState(Event.KeyInput.Key))
				Processed = Game::Instance().GetState()->HandleKeyPress(Event.KeyInput.Key);
			else if(!Event.KeyInput.PressedDown)
				Processed = Game::Instance().GetState()->HandleKeyLift(Event.KeyInput.Key);

			// Set the current key state
			SetKeyState(Event.KeyInput.Key, Event.KeyInput.PressedDown);

			return Processed;
		break;
		case EET_MOUSE_INPUT_EVENT:

			switch(Event.MouseInput.Event) {
				case EMIE_LMOUSE_PRESSED_DOWN:
				case EMIE_RMOUSE_PRESSED_DOWN:
				case EMIE_MMOUSE_PRESSED_DOWN:
					SetMouseState(Event.MouseInput.Event, true);
					Actions.MouseEvent(Event.MouseInput.Event, true);
					return Game::Instance().GetState()->HandleMousePress(Event.MouseInput.Event, Event.MouseInput.X, Event.MouseInput.Y);
				break;
				case EMIE_LMOUSE_LEFT_UP:
				case EMIE_RMOUSE_LEFT_UP:
				case EMIE_MMOUSE_LEFT_UP:
					SetMouseState(Event.MouseInput.Event - MOUSE_COUNT, false);
					Actions.MouseEvent(Event.MouseInput.Event - MOUSE_COUNT, false);
					Game::Instance().GetState()->HandleMouseLift(Event.MouseInput.Event - MOUSE_COUNT, Event.MouseInput.X, Event.MouseInput.Y);
				break;
				case EMIE_MOUSE_MOVED:

					// Save mouse position
					MouseX = Event.MouseInput.X;
					MouseY = Event.MouseInput.Y;

					// Check for mouse locking
					if(MouseLocked) {
						position2df MouseUpdate = irrDevice->getCursorControl()->getRelativePosition();

						// Center the cursor
						if(!(equals(MouseUpdate.X, 0.5f) && equals(MouseUpdate.Y, 0.5f)))
							irrDevice->getCursorControl()->setPosition(0.5f, 0.5f);						
						
						// Invert mouse
						float MouseScaleY = Config::Instance().MouseScaleY;
						if(Config::Instance().InvertMouse)
							MouseScaleY = -MouseScaleY;

						Game::Instance().GetState()->HandleMouseMotion((MouseUpdate.X - 0.5f) * irrDriver->getScreenSize().Width * 0.1f * Config::Instance().MouseScaleX,
																				(MouseUpdate.Y - 0.5f) * irrDriver->getScreenSize().Height * 0.1f * MouseScaleY);
					}
				break;
				case EMIE_MOUSE_WHEEL:
					Game::Instance().GetState()->HandleMouseWheel(Event.MouseInput.Wheel);
				break;
				default:
				break;
			}

			return false;
		break;
		case EET_GUI_EVENT:
			Game::Instance().GetState()->HandleGUI(Event.GUIEvent.EventType, Event.GUIEvent.Caller);
		break;
		case EET_JOYSTICK_INPUT_EVENT:
			JoystickState = Event.JoystickEvent;
		default:
		break;
	}

	return false;
}

// Set up joysticks
void InputClass::InitializeJoysticks() {

	// Find joysticks
	if(irrDevice->activateJoysticks(Joysticks)) {
		JoystickEnabled = true;
		Log.Write("%d joystick(s) found.", Joysticks.size());

		for(u32 i = 0; i < Joysticks.size(); i++) {
			Log.Write("Joystick %d", i);
			Log.Write("\tName: %s", Joysticks[i].Name.c_str());
			Log.Write("\tAxes: %d", Joysticks[i].Axes);
			Log.Write("\tButtons: %d", Joysticks[i].Buttons);

			switch(Joysticks[i].PovHat) {
				case SJoystickInfo::POV_HAT_PRESENT:
					Log.Write("\tHat is present");
				break;
				case SJoystickInfo::POV_HAT_ABSENT:
					Log.Write("\tHat is absent");
				break;
				case SJoystickInfo::POV_HAT_UNKNOWN:
				default:
					Log.Write("\tHat is unknown");
				break;
			}
		}
	}
	else
		JoystickEnabled = false;
}

// Return the joystick state
const irr::SEvent::SJoystickEvent &InputClass::GetJoystickEvent() {

	return JoystickState;
}

// Get a joystick axis value
float InputClass::GetAxis(int Axis) {
	if(!JoystickEnabled)
		return 0.0f;

	float Value = JoystickState.Axis[Axis] / 32767.f;
	if(Value < -1.0f)
		Value = -1.0f;
	if(Value > 1.0f)
		Value = 1.0f;
	return Value;
}

// Resets the keyboard state
void InputClass::ResetInputState() {

	for(int i = 0; i < KEY_KEY_CODES_COUNT; i++)
		Keys[i] = 0;

	Actions.ResetState();
}

// Enables mouse locking
void InputClass::SetMouseLocked(bool Value) {
	
	MouseLocked = Value;
	if(MouseLocked) {
		irrDevice->getCursorControl()->setPosition(0.5f, 0.5f);
		#ifdef _WIN32
			SetCapture(GetActiveWindow());
		#endif
	}
	else {
		#ifdef _WIN32
			ReleaseCapture();
		#endif
	}

	Graphics::Instance().ToggleCursor(!Value);
}

// Converts an irrlicht key code into a string
const char *InputClass::GetKeyName(int Key) {

	switch(Key) {
		case KEY_KEY_0:
			return "0";
		break;
		case KEY_KEY_1:
			return "1";
		break;
		case KEY_KEY_2:
			return "2";
		break;
		case KEY_KEY_3:
			return "3";
		break;
		case KEY_KEY_4:
			return "4";
		break;
		case KEY_KEY_5:
			return "5";
		break;
		case KEY_KEY_6:
			return "6";
		break;
		case KEY_KEY_7:
			return "7";
		break;
		case KEY_KEY_8:
			return "8";
		break;
		case KEY_KEY_9:
			return "9";
		break;
		case KEY_KEY_A:
			return "a";
		break;
		case KEY_KEY_B:
			return "b";
		break;
		case KEY_KEY_C:
			return "c";
		break;
		case KEY_KEY_D:
			return "d";
		break;
		case KEY_KEY_E:
			return "e";
		break;
		case KEY_KEY_F:
			return "f";
		break;
		case KEY_KEY_G:
			return "g";
		break;
		case KEY_KEY_H:
			return "h";
		break;
		case KEY_KEY_I:
			return "i";
		break;
		case KEY_KEY_J:
			return "j";
		break;
		case KEY_KEY_K:
			return "k";
		break;
		case KEY_KEY_L:
			return "l";
		break;
		case KEY_KEY_M:
			return "m";
		break;
		case KEY_KEY_N:
			return "n";
		break;
		case KEY_KEY_O:
			return "o";
		break;
		case KEY_KEY_P:
			return "p";
		break;
		case KEY_KEY_Q:
			return "q";
		break;
		case KEY_KEY_R:
			return "r";
		break;
		case KEY_KEY_S:
			return "s";
		break;
		case KEY_KEY_T:
			return "t";
		break;
		case KEY_KEY_U:
			return "u";
		break;
		case KEY_KEY_V:
			return "v";
		break;
		case KEY_KEY_W:
			return "w";
		break;
		case KEY_KEY_X:
			return "x";
		break;
		case KEY_KEY_Y:
			return "y";
		break;
		case KEY_KEY_Z:
			return "z";
		break;
		case KEY_LEFT:
			return "left";
		break;
		case KEY_UP:
			return "up";
		break;
		case KEY_RIGHT:
			return "right";
		break;
		case KEY_DOWN:
			return "down";
		break;
		case KEY_SPACE:
			return "space";
		break;
		case KEY_SHIFT:
			return "shift";
		break;
		case KEY_CONTROL:
			return "control";
		break;
		case KEY_TAB:
			return "tab";
		break;
		case KEY_RETURN:
			return "enter";
		break;
	}

	return "";
}
