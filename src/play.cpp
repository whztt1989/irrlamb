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
#include "play.h"
#include "engine/constants.h"
#include "engine/globals.h"
#include "engine/input.h"
#include "engine/log.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "engine/config.h"
#include "engine/physics.h"
#include "engine/scripting.h"
#include "engine/objectmanager.h"
#include "engine/replay.h"
#include "engine/interface.h"
#include "engine/camera.h"
#include "engine/game.h"
#include "engine/level.h"
#include "engine/campaign.h"
#include "engine/fader.h"
#include "engine/actions.h"
#include "engine/save.h"
#include "objects/player.h"
#include "menu.h"
#include "viewreplay.h"
#include "engine/namespace.h"
#include <IGUIEditBox.h>

_PlayState PlayState;

const int WIN_WIDTH = 430;
const int WIN_HEIGHT = 350;

PlayerClass *GlobalPlayer = 0;

// Initializes the state
int _PlayState::Init() {
	Player = NULL;
	Timer = 0.0f;
	Resetting = false;
	WinStats = NULL;
	ShowHUD = true;
	Physics.SetEnabled(true);
	Interface.ChangeSkin(InterfaceClass::SKIN_GAME);

	// Set up mapping
	Actions.ClearMappings();
	for(int i = 0; i < _Actions::COUNT; i++)
		Actions.AddKeyMap(Config.Keys[i], i);

	// Add camera
	Camera = new CameraClass();

	// Load the level
	std::string LevelFile;
	
	// Select a level to load
	if(TestLevel != "")
		LevelFile = TestLevel;
	else
		LevelFile = Campaign.GetLevel(CurrentCampaign, CampaignLevel);

	// Load level
	if(!Level.Init(LevelFile))
		return 0;

	// Reset level
	ResetLevel();

	return 1;
}

// Shuts the state down
int _PlayState::Close() {

	// Stop the replay
	Replay.StopRecording();

	// Close the system down
	delete Camera;
	Level.Close();
	ObjectManager.ClearObjects();
	Interface.Clear();
	irrScene->clear();

	// Save stats
	if(TestLevel == "") {
		Save.IncrementLevelPlayTime(Level.GetLevelName(), Timer);
		Save.SaveLevelStats(Level.GetLevelName());
	}

	return 1;
}

// Key presses
bool _PlayState::HandleKeyPress(int Key) {
	if(Resetting)
		return true;

	bool Processed = true, LuaProcessed = false;
	
	switch(State) {
		case STATE_PLAY:
			if(Actions.GetState(_Actions::RESET))
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					if(TestLevel != "")
						Game.SetDone(true);
					else
						InitPause();
				break;
				case KEY_F1:
					InitPause();
				break;
				case KEY_F2:
					Config.InvertMouse = !Config.InvertMouse;
				break;
				case KEY_F3:
					Log.Write("Player: position=%.3f %.3f %.3f", Player->GetPosition()[0], Player->GetPosition()[1], Player->GetPosition()[2]);
				break;
				case KEY_F5:
					Game.ChangeState(&PlayState);
				break;
				case KEY_F11:
					ShowHUD = !ShowHUD;
				break;
				case KEY_F12:
					Graphics.SaveScreenshot();
				break;
				default:
					Processed = Player->ProcessKeyPress(Key);
				break;
			}

			// Send key presses to Lua
			LuaProcessed = Scripting.HandleKeyPress(Key);
		break;
		case STATE_PAUSED:
			switch(Key) {
				case KEY_ESCAPE:
					InitPlay();
				break;
			}
		break;
		case STATE_SAVEREPLAY:
			switch(Key) {
				case KEY_ESCAPE:
					InitPause();
				break;
				case KEY_RETURN:
					SaveReplay();
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_LOSE:
			if(Key == Config.Keys[_Actions::RESET])
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					Game.ChangeState(&MenuState);
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_WIN:
			if(Key == Config.Keys[_Actions::RESET])
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					Game.ChangeState(&MenuState);
				break;
				default:
					Processed = false;
				break;
			}
		break;
	}

	return Processed || LuaProcessed;
}

