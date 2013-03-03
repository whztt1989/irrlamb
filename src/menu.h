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
#ifndef MENU_H
#define MENU_H

// Libraries
#include "engine/state.h"
#include "engine/actiontypes.h"
#include <vector>
#include <string>

// Forward Declarations
struct SaveLevelStruct;

// Classes
class MenuState : public StateClass {

	public:

		enum GUIElements {
			MAIN_SINGLEPLAYER,
			MAIN_REPLAYS,
			MAIN_OPTIONS,
			MAIN_QUIT,
			SINGLEPLAYER_BACK,
			LEVELS_GO,
			LEVELS_BUY,
			LEVELS_HIGHSCORES,
			LEVELS_BACK,
			LEVELS_SELECTEDLEVEL,
			LEVELINFO_DESCRIPTION,
			LEVELINFO_ATTEMPTS,
			LEVELINFO_WINS,
			LEVELINFO_LOSSES,
			LEVELINFO_PLAYTIME,
			LEVELINFO_BESTTIME,
			REPLAYS_FILES,
			REPLAYS_GO,
			REPLAYS_DELETE,
			REPLAYS_BACK,
			OPTIONS_VIDEO,
			OPTIONS_AUDIO,
			OPTIONS_CONTROLS,
			OPTIONS_BACK,
			VIDEO_SAVE,
			VIDEO_CANCEL,
			VIDEO_VIDEOMODES,
			VIDEO_FULLSCREEN,
			VIDEO_SHADOWS,
			VIDEO_SHADERS,
			AUDIO_ENABLED,
			AUDIO_SAVE,
			AUDIO_CANCEL,
			CONTROLS_SAVE,
			CONTROLS_CANCEL,
			CONTROLS_MOVEFORWARD,
			CONTROLS_MOVEBACK,
			CONTROLS_MOVELEFT,
			CONTROLS_MOVERIGHT,
			CONTROLS_MOVERESET,
			CONTROLS_MOVEJUMP,
		};

		enum MenuStateType {
			STATE_NONE,
			STATE_INITMAIN,
			STATE_MAIN,
			STATE_INITSINGLEPLAYER,
			STATE_SINGLEPLAYER,
			STATE_INITLEVELS,
			STATE_LEVELS,
			STATE_INITREPLAYS,
			STATE_REPLAYS,
			STATE_INITOPTIONS,
			STATE_OPTIONS,
			STATE_INITVIDEO,
			STATE_VIDEO,
			STATE_INITAUDIO,
			STATE_AUDIO,
			STATE_INITCONTROLS,
			STATE_CONTROLS,
		};

		MenuState() : TargetState(STATE_NONE) { }

		int Init();
		int Close();

		bool HandleKeyPress(int Key);
		void HandleGUI(int EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void Draw();

		void SetTargetState(MenuStateType NewState) { TargetState = NewState; }
		
		static MenuState *Instance() {
			static MenuState ClassInstance;
			return &ClassInstance;
		}

	private:

		void CancelKeyBind();
		std::string GetReplayFile();
		void LaunchReplay();
		void LaunchLevel();

		MenuStateType State, TargetState;
		bool FirstStateLoad;
		std::string MenuMessage;

		// Replays
		std::vector<std::string> ReplayFiles;
		
		// Campaigns
		int CampaignIndex, SelectedLevel;
		float DoubleClickTimer;

		// Level info
		std::vector<const SaveLevelStruct *> LevelStats;

		// Key mapping
		static const wchar_t *ActionNames[ACTIONS::COUNT];
		int CurrentKeys[ACTIONS::COUNT];
		irr::gui::IGUIButton *KeyButton;
		std::wstring KeyButtonOldText;

};

#endif
