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
#include <engine/actions.h>

// Forward Declarations
struct SaveLevelStruct;

// Classes
class _Menu {

	public:

		enum MenuType {
			STATE_NONE,
			STATE_MAIN,
			STATE_SINGLEPLAYER,
			STATE_LEVELS,
			STATE_REPLAYS,
			STATE_OPTIONS,
			STATE_VIDEO,
			STATE_AUDIO,
			STATE_CONTROLS,
			STATE_PAUSED,
			STATE_SAVEREPLAY,
			STATE_LOSE,
			STATE_WIN
		};

		void InitMain();
		void InitSinglePlayer();
		void InitLevels();
		void InitReplays();
		void InitOptions();
		void InitVideo();
		void InitAudio();
		void InitControls();
		
		void InitPlay();
		void InitLose();
		void InitWin();
		void InitPause();
		void InitSaveReplay();
		void SaveReplay();

		bool HandleKeyPress(int Key);
		void HandleAction(int Action, float Value);
		void HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, irr::gui::IGUIElement *Element);

		void Update(float FrameTime);
		void Draw();

		void DrawWinScreen();

	private:

		void CancelKeyBind();
		std::string GetReplayFile();
		void LaunchReplay();
		void LaunchLevel();

		MenuType State;
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
		static const wchar_t *ActionNames[_Actions::COUNT];
		int CurrentKeys[_Actions::COUNT];
		irr::gui::IGUIButton *KeyButton;
		std::wstring KeyButtonOldText;

};

extern _Menu Menu;