// Mouse motion
void _PlayState::HandleMouseMotion(float UpdateX, float UpdateY) {
	if(Resetting)
		return;

	switch(State) {
		case STATE_PLAY:
			if(Camera)
				Camera->HandleMouseMotion(UpdateX, UpdateY);
		break;
	}
}

// Mouse buttons
bool _PlayState::HandleMousePress(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return false;

	switch(State) {
		case STATE_PLAY:
			Scripting.HandleMousePress(Button, MouseX, MouseY); 
		break;
	}

	return false;
}

// Mouse buttons
void _PlayState::HandleMouseLift(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return;
}

// Mouse wheel
void _PlayState::HandleMouseWheel(float Direction) {

}

// GUI events
void _PlayState::HandleGUI(int EventType, IGUIElement *Element) {
	if(Resetting)
		return;

	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			Interface.PlaySound(InterfaceClass::SOUND_CONFIRM);

			switch(Element->getID()) {
				case PAUSE_RESUME:
					InitPlay();
				break;
				case PAUSE_SAVEREPLAY:
					TargetState = STATE_PAUSED;
					InitSaveReplay();
				break;
				case PAUSE_RESTART:
					StartReset();
				break;
				case PAUSE_MAINMENU:
					if(TestLevel == "")
						MenuState.SetTargetState(_MenuState::STATE_INITLEVELS);
					Game.ChangeState(&MenuState);
				break;
				case SAVEREPLAY_SAVE:
					SaveReplay();
				break;
				case SAVEREPLAY_CANCEL:
					if(TargetState == STATE_WIN)
						InitWin();
					else if(TargetState == STATE_LOSE)
						InitLose();
					else
						InitPause();
				break;
				case LOSE_RESTARTLEVEL:
					StartReset();
				break;
				case LOSE_SAVEREPLAY:
					TargetState = STATE_LOSE;
					InitSaveReplay();
				break;
				case LOSE_MAINMENU:
					if(TestLevel == "")
						MenuState.SetTargetState(_MenuState::STATE_INITLEVELS);
					Game.ChangeState(&MenuState);
				break;
				case WIN_RESTARTLEVEL:
					StartReset();
				break;
				case WIN_NEXTLEVEL:
					if(CampaignLevel+1 < Campaign.GetLevelCount(CurrentCampaign))
						CampaignLevel++;
					Game.ChangeState(&PlayState);
				break;
				case WIN_SAVEREPLAY:
					TargetState = STATE_WIN;
					InitSaveReplay();
				break;
				case WIN_MAINMENU:
					if(TestLevel == "")
						MenuState.SetTargetState(_MenuState::STATE_INITLEVELS);
					Game.ChangeState(&MenuState);
				break;
			}
		break;
	}
}

// Updates the current state
void _PlayState::Update(float FrameTime) {

	if(Resetting) {
		if(Fader.IsDoneFading()) {
			ResetLevel();
		}

		return;
	}

	switch(State) {
		case STATE_PLAY: {

			// Update time
			Timer += FrameTime;

			// Update replay
			Replay.Update(FrameTime);

			// Update game logic
			ObjectManager.BeginFrame();

			Player->HandleInput();
			ObjectManager.Update(FrameTime);
			Physics.Update(FrameTime);
			Interface.Update(FrameTime);
			Scripting.UpdateTimedCallbacks();

			ObjectManager.EndFrame();

			// Update audio
			const btVector3 &Position = Player->GetPosition();
			Audio.SetPosition(Position[0], Position[1], Position[2]);

			// Update camera for replay
			Camera->Update(vector3df(Position[0], Position[1], Position[2]));
			Camera->RecordReplay();

			Replay.ResetNextPacketTimer();
		} break;
		default:
		break;
	}
}

