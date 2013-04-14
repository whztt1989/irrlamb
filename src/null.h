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

// Libraries
#include <engine/state.h>

// Forward Declarations
class _Object;
class _Player;
class _Camera;
struct SaveLevelStruct;

// Classes
class _NullState : public _State {

	public:
	
		int Init();
		int Close();

		void HandleAction(int Action, float Value);
		bool HandleKeyPress(int Key);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void UpdateRender(float TimeStepRemainder);
		void Draw();

	private:
		
};

extern _NullState NullState;
