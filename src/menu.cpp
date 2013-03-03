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
#include "menu.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/constants.h"
#include "engine/interface.h"
#include "engine/globals.h"
#include "engine/config.h"
#include "engine/game.h"
#include "engine/save.h"
#include "engine/campaign.h"
#include "viewreplay.h"
#include "play.h"
#include "engine/namespace.h"

const int CAMPAIGN_LEVELID = 1000;
const int PLAY_CAMPAIGNID = 900;

// List of action names
const wchar_t *MenuState::ActionNames[ACTIONS::COUNT] = {
	L"Move Forward",
	L"Move Back",
	L"Move Left",
	L"Move Right",
	L"Restart Level",
	L"Jump"
};

// Initializes the state
int MenuState::Init() {

	Interface::Instance().ChangeSkin(InterfaceClass::SKIN_MENU);
	Input::Instance().SetMouseLocked(false);

	State = STATE_INITMAIN;
	FirstStateLoad = true;
	if(TargetState != STATE_NONE) {
		State = TargetState;
		TargetState = STATE_NONE;
	}

	return 1;
}

// Shuts the state down
int MenuState::Close() {

	return 1;
}

// Key presses
bool MenuState::HandleKeyPress(int Key) {

	bool Processed = true;
	switch(State) {
		case STATE_MAIN:
			switch(Key) {
				case KEY_ESCAPE:
					Game::Instance().SetDone(true);
				break;
				case KEY_RETURN:
					State = STATE_INITSINGLEPLAYER;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_SINGLEPLAYER:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITMAIN;
				break;
				case KEY_RETURN:
					State = STATE_INITLEVELS;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_LEVELS:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITSINGLEPLAYER;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_REPLAYS:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITMAIN;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_OPTIONS:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITMAIN;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_VIDEO:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITOPTIONS;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_AUDIO:
			switch(Key) {
				case KEY_ESCAPE:
					State = STATE_INITOPTIONS;
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_CONTROLS:
			if(KeyButton != NULL) {
				stringw KeyName = Input::Instance().GetKeyName(Key);

				// Assign the key
				if(Key != KEY_ESCAPE && KeyName != "") {
					
					int ActionType = KeyButton->getID() - CONTROLS_MOVEFORWARD;

					// Swap if the key already exists
					for(int i = 0; i < ACTIONS::COUNT; i++) {
						if(CurrentKeys[i] == Key) {
							
							// Get button
							IGUIButton *SwapButton = static_cast<IGUIButton *>(irrGUI->getRootGUIElement()->getElementFromId(CONTROLS_MOVEFORWARD + i));

							// Swap text
							SwapButton->setText(stringw(Input::Instance().GetKeyName(CurrentKeys[ActionType])).c_str());
							CurrentKeys[i] = CurrentKeys[ActionType];
							break;
						}
					}

					// Update key
					KeyButton->setText(KeyName.c_str());
					CurrentKeys[ActionType] = Key;
				}
				else
					KeyButton->setText(KeyButtonOldText.c_str());

				KeyButton = NULL;
			}
			else {
				switch(Key) {
					case KEY_ESCAPE:
						State = STATE_INITOPTIONS;
					break;
					default:
						Processed = false;
					break;
				}
			}
		break;
	}

	return Processed;
}

// Handles GUI events
void MenuState::HandleGUI(int EventType, IGUIElement *Element) {

	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_SINGLEPLAYER:
					State = STATE_INITSINGLEPLAYER;
				break;
				case MAIN_REPLAYS:
					State = STATE_INITREPLAYS;
				break;
				case MAIN_OPTIONS:
					State = STATE_INITOPTIONS;
				break;
				case MAIN_QUIT:
					Game::Instance().SetDone(true);
				break;
				case SINGLEPLAYER_BACK:
					State = STATE_INITMAIN;
				break;
				case LEVELS_GO:
					LaunchLevel();
				break;
				case LEVELS_BACK:
					State = STATE_INITSINGLEPLAYER;
				break;
				case REPLAYS_GO:
					LaunchReplay();
				break;
				case REPLAYS_DELETE: {

					// Get list
					IGUIListBox *ReplayList = static_cast<IGUIListBox *>(irrGUI->getRootGUIElement()->getElementFromId(REPLAYS_FILES));
					int SelectedIndex = ReplayList->getSelected();
					if(SelectedIndex != -1) {

						// Remove file
						std::string FilePath = Save::Instance().GetReplayPath() + GetReplayFile();
						remove(FilePath.c_str());

						// Remove entry
						ReplayList->removeItem(SelectedIndex);
					}
				}
				break;
				case REPLAYS_BACK:
					State = STATE_INITMAIN;
				break;
				case OPTIONS_VIDEO:
					State = STATE_INITVIDEO;
				break;
				case OPTIONS_AUDIO:
					State = STATE_INITAUDIO;
				break;
				case OPTIONS_CONTROLS:
					State = STATE_INITCONTROLS;
				break;
				case OPTIONS_BACK:
					State = STATE_INITMAIN;
				break;
				case VIDEO_SAVE: {
					
					// Save the video mode
					IGUIComboBox *VideoModes = static_cast<IGUIComboBox *>(irrGUI->getRootGUIElement()->getElementFromId(VIDEO_VIDEOMODES));
					if(VideoModes != NULL) {
						VideoModeStruct Mode = Graphics::Instance().GetVideoModes()[VideoModes->getSelected()];
						Config::Instance().ScreenWidth = Mode.Width;
						Config::Instance().ScreenHeight = Mode.Height;
					}

					// Save full screen
					IGUICheckBox *Fullscreen = static_cast<IGUICheckBox *>(irrGUI->getRootGUIElement()->getElementFromId(VIDEO_FULLSCREEN));
					Config::Instance().Fullscreen = Fullscreen->isChecked();

					// Save shadows
					IGUICheckBox *Shadows = static_cast<IGUICheckBox *>(irrGUI->getRootGUIElement()->getElementFromId(VIDEO_SHADOWS));
					Config::Instance().Shadows = Shadows->isChecked();

					/*// Save shaders
					IGUICheckBox *Shaders = static_cast<IGUICheckBox *>(irrGUI->getRootGUIElement()->getElementFromId(VIDEO_SHADERS));
					Config::Instance().Shaders = Shaders->isChecked();
					 */

					// Write config
					Config::Instance().WriteConfig();

					State = STATE_INITOPTIONS;
				}
				break;
				case VIDEO_CANCEL:
					State = STATE_INITOPTIONS;
				break;
				case AUDIO_SAVE: {
					bool OldAudioEnabled = Config::Instance().AudioEnabled;

					// Get settings
					IGUICheckBox *AudioEnabled = static_cast<IGUICheckBox *>(irrGUI->getRootGUIElement()->getElementFromId(AUDIO_ENABLED));
					bool Enabled = AudioEnabled->isChecked();

					// Save
					Config::Instance().AudioEnabled = Enabled;
					Config::Instance().WriteConfig();

					// Init or disable audio system
					if(OldAudioEnabled != Enabled) {
						if(Enabled) {
							Game::Instance().EnableAudio();
						}
						else
							Game::Instance().DisableAudio();
					}

					State = STATE_INITOPTIONS;
				}
				break;
				case AUDIO_CANCEL:
					State = STATE_INITOPTIONS;
				break;
				case CONTROLS_SAVE: {

					// Write config
					for(int i = 0; i < ACTIONS::COUNT; i++)
						Config::Instance().Keys[i] = CurrentKeys[i];

					Config::Instance().WriteConfig();

					State = STATE_INITOPTIONS;
				}
				break;
				case CONTROLS_CANCEL:
					State = STATE_INITOPTIONS;
				break;
				case CONTROLS_MOVEFORWARD:
				case CONTROLS_MOVEBACK:
				case CONTROLS_MOVELEFT:
				case CONTROLS_MOVERIGHT:
				case CONTROLS_MOVERESET:
				case CONTROLS_MOVEJUMP:	{
					if(KeyButton)
						CancelKeyBind();

					KeyButton = static_cast<IGUIButton *>(Element);
					KeyButtonOldText = KeyButton->getText();
					KeyButton->setText(L"");
				}
				break;
				default: {

					if(Element->getID() >= CAMPAIGN_LEVELID) {
						SelectedLevel = Element->getID() - CAMPAIGN_LEVELID;
						LaunchLevel();
					}
					else if(Element->getID() >= PLAY_CAMPAIGNID) {
						CampaignIndex = Element->getID() - PLAY_CAMPAIGNID;
						State = STATE_INITLEVELS;
					}
				}
				break;
			}
		break;
		case EGET_LISTBOX_SELECTED_AGAIN:
			switch(Element->getID()) {
				case REPLAYS_FILES:
					LaunchReplay();
				break;
			}
		break;
		case EGET_ELEMENT_HOVERED:
			if(Element->getID() >= CAMPAIGN_LEVELID) {
				SelectedLevel = Element->getID() - CAMPAIGN_LEVELID;
			}
		break;
		case EGET_ELEMENT_LEFT:
			if(State == STATE_LEVELS)
				SelectedLevel = -1;
		break;
	}
}

