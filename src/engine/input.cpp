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
#include <engine/input.h>
#include <engine/state.h>
#include <engine/config.h>
#include <engine/globals.h>
#include <engine/graphics.h>
#include <engine/game.h>
#include <engine/namespace.h>
#include <engine/actions.h>
#include <engine/log.h>

_Input Input;

// Event receiver constructor
_Input::_Input()
:	MouseLocked(false),
	MouseX(0),
	MouseY(0),
	DeadZone(0.05f),
	VirtualMouseMoved(false),
	LastJoystickButtonState(0) {

	// Set up input
	ResetInputState();
}

// Event receiver for irrlicht
bool _Input::OnEvent(const SEvent &Event) {
	bool Processed = false;

	if(Game.GetManagerState() != _Game::STATE_UPDATE)
		return false;

	switch(Event.EventType) {
		case EET_KEY_INPUT_EVENT:
			
			// Send key press events
			if(Event.KeyInput.PressedDown && !GetKeyState(Event.KeyInput.Key)) {
				Processed = Game.GetState()->HandleKeyPress(Event.KeyInput.Key);
				Actions.InputEvent(_Input::KEYBOARD, Event.KeyInput.Key, Event.KeyInput.PressedDown);
			}
			else if(!Event.KeyInput.PressedDown) {
				Processed = Game.GetState()->HandleKeyLift(Event.KeyInput.Key);
				Actions.InputEvent(_Input::KEYBOARD, Event.KeyInput.Key, Event.KeyInput.PressedDown);
			}

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
					if(!Event.UserEvent.UserData1)
						Actions.InputEvent(_Input::MOUSE_BUTTON, Event.MouseInput.Event, true);
					return Game.GetState()->HandleMousePress(Event.MouseInput.Event, Event.MouseInput.X, Event.MouseInput.Y);
				break;
				case EMIE_LMOUSE_LEFT_UP:
				case EMIE_RMOUSE_LEFT_UP:
				case EMIE_MMOUSE_LEFT_UP:
					SetMouseState(Event.MouseInput.Event - MOUSE_COUNT, false);
					if(!Event.UserEvent.UserData1)
						Actions.InputEvent(_Input::MOUSE_BUTTON, Event.MouseInput.Event - MOUSE_COUNT, false);
					Game.GetState()->HandleMouseLift(Event.MouseInput.Event - MOUSE_COUNT, Event.MouseInput.X, Event.MouseInput.Y);
				break;
				case EMIE_MOUSE_MOVED:

					// Save mouse position
					if(!VirtualMouseMoved) {
						MouseX = (float)Event.MouseInput.X;
						MouseY = (float)Event.MouseInput.Y;
					}
					VirtualMouseMoved = false;

					// Check for mouse locking
					if(MouseLocked) {
						position2df MouseUpdate = irrDevice->getCursorControl()->getRelativePosition();

						// Center the cursor
						if(!(equals(MouseUpdate.X, 0.5f) && equals(MouseUpdate.Y, 0.5f)))
							irrDevice->getCursorControl()->setPosition(0.5f, 0.5f);						
						
						// Invert mouse
						float MouseScaleY = Config.MouseScaleY;
						if(Config.InvertMouse)
							MouseScaleY = -MouseScaleY;

						float MouseValueX = (MouseUpdate.X - 0.5f) * irrDriver->getScreenSize().Width * 0.1f * Config.MouseScaleX;
						float MouseValueY = (MouseUpdate.Y - 0.5f) * irrDriver->getScreenSize().Height * 0.1f * MouseScaleY;
						int AxisX = MouseValueX < 0.0f ? 0 : 1;
						int AxisY = MouseValueY < 0.0f ? 2 : 3;
						Actions.InputEvent(_Input::MOUSE_AXIS, AxisX, fabs(MouseValueX));
						Actions.InputEvent(_Input::MOUSE_AXIS, AxisY, fabs(MouseValueY));
						Game.GetState()->HandleMouseMotion(MouseValueX, MouseValueY);
					}
				break;
				case EMIE_MOUSE_WHEEL:
					Game.GetState()->HandleMouseWheel(Event.MouseInput.Wheel);
				break;
				default:
				break;
			}

			return false;
		break;
		case EET_GUI_EVENT:
			Game.GetState()->HandleGUI(Event.GUIEvent.EventType, Event.GUIEvent.Caller);
		break;
		case EET_JOYSTICK_INPUT_EVENT: {
			if(!Config.JoystickEnabled || Event.JoystickEvent.Joystick > 0)
				return false;

			LastJoystickButtonState = JoystickState.ButtonStates;
			JoystickState = Event.JoystickEvent;

			// Handle buttons
			for(u32 i = 0; i < Joysticks[JoystickState.Joystick].Buttons; i++) {
				if(JoystickState.IsButtonPressed(i) && !(LastJoystickButtonState & (1 << i))) {
					Actions.InputEvent(_Input::JOYSTICK_BUTTON, i, true);
				}
				else if(!JoystickState.IsButtonPressed(i) && (LastJoystickButtonState & (1 << i))) {
					Actions.InputEvent(_Input::JOYSTICK_BUTTON, i, false);
				}
			}

			// Handles axes
			for(u32 i = 0; i < Joysticks[JoystickState.Joystick].Axes; i++) {
				float AxisValue = GetAxis(i);
				if(AxisValue != 0.0f) {
					int AxisType = AxisValue < 0.0f ? i * 2 : i * 2 + 1;
					Actions.InputEvent(_Input::JOYSTICK_AXIS, AxisType, fabs(AxisValue));
				}
				else {
					Actions.InputEvent(_Input::JOYSTICK_AXIS, i * 2, 0.0f);
					Actions.InputEvent(_Input::JOYSTICK_AXIS, i * 2 + 1, 0.0f);
				}
			}
			
			//for(u32 i = 0; i < Joysticks[JoystickState.Joystick].Axes; i++) {
			//	printf("%f\t", GetAxis(i));
			//}
			//printf("\n");
		} break;
		default:
		break;
	}

	return false;
}

