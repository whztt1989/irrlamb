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
#include <test.h>
#include <engine/globals.h>
#include <engine/input.h>
#include <engine/graphics.h>
#include <engine/config.h>
#include <engine/physics.h>
#include <engine/interface.h>
#include <engine/game.h>
#include <engine/namespace.h>
#include <ui.h>

_TestState TestState;

// Initializes the state
int _TestState::Init() {

	UI.InitTitleScreen();

	return 1;
}

// Shuts the state down
int _TestState::Close() {

	return 1;
}

// Handle new actions
void _TestState::HandleAction(int Action, float Value) {
	
}

// Key presses
bool _TestState::HandleKeyPress(int Key) {
	if(Key == 27)
		Game.SetDone(true);
	
	return true;
}

// Mouse buttons
bool _TestState::HandleMousePress(int Button, int MouseX, int MouseY) {

	return false;
}

// GUI events
void _TestState::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, IGUIElement *Element) {
	
	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			//Interface.PlaySound(_Interface::SOUND_CONFIRM);
		break;
	}
}

// Updates the current state
void _TestState::Update(float FrameTime) {

}

// Interpolate object positions
void _TestState::UpdateRender(float TimeStepRemainder) {
	
}

// Draws the current state
void _TestState::Draw() {
	irrGUI->drawAll();
}