// Updates the current state
void MenuState::Update(float FrameTime) {
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2, X, Y;
	switch(State) {
		case STATE_INITMAIN: {
			Interface::Instance().Clear();

			// Logo
			irrGUI->addImage(irrDriver->getTexture("art/logo.jpg"), position2di(CenterX - 256, CenterY - 215));
			IGUIStaticText *TextVersion = irrGUI->addStaticText(stringw(GAME_VERSION).c_str(), Interface::Instance().GetCenteredRect(40, irrDriver->getScreenSize().Height - 20, 50, 15), false, false);

			// Button
			int Y = CenterY - 50;
			IGUIButton *ButtonSinglePlayer = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, Y, 130, 34), 0, MAIN_SINGLEPLAYER, L"Single Player");
			IGUIButton *ButtonReplays = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, Y + 50, 130, 34), 0, MAIN_REPLAYS, L"Replays");
			IGUIButton *ButtonOptions = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, Y + 100, 130, 34), 0, MAIN_OPTIONS, L"Options");
			IGUIButton *ButtonQuit = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, Y + 150, 130, 34), 0, MAIN_QUIT, L"Quit");
			ButtonSinglePlayer->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonSinglePlayer->setUseAlphaChannel(true);
			ButtonSinglePlayer->setDrawBorder(false);
			ButtonReplays->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonReplays->setUseAlphaChannel(true);
			ButtonReplays->setDrawBorder(false);
			ButtonOptions->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonOptions->setUseAlphaChannel(true);
			ButtonOptions->setDrawBorder(false);
			ButtonQuit->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonQuit->setUseAlphaChannel(true);
			ButtonQuit->setDrawBorder(false);

			// Play sound
			if(!FirstStateLoad)
				Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);
			FirstStateLoad = false;

			State = STATE_MAIN;
		}
		break;
		case STATE_MAIN:
		break;
		case STATE_INITSINGLEPLAYER: {
			Interface::Instance().Clear();

			// Reset menu variables
			CampaignIndex = 0;
			SelectedLevel = -1;

			// Text
			X = CenterX, Y = CenterY - 150;
			IGUIStaticText *Text = irrGUI->addStaticText(L"Level Sets", Interface::Instance().GetCenteredRect(X, Y, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Campaigns
			Y += 50;
			const std::vector<CampaignStruct> &Campaigns = Campaign::Instance().GetCampaigns();
			for(u32 i = 0; i < Campaigns.size(); i++) {
				irr::core::stringw Name(Campaigns[i].Name.c_str());
				IGUIButton *Button = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 130, 34), 0, PLAY_CAMPAIGNID + i, Name.c_str());
				Button->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
				Button->setUseAlphaChannel(true);
				Button->setDrawBorder(false);

				Y += 40;
			}

			Y += 50;
			IGUIButton *BackButton = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 130, 34), 0, SINGLEPLAYER_BACK, L"Back");
			BackButton->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			BackButton->setUseAlphaChannel(true);
			BackButton->setDrawBorder(false);

			// Play sound
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

			State = STATE_SINGLEPLAYER;
		}
		break;
		case STATE_SINGLEPLAYER:
		break;
		case STATE_INITLEVELS: {
			Interface::Instance().Clear();
			LevelStats.clear();
			SelectedLevel = -1;
			X = CenterX, Y = CenterY - 190;

			// Text
			IGUIStaticText *Text = irrGUI->addStaticText(L"Levels", Interface::Instance().GetCenteredRect(X, Y, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Add level list
			X = CenterX - 160;
			Y += 60;
			int Column = 0, Row = 0;
			const CampaignStruct &Campaign = Campaign::Instance().GetCampaign(CampaignIndex);
			for(u32 i = 0; i < Campaign.Levels.size(); i++) {
				bool Unlocked = true;
				
				// Get level stats
				const SaveLevelStruct *Stats = Save::Instance().GetLevelStats(Campaign.Levels[i].File);
				LevelStats.push_back(Stats);

				// Set unlocked status
				if(Stats->Unlocked == 0) {
					Unlocked = false;

					// Unlock the level if it's always unlocked in the campaign
					if(Campaign.Levels[i].Unlocked) {
						Save::Instance().UnlockLevel(Campaign.Levels[i].File);
						Unlocked = true;
					}
				}				

				// Add button
				IGUIButton *Level = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + Column * 80, Y + Row * 80, 64, 64), 0, CAMPAIGN_LEVELID + i);

				// Set thumbnail
				if(Unlocked)
					Level->setImage(irrDriver->getTexture((Campaign.Levels[i].DataPath + "icon.jpg").c_str()));
				else
					Level->setImage(irrDriver->getTexture("art/locked.png"));

				Column++;
				if(Column >= 5) {
					Column = 0;
					Row++;
				}
			}

			// Buttons
			X = CenterX;
			Y = CenterY + 180;
			IGUIButton *ButtonBack = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 82, 34), 0, LEVELS_BACK, L"Back");
			ButtonBack->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonBack->setUseAlphaChannel(true);
			ButtonBack->setDrawBorder(false);

			// Play sound
			if(!FirstStateLoad)
				Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);
			FirstStateLoad = false;

			State = STATE_LEVELS;
		}
		break;
		case STATE_LEVELS:
		break;
		case STATE_INITREPLAYS: {
			Interface::Instance().Clear();
			char Buffer[256];

			// Text
			X = CenterX, Y = CenterY - 180;
			IGUIStaticText *Text = irrGUI->addStaticText(L"Replays", Interface::Instance().GetCenteredRect(X, Y, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Level selection
			Y += 160;
			IGUIListBox *ListReplays = irrGUI->addListBox(Interface::Instance().GetCenteredRect(X, Y, 450, 250), 0, REPLAYS_FILES, true);

			// Change directories
			std::string OldWorkingDirectory(irrFile->getWorkingDirectory().c_str());
			irrFile->changeWorkingDirectoryTo(Save::Instance().GetReplayPath().c_str());

			// Get a list of replays
			IFileList *FileList = irrFile->createFileList();
			u32 FileCount = FileList->getFileCount();
			ReplayFiles.clear();
			for(u32 i = 0; i < FileCount; i++) {
				if(!FileList->isDirectory(i) && FileList->getFileName(i).find(".replay") != -1) {
					ReplayFiles.push_back(FileList->getFileName(i).c_str());
				}
			}
			irrFile->changeWorkingDirectoryTo(OldWorkingDirectory.c_str());

			// Add replays to menu list
			for(u32 i = 0; i < ReplayFiles.size(); i++) {
				bool Loaded = Replay::Instance().LoadReplay(ReplayFiles[i].c_str(), true);
				if(Loaded && Replay::Instance().GetVersion() == REPLAY_VERSION) {

					// Get time string
					Interface::Instance().ConvertSecondsToString(Replay::Instance().GetFinishTime(), Buffer);

					// Build replay string
					std::string ReplayInfo = ReplayFiles[i] + std::string(" - ") + Replay::Instance().GetLevelName()
											+ std::string(" - ") + Replay::Instance().GetDescription() + std::string(" - ") + Buffer;

					irr::core::stringw ReplayString(ReplayInfo.c_str());
					ListReplays->addItem(ReplayString.c_str());
				}
			}

			// Confirmations
			Y += 160;
			IGUIButton *ButtonGo = irrGUI->addButton(Interface::Instance().GetCenteredRect(X - 123, Y, 102, 34), 0, REPLAYS_GO, L"View");
			IGUIButton *ButtonDelete = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 102, 34), 0, REPLAYS_DELETE, L"Delete");
			IGUIButton *ButtonBack = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 123, Y, 102, 34), 0, REPLAYS_BACK, L"Back");
			ButtonGo->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
			ButtonGo->setUseAlphaChannel(true);
			ButtonGo->setDrawBorder(false);
			ButtonDelete->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
			ButtonDelete->setUseAlphaChannel(true);
			ButtonDelete->setDrawBorder(false);
			ButtonBack->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
			ButtonBack->setUseAlphaChannel(true);
			ButtonBack->setDrawBorder(false);

			// Play sound
			if(!FirstStateLoad)
				Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);
			FirstStateLoad = false;

			State = STATE_REPLAYS;
		}
		break;
		case STATE_REPLAYS:
		break;
		case STATE_INITOPTIONS:	{
			Interface::Instance().Clear();
			
			// Text
			IGUIStaticText *Text = irrGUI->addStaticText(L"Options", Interface::Instance().GetCenteredRect(CenterX, CenterY - 120, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Buttons
			IGUIButton *ButtonVideo = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY - 50, 130, 34), 0, OPTIONS_VIDEO, L"Video");
			IGUIButton *ButtonAudio = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY, 130, 34), 0, OPTIONS_AUDIO, L"Audio");
			IGUIButton *ButtonControls = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY + 50, 130, 34), 0, OPTIONS_CONTROLS, L"Controls");
			IGUIButton *ButtonBack = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY + 100, 130, 34), 0, OPTIONS_BACK, L"Back");
			ButtonVideo->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonVideo->setUseAlphaChannel(true);
			ButtonVideo->setDrawBorder(false);
			ButtonAudio->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonAudio->setUseAlphaChannel(true);
			ButtonAudio->setDrawBorder(false);
			ButtonControls->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonControls->setUseAlphaChannel(true);
			ButtonControls->setDrawBorder(false);
			ButtonBack->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
			ButtonBack->setUseAlphaChannel(true);
			ButtonBack->setDrawBorder(false);

			// Play sound
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

			State = STATE_OPTIONS;
		}
		break;
		case STATE_OPTIONS:
		break;
		case STATE_INITVIDEO: {
			Interface::Instance().Clear();

			// Text
			X = CenterX, Y = CenterY - 150;
			IGUIStaticText *Text = irrGUI->addStaticText(L"Video", Interface::Instance().GetCenteredRect(X, Y, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Video modes
			Y += 40;
			const std::vector<VideoModeStruct> &ModeList = Graphics::Instance().GetVideoModes();
			if(ModeList.size() > 0) {
				IGUIStaticText *TextScreenResolution = irrGUI->addStaticText(L"Screen Resolution", Interface::Instance().GetCenteredRect(X - 65, Y, 110, 25));
				TextScreenResolution->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_CENTER);
				IGUIComboBox *ListScreenResolution = irrGUI->addComboBox(Interface::Instance().GetCenteredRect(X + 60, Y, 100, 25), 0, VIDEO_VIDEOMODES);

				// Populate mode list
				for(u32 i = 0; i < ModeList.size(); i++)
					ListScreenResolution->addItem(ModeList[i].String.c_str());
				ListScreenResolution->setSelected(Graphics::Instance().GetCurrentVideoModeIndex());
			}
				
			// Full Screen
			Y += 30;
			IGUIStaticText *TextFullscreen = irrGUI->addStaticText(L"Fullscreen", Interface::Instance().GetCenteredRect(X - 65, Y, 110, 25));
			TextFullscreen->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_CENTER);
			IGUICheckBox *CheckBoxFullscreen = irrGUI->addCheckBox(Config::Instance().Fullscreen, Interface::Instance().GetCenteredRect(X + 60, Y, 100, 25), 0, VIDEO_FULLSCREEN);

			// Shadows
			Y += 30;
			IGUIStaticText *TextShadows = irrGUI->addStaticText(L"Shadows", Interface::Instance().GetCenteredRect(X - 65, Y, 110, 25), false, false);
			TextShadows->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_CENTER);
			IGUICheckBox *CheckBoxShadows = irrGUI->addCheckBox(Config::Instance().Shadows, Interface::Instance().GetCenteredRect(X + 60, Y, 100, 25), 0, VIDEO_SHADOWS);