// Set up joysticks
void _Input::InitializeJoysticks(bool ShowLog) {
	
	// Check to see if the real device has been made yet
	IrrlichtDevice *Device = irrDevice;
	if(Device == NULL) {
		SIrrlichtCreationParameters Parameters;
		Parameters.DriverType = EDT_NULL;
		Parameters.LoggingLevel = ELL_ERROR;
		Device = createDeviceEx(Parameters);
	}
	
	// Find joysticks
	if(Device->activateJoysticks(Joysticks) && ShowLog) {
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
	
	// Drop the temporary device
	if(irrDevice == NULL)
		Device->drop();
}

// Return the joystick state
const irr::SEvent::SJoystickEvent &_Input::GetJoystickState() {

	return JoystickState;
}

// Get info about the joystick
const irr::SJoystickInfo &_Input::GetJoystickInfo() {
	return Joysticks[0];
}

// Get a joystick axis value
float _Input::GetAxis(int Axis) {
	if(!HasJoystick())
		return 0.0f;

	float Value = JoystickState.Axis[Axis] / 32767.f;
	if(Value < -1.0f)
		Value = -1.0f;
	if(Value > 1.0f)
		Value = 1.0f;

	if(fabs(Value) <= DeadZone)
		Value = 0.0f;

	return Value;
}

// Use actions to drive the mouse
void _Input::DriveMouse(int Action, float Value) {
	
	if(Action == _Actions::MENU_GO) {
		//printf("%d %f\n", Action, Value);

		SEvent NewEvent;
		NewEvent.UserEvent.UserData1 = 1;
		NewEvent.EventType = EET_MOUSE_INPUT_EVENT;
		NewEvent.MouseInput.X = (int)MouseX;
		NewEvent.MouseInput.Y = (int)MouseY;
		if(Value)
			NewEvent.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
		else
			NewEvent.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
		irrDevice->postEventFromUser(NewEvent);
	}
	
	if(Value == 0.0f)
		return;
		
	//printf("%f %f\n", MouseX, MouseY);
	switch(Action) {
		case _Actions::CURSOR_LEFT:
			MouseX -= Value * Game.GetLastFrameTime();
			VirtualMouseMoved = true;
			irrDevice->getCursorControl()->setPosition((int)MouseX, (int)MouseY);
		break;
		case _Actions::CURSOR_RIGHT:
			MouseX += Value * Game.GetLastFrameTime();
			VirtualMouseMoved = true;
			irrDevice->getCursorControl()->setPosition((int)MouseX, (int)MouseY);
		break;
		case _Actions::CURSOR_UP:
			MouseY -= Value * Game.GetLastFrameTime();
			VirtualMouseMoved = true;
			irrDevice->getCursorControl()->setPosition((int)MouseX, (int)MouseY);
		break;
		case _Actions::CURSOR_DOWN:
			MouseY += Value * Game.GetLastFrameTime();
			VirtualMouseMoved = true;
			irrDevice->getCursorControl()->setPosition((int)MouseX, (int)MouseY);
		break;
	}
}

// Resets the keyboard state
void _Input::ResetInputState() {

	for(int i = 0; i < KEY_KEY_CODES_COUNT; i++)
		Keys[i] = 0;

	Actions.ResetState();
}

// Enables mouse locking
void _Input::SetMouseLocked(bool Value) {
	
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

	Graphics.ToggleCursor(!Value);
}

// Converts an irrlicht key code into a string
const char *_Input::GetKeyName(int Key) {

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
