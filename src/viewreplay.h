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
#ifndef VIEWREPLAY_H
#define VIEWREPLAY_H

// Libraries
#include "engine/state.h"
#include "engine/replay.h"

// Forward Declarations
class ObjectClass;
class PlayerClass;
class CameraClass;

// Classes
class ViewReplayState : public StateClass {

	public:

		enum GUIElements {
			MAIN_RESTART,
			MAIN_PAUSE,
			MAIN_SKIP,
			MAIN_INCREASE,
			MAIN_DECREASE,
			MAIN_EXIT,
		};

		int Init();
		int Close();

		bool HandleKeyPress(int Key);
		void HandleMouseWheel(float Direction);
		void HandleGUI(int EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void Draw();
		
		void SetCurrentReplay(const std::string &File) { CurrentReplay = File; }

		static ViewReplayState *Instance() {
			static ViewReplayState ClassInstance;
			return &ClassInstance;
		}

	private:

		void SetupGUI();
		void ChangeReplaySpeed(float Amount);
		void Pause();
		void Skip(float Amount);

		// States
		std::string CurrentReplay;
		float Timer;

		// Objects
		CameraClass *Camera;
	
		// Replay information
		ReplayEventStruct NextEvent;
		float ReplaySpeed, PauseSpeed;

		// Events
		int NextPacketType;
};

#endif