/*
			// Shaders
			Y += 30;
			IGUIStaticText *TextShaders = irrGUI->addStaticText(L"Shaders", Interface::Instance().GetCenteredRect(X - 65, Y, 110, 25), false, false);
			TextShaders->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_CENTER);
			IGUICheckBox *CheckBoxShaders = irrGUI->addCheckBox(Config::Instance().Shaders, Interface::Instance().GetCenteredRect(X + 60, Y, 100, 25), 0, VIDEO_SHADERS);
			if(!Graphics::Instance().GetShadersSupported())
				CheckBoxShaders->setEnabled(false);
*/
			// Save
			Y += 60;
			IGUIButton *ButtonSave = irrGUI->addButton(Interface::Instance().GetCenteredRect(X - 50, Y, 82, 34), 0, VIDEO_SAVE, L"Save");
			IGUIButton *ButtonCancel = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 50, Y, 82, 34), 0, VIDEO_CANCEL, L"Cancel");
			ButtonSave->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonSave->setUseAlphaChannel(true);
			ButtonSave->setDrawBorder(false);
			ButtonCancel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonCancel->setUseAlphaChannel(true);
			ButtonCancel->setDrawBorder(false);

			// Warning
			Y += 40;
			IGUIStaticText *TextWarning = irrGUI->addStaticText(L"Some changes are applied after restart", Interface::Instance().GetCenteredRect(X, Y, 250, 25), false, false, 0, -1, true);
			TextWarning->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

			// Play sound
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

			State = STATE_VIDEO;
		}
		break;
		case STATE_VIDEO:
		break;
		case STATE_INITAUDIO: {
			Interface::Instance().Clear();

			// Text
			X = CenterX, Y = CenterY - 150;
			IGUIStaticText *Text = irrGUI->addStaticText(L"Audio", Interface::Instance().GetCenteredRect(X, Y, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Sound enabled
			Y += 60;
			IGUIStaticText *TextFullscreen = irrGUI->addStaticText(L"Audio Enabled", Interface::Instance().GetCenteredRect(X - 65, Y, 110, 25));
			TextFullscreen->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_CENTER);
			IGUICheckBox *CheckBoxFullscreen = irrGUI->addCheckBox(Config::Instance().AudioEnabled, Interface::Instance().GetCenteredRect(X + 60, Y, 100, 25), 0, AUDIO_ENABLED);

			// Save
			Y += 90;
			IGUIButton *ButtonSave = irrGUI->addButton(Interface::Instance().GetCenteredRect(X - 50, Y, 82, 34), 0, AUDIO_SAVE, L"Save");
			IGUIButton *ButtonCancel = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 50, Y, 82, 34), 0, AUDIO_CANCEL, L"Cancel");
			ButtonSave->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonSave->setUseAlphaChannel(true);
			ButtonSave->setDrawBorder(false);
			ButtonCancel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonCancel->setUseAlphaChannel(true);
			ButtonCancel->setDrawBorder(false);

			// Play sound
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

			State = STATE_AUDIO;
		}
		break;
		case STATE_AUDIO:
		break;
		case STATE_INITCONTROLS: {
			Interface::Instance().Clear();
			KeyButton = NULL;

			// Text
			IGUIStaticText *Text = irrGUI->addStaticText(L"Controls", Interface::Instance().GetCenteredRect(CenterX, CenterY - 160, 150, 40), false, false);
			Text->setOverrideFont(Interface::Instance().GetFont(InterfaceClass::FONT_LARGE));
			Text->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);

			// Create the key buttons
			X = CenterX;
			Y = CenterY - 110;
			for(int i = 0; i < ACTIONS::COUNT; i++) {
				
				CurrentKeys[i] = Config::Instance().Keys[i];
				IGUIStaticText *Text = irrGUI->addStaticText(ActionNames[i], Interface::Instance().GetCenteredRect(X - 50, Y, 80, 20), false, false);
				Text->setTextAlignment(EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT);
				IGUIButton *Button = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 50, Y, 82, 34), 0, CONTROLS_MOVEFORWARD + i, stringw(Input::Instance().GetKeyName(CurrentKeys[i])).c_str());
				Button->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
				Button->setUseAlphaChannel(true);
				Button->setDrawBorder(false);

				Y += 35;
			}

			// Save
			IGUIButton *ButtonSave = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX - 50, CenterY + 150, 82, 34), 0, CONTROLS_SAVE, L"Save");
			IGUIButton *ButtonCancel = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX + 50, CenterY + 150, 82, 34), 0, CONTROLS_CANCEL, L"Cancel");
			ButtonSave->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonSave->setUseAlphaChannel(true);
			ButtonCancel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
			ButtonCancel->setUseAlphaChannel(true);
			ButtonCancel->setDrawBorder(false);
	
			// Play sound
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

			State = STATE_CONTROLS;
		}
		break;
		case STATE_CONTROLS:
		break;
	}
}