// Interpolate object positions
void _PlayState::UpdateRender(float TimeStepRemainder) {
	if(Resetting)
		return;

	switch(State) {
		case STATE_PLAY:

			Physics.GetWorld()->setTimeStepRemainder(TimeStepRemainder);
			Physics.GetWorld()->synchronizeMotionStates();

			// Set camera position
			btVector3 Position = Player->GetGraphicsPosition();
			Camera->Update(vector3df(Position[0], Position[1], Position[2]));
		break;
	}
}

// Draws the current state
void _PlayState::Draw() {
	char Buffer[256];
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;

	// Draw interface elements
	if(ShowHUD)
		Interface.Draw();

	// Draw timer
	char TimeString[32];
	Interface.ConvertSecondsToString(Timer, TimeString);
	if(ShowHUD)
		Interface.RenderText(TimeString, 10, 10, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_LARGE);

	switch(State) {
		case STATE_PAUSED:
		case STATE_SAVEREPLAY:
		case STATE_LOSE:
			Interface.FadeScreen(0.8f);
		break;
		case STATE_WIN: {
			Interface.FadeScreen(0.8f);

			// Draw header
			int X = CenterX;
			int Y = CenterY - WIN_HEIGHT / 2 + 15;
			Interface.DrawTextBox(CenterX, CenterY, WIN_WIDTH, WIN_HEIGHT);
			Interface.RenderText("Level Completed!", X, Y, InterfaceClass::ALIGN_CENTER, InterfaceClass::FONT_LARGE);

			// Draw time
			Y += 45;
			Interface.RenderText("Your Time", X - 115, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
			Interface.RenderText(TimeString, X + 115, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));

			// Best time
			Y += 25;
			if(WinStats->HighScores.size() > 0) {
				Interface.RenderText("Best Time", X - 115, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
				Interface.ConvertSecondsToString(WinStats->HighScores[0].Time, Buffer);
				Interface.RenderText(Buffer, X + 115, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
			}

			// High scores
			int HighX = CenterX - 75, HighY = Y + 48;

			// Draw header
			Interface.RenderText("#", HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			Interface.RenderText("Time", HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			Interface.RenderText("Date", HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			HighY += 17;
			for(u32 i = 0; i < WinStats->HighScores.size(); i++) {
				
				// Number
				char SmallBuffer[32];
				sprintf(SmallBuffer, "%d", i+1);
				Interface.RenderText(SmallBuffer, HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				// Time
				Interface.ConvertSecondsToString(WinStats->HighScores[i].Time, Buffer);
				Interface.RenderText(Buffer, HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				// Date
				char DateString[32];
				strftime(DateString, 32, "%m-%d-%Y", localtime(&WinStats->HighScores[i].DateStamp));
				Interface.RenderText(DateString, HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				HighY += 17;
			}
		} break;
	}

	// Draw irrlicht GUI
	irrGUI->drawAll();
}

// Init play GUI
void _PlayState::InitPlay() {
	irrGUI->clear();

	Graphics.SetClearColor(SColor(255, 0, 0, 0));
	Input.SetMouseLocked(true);

	State = STATE_PLAY;
}

// Draws the pause menu
void _PlayState::InitPause() {
	irrGUI->clear();
	
	// Draw interface
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;
	IGUIButton *ButtonResume = irrGUI->addButton(Interface.GetCenteredRect(CenterX, CenterY - 75, 130, 34), 0, PAUSE_RESUME, L"Resume");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface.GetCenteredRect(CenterX, CenterY - 25, 130, 34), 0, PAUSE_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonRestart = irrGUI->addButton(Interface.GetCenteredRect(CenterX, CenterY + 25, 130, 34), 0, PAUSE_RESTART, L"Restart Level");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface.GetCenteredRect(CenterX, CenterY + 75, 130, 34), 0, PAUSE_MAINMENU, L"Main Menu");
	ButtonResume->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonResume->setUseAlphaChannel(true);
	ButtonResume->setDrawBorder(false);
	ButtonSaveReplay->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonSaveReplay->setUseAlphaChannel(true);
	ButtonSaveReplay->setDrawBorder(false);
	ButtonRestart->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonRestart->setUseAlphaChannel(true);
	ButtonRestart->setDrawBorder(false);
	ButtonMainMenu->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonMainMenu->setUseAlphaChannel(true);
	ButtonMainMenu->setDrawBorder(false);
	
	Input.SetMouseLocked(false);

	State = STATE_PAUSED;
}

// Draws the save replay GUI
void _PlayState::InitSaveReplay() {
	irrGUI->clear();

	// Draw interface
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;
	IGUIEditBox *EditName = irrGUI->addEditBox(L"", Interface.GetCenteredRect(CenterX, CenterY - 20, 172, 32), true, 0, SAVEREPLAY_NAME);
	IGUIButton *ButtonSave = irrGUI->addButton(Interface.GetCenteredRect(CenterX - 45, CenterY + 20, 82, 34), 0, SAVEREPLAY_SAVE, L"Save");
	IGUIButton *ButtonCancel = irrGUI->addButton(Interface.GetCenteredRect(CenterX + 45, CenterY + 20, 82, 34), 0, SAVEREPLAY_CANCEL, L"Cancel");
	ButtonSave->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON80));
	ButtonSave->setUseAlphaChannel(true);
	ButtonSave->setDrawBorder(false);
	ButtonCancel->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON80));
	ButtonCancel->setUseAlphaChannel(true);
	ButtonCancel->setDrawBorder(false);

	irrGUI->setFocus(EditName);
	EditName->setMax(32);

	State = STATE_SAVEREPLAY;
}

// Draws the lose screen
void _PlayState::InitLose() {
/*	
	// Update stats
	if(TestLevel == "") {
		Save.IncrementLevelLoseCount(Level.GetLevelFile());
		Save.SaveLevelStats();
	}

	// Draw interface
	irrGUI->clear();
	IGUIStaticText *TextWin = irrGUI->addStaticText(L"You Lose", Interface.GetCenteredTextRect(L"You Lose", 0.15f, 0.20f), false, false, 0, -1, false);
	IGUIButton *ButtonRestartLevel = irrGUI->addButton(Interface.GetAbsoluteRectWH(0.4f, 0.3f, 0.2f, 0.05f), 0, LOSE_RESTARTLEVEL, L"Restart Level");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface.GetAbsoluteRectWH(0.4f, 0.4f, 0.2f, 0.05f), 0, LOSE_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface.GetAbsoluteRectWH(0.4f, 0.5f, 0.2f, 0.05f), 0, LOSE_MAINMENU, L"Main Menu");

	Input.SetMouseLocked(false);
*/
	State = STATE_LOSE;
}

// Draws the win screen
void _PlayState::InitWin() {
	
	// Skip stats if just testing a level
	bool LastLevelInCampaign = false;
	if(TestLevel == "") {

		// Increment win count
		Save.IncrementLevelWinCount(Level.GetLevelName());

		// Add high score
		Save.AddScore(Level.GetLevelName(), Timer);

		// Unlock next level
		int LevelCount = Campaign.GetLevelCount(CurrentCampaign);
		if(CampaignLevel+1 >= LevelCount) {
			LastLevelInCampaign = true;
		}
		else {
			const std::string &NextLevelFile = Campaign.GetLevel(CurrentCampaign, CampaignLevel+1);
			Save.UnlockLevel(NextLevelFile);
		}

		// Save stats to a file
		Save.SaveLevelStats(Level.GetLevelName());
	}
	else
		LastLevelInCampaign = true;

	// Get level stats
	WinStats = Save.GetLevelStats(Level.GetLevelName());
	
	// Clear interface
	Interface.Clear();

	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2, X, Y;
	X = CenterX;
	Y = CenterY + WIN_HEIGHT / 2 + 25;
	IGUIButton *ButtonRestartLevel = irrGUI->addButton(Interface.GetCenteredRect(X - 165, Y, 102, 34), 0, WIN_RESTARTLEVEL, L"Retry Level");
	IGUIButton *ButtonNextLevel = irrGUI->addButton(Interface.GetCenteredRect(X - 55, Y, 102, 34), 0, WIN_NEXTLEVEL, L"Next Level");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface.GetCenteredRect(X + 55, Y, 102, 34), 0, WIN_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface.GetCenteredRect(X + 165, Y, 102, 34), 0, WIN_MAINMENU, L"Main Menu");
	ButtonRestartLevel->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonRestartLevel->setUseAlphaChannel(true);
	ButtonRestartLevel->setDrawBorder(false);
	ButtonNextLevel->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonNextLevel->setUseAlphaChannel(true);
	ButtonNextLevel->setDrawBorder(false);
	ButtonSaveReplay->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonSaveReplay->setUseAlphaChannel(true);
	ButtonSaveReplay->setDrawBorder(false);
	ButtonMainMenu->setImage(Interface.GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonMainMenu->setUseAlphaChannel(true);
	ButtonMainMenu->setDrawBorder(false);
	
	if(LastLevelInCampaign)
		ButtonNextLevel->setEnabled(false);

	Input.SetMouseLocked(false);

	State = STATE_WIN;
}

// Saves a replay
void _PlayState::SaveReplay() {

	IGUIEditBox *EditName = static_cast<IGUIEditBox *>(irrGUI->getRootGUIElement()->getElementFromId(SAVEREPLAY_NAME));
	if(EditName != NULL) {
		irr::core::stringc ReplayTitle(EditName->getText());
		Replay.SaveReplay(ReplayTitle.c_str());
	}

	switch(TargetState) {
		case STATE_WIN: {
			InitWin();
			
			IGUIButton *ButtonSaveReplay = static_cast<IGUIButton *>(irrGUI->getRootGUIElement()->getElementFromId(WIN_SAVEREPLAY));
			ButtonSaveReplay->setEnabled(false);
		}
		break;
		case STATE_LOSE: {
			InitLose();
			
			IGUIButton *ButtonSaveReplay = static_cast<IGUIButton *>(irrGUI->getRootGUIElement()->getElementFromId(LOSE_SAVEREPLAY));
			ButtonSaveReplay->setEnabled(false);
		}
		break;
		default:
			InitPause();
		break;
	}
}

// Start resetting the level
void _PlayState::StartReset() {
	Fader.Start(-FADE_SPEED);
	Resetting = true;
}

// Resets the level
void _PlayState::ResetLevel() {

	// Handle saves
	if(TestLevel == "") {
		Save.IncrementLevelLoadCount(Level.GetLevelName());
		Save.IncrementLevelPlayTime(Level.GetLevelName(), Timer);
		Save.SaveLevelStats(Level.GetLevelName());
	}

	// Stop recording
	Replay.StopRecording();

	// Set up GUI
	InitPlay();
	Timer = 0.0f;

	// Set up camera
	Camera->SetRotation(0.0f, 30.0f);
	Camera->SetDistance(5.0f);

	// Clear objects
	ObjectManager.ClearObjects();
	Physics.Reset();

	// Start replay recording
	Replay.StartRecording();

	// Load level objects
	Level.SpawnObjects();
	Level.RunScripts();

	// Get the player
	GlobalPlayer = Player = static_cast<PlayerClass *>(ObjectManager.GetObjectByType(ObjectClass::PLAYER));
	if(Player == NULL) {
		Log.Write("_PlayState::ResetLevel - Cannot find player object");
		return;
	}
	Player->SetCamera(Camera);

	// Record camera in replay
	btVector3 Position = Player->GetPosition();
	Camera->Update(vector3df(Position[0], Position[1], Position[2]));
	Camera->RecordReplay();

	// Reset game timer
	Game.ResetTimer();
	Fader.Start(FADE_SPEED);
	Resetting = false;
}