// Draws the current state
void MenuState::Draw() {
	irrGUI->drawAll();

	// Draw level tooltip
	if(State == STATE_LEVELS) {

		if(SelectedLevel != -1) {
			char Buffer[256];
			const SaveLevelStruct *Stats = LevelStats[SelectedLevel];

			// Get box position
			int Width = 210, Height = 285, X, Y;
			int Left = Input::Instance().GetMouseX() + 20;
			int Top = Input::Instance().GetMouseY() - 105;

			// Cap limits
			if(Top < 10)
				Top = 10;
			if(Left + Width > (int)irrDriver->getScreenSize().Width - 10)
				Left -= Width + 35;

			// Draw box
			Interface::Instance().DrawTextBox(Left + Width/2, Top + Height/2, Width, Height);
			X = Left + Width/2;
			Y = Top + 17;

			if(Stats->Unlocked) {

				// Load count
				Interface::Instance().RenderText("Play time", X - 10, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
				Interface::Instance().ConvertSecondsToString(Stats->PlayTime, Buffer);
				Interface::Instance().RenderText(Buffer, X + 10, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));

				// Load count
				Y += 17;
				Interface::Instance().RenderText("Plays", X - 10, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
				sprintf(Buffer, "%d", Stats->LoadCount);
				Interface::Instance().RenderText(Buffer, X + 10, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));

				// Win count
				Y += 17;
				Interface::Instance().RenderText("Wins", X - 10, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
				sprintf(Buffer, "%d", Stats->WinCount);
				Interface::Instance().RenderText(Buffer, X + 10, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));

				// Scores
				if(Stats->HighScores.size() > 0) {

					// High scores
					int HighX = Left + Width/2 - 80, HighY = Y + 28;

					// Draw header
					Interface::Instance().RenderText("#", HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
					Interface::Instance().RenderText("Time", HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
					Interface::Instance().RenderText("Date", HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
					HighY += 17;

					for(size_t i = 0; i < Stats->HighScores.size(); i++) {
						
						// Number
						char SmallBuffer[32];
						sprintf(SmallBuffer, "%d", i+1);
						Interface::Instance().RenderText(SmallBuffer, HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));

						// Time
						Interface::Instance().ConvertSecondsToString(Stats->HighScores[i].Time, Buffer);
						Interface::Instance().RenderText(Buffer, HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));

						// Date
						char DateString[32];
						strftime(DateString, 32, "%m-%d-%Y", localtime(&Stats->HighScores[i].DateStamp));
						Interface::Instance().RenderText(DateString, HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

						HighY += 17;
					}
				}
			}
			else {
				
				// Locked
				Interface::Instance().RenderText("Level Locked", X, Y, InterfaceClass::ALIGN_CENTER, InterfaceClass::FONT_MEDIUM, SColor(255, 255, 255, 255));
			}
		}
	}
}

// Cancels the key bind state
void MenuState::CancelKeyBind() {
	KeyButton->setText(KeyButtonOldText.c_str());
	KeyButton = NULL;
}

// Gets the replay name from a selection box
std::string MenuState::GetReplayFile() {

	// Get list
	IGUIListBox *ReplayList = static_cast<IGUIListBox *>(irrGUI->getRootGUIElement()->getElementFromId(REPLAYS_FILES));
	
	int SelectedIndex = ReplayList->getSelected();
	if(SelectedIndex != -1) {
		return ReplayFiles[SelectedIndex];
	}

	return "";
}

// Launchs a level
void MenuState::LaunchLevel() {
	
	SaveLevelStruct Stats;
	Save::Instance().GetLevelStats(Campaign::Instance().GetCampaign(CampaignIndex).Levels[SelectedLevel].File, Stats);
	if(Stats.Unlocked == 0)
		return;

	PlayState::Instance()->SetTestLevel("");
	PlayState::Instance()->SetCampaign(CampaignIndex);
	PlayState::Instance()->SetCampaignLevel(SelectedLevel);
	Game::Instance().ChangeState(PlayState::Instance());
}

// Launchs a replay from a list item
void MenuState::LaunchReplay() {

	// Get replay file
	std::string File = GetReplayFile();
	if(File != "") {

		// Load replay
		ViewReplayState::Instance()->SetCurrentReplay(File);
		Game::Instance().ChangeState(ViewReplayState::Instance());
	}
}
