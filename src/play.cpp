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
#include "engine/save.h"
#include "objects/player.h"
#include "menu.h"
#include "viewreplay.h"
#include "engine/namespace.h"

const int WIN_WIDTH = 430;
const int WIN_HEIGHT = 350;

// Initializes the state
int PlayState::Init() {
	Player = NULL;
	Timer = 0.0f;
	Resetting = false;
	WinStats = NULL;
	ShowHUD = true;
	Physics::Instance().SetEnabled(true);
	Interface::Instance().ChangeSkin(InterfaceClass::SKIN_GAME);

	// Add camera
	Camera = new CameraClass();

	// Load the level
	std::string LevelFile;
	
	// Select a level to load
	if(TestLevel != "")
		LevelFile = TestLevel;
	else
		LevelFile = Campaign::Instance().GetLevel(Campaign, CampaignLevel);

	// Load level
	if(!Level::Instance().Init(LevelFile))
		return 0;

	// Reset level
	ResetLevel();

	return 1;
}

// Shuts the state down
int PlayState::Close() {

	// Stop the replay
	Replay::Instance().StopRecording();

	// Close the system down
	delete Camera;
	Level::Instance().Close();
	ObjectManager::Instance().ClearObjects();
	Interface::Instance().Clear();
	irrScene->clear();

	// Save stats
	if(TestLevel == "") {
		Save::Instance().IncrementLevelPlayTime(Level::Instance().GetLevelName(), Timer);
		Save::Instance().SaveLevelStats(Level::Instance().GetLevelName());
	}

	return 1;
}

// Key presses
bool PlayState::HandleKeyPress(int Key) {
	if(Resetting)
		return true;

	bool Processed = true, LuaProcessed = false;
	
	switch(State) {
		case STATE_PLAY:
			if(Key == Config::Instance().Keys[ACTIONS::RESET])
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					if(TestLevel != "")
						Game::Instance().SetDone(true);
					else
						InitPause();
				break;
				case KEY_F1:
					InitPause();
				break;
				case KEY_F2:
					Config::Instance().InvertMouse = !Config::Instance().InvertMouse;
				break;
				case KEY_F3:
					Log.Write("Player: position=%.3f %.3f %.3f", Player->GetPosition()[0], Player->GetPosition()[1], Player->GetPosition()[2]);
				break;
				case KEY_F5:
					Game::Instance().ChangeState(PlayState::Instance());
				break;
				case KEY_F11:
					ShowHUD = !ShowHUD;
				break;
				case KEY_F12:
					Graphics::Instance().SaveScreenshot();
				break;
				default:
					Processed = Player->ProcessKeyPress(Key);
				break;
			}

			// Send key presses to Lua
			LuaProcessed = Scripting::Instance().HandleKeyPress(Key);
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
			if(Key == Config::Instance().Keys[ACTIONS::RESET])
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					Game::Instance().ChangeState(MenuState::Instance());
				break;
				default:
					Processed = false;
				break;
			}
		break;
		case STATE_WIN:
			if(Key == Config::Instance().Keys[ACTIONS::RESET])
				StartReset();

			switch(Key) {
				case KEY_ESCAPE:
					Game::Instance().ChangeState(MenuState::Instance());
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
void PlayState::HandleMouseMotion(float UpdateX, float UpdateY) {
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
bool PlayState::HandleMousePress(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return false;

	switch(State) {
		case STATE_PLAY:
			Scripting::Instance().HandleMousePress(Button, MouseX, MouseY); 
			Player->HandleMousePress(Button, MouseX, MouseY);
		break;
	}

	return false;
}

// Mouse buttons
void PlayState::HandleMouseLift(int Button, int MouseX, int MouseY) {
	if(Resetting)
		return;

	Player->HandleMouseLift(Button, MouseX, MouseY);
}

// Mouse wheel
void PlayState::HandleMouseWheel(float Direction) {

}

// GUI events
void PlayState::HandleGUI(int EventType, IGUIElement *Element) {
	if(Resetting)
		return;

	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			Interface::Instance().PlaySound(InterfaceClass::SOUND_CONFIRM);

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
						MenuState::Instance()->SetTargetState(MenuState::STATE_INITLEVELS);
					Game::Instance().ChangeState(MenuState::Instance());
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
						MenuState::Instance()->SetTargetState(MenuState::STATE_INITLEVELS);
					Game::Instance().ChangeState(MenuState::Instance());
				break;
				case WIN_RESTARTLEVEL:
					StartReset();
				break;
				case WIN_NEXTLEVEL:
					if(CampaignLevel+1 < Campaign::Instance().GetLevelCount(Campaign))
						CampaignLevel++;
					Game::Instance().ChangeState(PlayState::Instance());
				break;
				case WIN_SAVEREPLAY:
					TargetState = STATE_WIN;
					InitSaveReplay();
				break;
				case WIN_MAINMENU:
					if(TestLevel == "")
						MenuState::Instance()->SetTargetState(MenuState::STATE_INITLEVELS);
					Game::Instance().ChangeState(MenuState::Instance());
				break;
			}
		break;
	}
}

// Updates the current state
void PlayState::Update(float FrameTime) {

	if(Resetting) {
		if(Fader::Instance().IsDoneFading()) {
			ResetLevel();
		}

		return;
	}

	switch(State) {
		case STATE_PLAY: {

			// Update time
			Timer += FrameTime;

			// Update replay
			Replay::Instance().Update(FrameTime);

			// Update game logic
			ObjectManager::Instance().BeginFrame();

			Player->HandleInput();
			ObjectManager::Instance().Update(FrameTime);
			Physics::Instance().Update(FrameTime);
			Interface::Instance().Update(FrameTime);
			Scripting::Instance().UpdateTimedCallbacks();

			ObjectManager::Instance().EndFrame();

			// Update audio
			const btVector3 &Position = Player->GetPosition();
			Audio::Instance().SetPosition(Position[0], Position[1], Position[2]);

			// Update camera for replay
			Camera->Update(vector3df(Position[0], Position[1], Position[2]));
			Camera->RecordReplay();

			Replay::Instance().ResetNextPacketTimer();
		} break;
		default:
		break;
	}
}

// Interpolate object positions
void PlayState::UpdateRender(float TimeStepRemainder) {
	if(Resetting)
		return;

	switch(State) {
		case STATE_PLAY:

			Physics::Instance().GetWorld()->setTimeStepRemainder(TimeStepRemainder);
			Physics::Instance().GetWorld()->synchronizeMotionStates();

			// Set camera position
			btVector3 Position = Player->GetGraphicsPosition();
			Camera->Update(vector3df(Position[0], Position[1], Position[2]));
		break;
	}
}

// Draws the current state
void PlayState::Draw() {
	char Buffer[256];
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;

	// Draw interface elements
	if(ShowHUD)
		Interface::Instance().Draw();

	// Draw timer
	char TimeString[32];
	Interface::Instance().ConvertSecondsToString(Timer, TimeString);
	if(ShowHUD)
		Interface::Instance().RenderText(TimeString, 10, 10, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_LARGE);

	switch(State) {
		case STATE_PAUSED:
		case STATE_SAVEREPLAY:
		case STATE_LOSE:
			Interface::Instance().FadeScreen(0.8f);
		break;
		case STATE_WIN: {
			Interface::Instance().FadeScreen(0.8f);

			// Draw header
			int X = CenterX;
			int Y = CenterY - WIN_HEIGHT / 2 + 15;
			Interface::Instance().DrawTextBox(CenterX, CenterY, WIN_WIDTH, WIN_HEIGHT);
			Interface::Instance().RenderText("Level Completed!", X, Y, InterfaceClass::ALIGN_CENTER, InterfaceClass::FONT_LARGE);

			// Draw time
			Y += 45;
			Interface::Instance().RenderText("Your Time", X - 115, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
			Interface::Instance().RenderText(TimeString, X + 115, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));

			// Best time
			Y += 25;
			if(WinStats->HighScores.size() > 0) {
				Interface::Instance().RenderText("Best Time", X - 115, Y, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
				Interface::Instance().ConvertSecondsToString(WinStats->HighScores[0].Time, Buffer);
				Interface::Instance().RenderText(Buffer, X + 115, Y, InterfaceClass::ALIGN_RIGHT, InterfaceClass::FONT_MEDIUM, SColor(200, 255, 255, 255));
			}

			// High scores
			int HighX = CenterX - 75, HighY = Y + 48;

			// Draw header
			Interface::Instance().RenderText("#", HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			Interface::Instance().RenderText("Time", HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			Interface::Instance().RenderText("Date", HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(255, 255, 255, 255));
			HighY += 17;
			for(u32 i = 0; i < WinStats->HighScores.size(); i++) {
				
				// Number
				char SmallBuffer[32];
				sprintf(SmallBuffer, "%d", i+1);
				Interface::Instance().RenderText(SmallBuffer, HighX, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				// Time
				Interface::Instance().ConvertSecondsToString(WinStats->HighScores[i].Time, Buffer);
				Interface::Instance().RenderText(Buffer, HighX + 30, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				// Date
				char DateString[32];
				strftime(DateString, 32, "%m-%d-%Y", localtime(&WinStats->HighScores[i].DateStamp));
				Interface::Instance().RenderText(DateString, HighX + 110, HighY, InterfaceClass::ALIGN_LEFT, InterfaceClass::FONT_SMALL, SColor(200, 255, 255, 255));

				HighY += 17;
			}
		} break;
	}

	// Draw irrlicht GUI
	irrGUI->drawAll();
}

// Init play GUI
void PlayState::InitPlay() {
	irrGUI->clear();

	Graphics::Instance().SetClearColor(SColor(255, 0, 0, 0));
	Input::Instance().SetMouseLocked(true);

	State = STATE_PLAY;
}

// Draws the pause menu
void PlayState::InitPause() {
	irrGUI->clear();
	
	// Draw interface
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;
	IGUIButton *ButtonResume = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY - 75, 130, 34), 0, PAUSE_RESUME, L"Resume");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY - 25, 130, 34), 0, PAUSE_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonRestart = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY + 25, 130, 34), 0, PAUSE_RESTART, L"Restart Level");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX, CenterY + 75, 130, 34), 0, PAUSE_MAINMENU, L"Main Menu");
	ButtonResume->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonResume->setUseAlphaChannel(true);
	ButtonResume->setDrawBorder(false);
	ButtonSaveReplay->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonSaveReplay->setUseAlphaChannel(true);
	ButtonSaveReplay->setDrawBorder(false);
	ButtonRestart->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonRestart->setUseAlphaChannel(true);
	ButtonRestart->setDrawBorder(false);
	ButtonMainMenu->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON128));
	ButtonMainMenu->setUseAlphaChannel(true);
	ButtonMainMenu->setDrawBorder(false);
	
	Input::Instance().SetMouseLocked(false);

	State = STATE_PAUSED;
}

// Draws the save replay GUI
void PlayState::InitSaveReplay() {
	irrGUI->clear();

	// Draw interface
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;
	IGUIEditBox *EditName = irrGUI->addEditBox(L"", Interface::Instance().GetCenteredRect(CenterX, CenterY - 20, 172, 32), true, 0, SAVEREPLAY_NAME);
	IGUIButton *ButtonSave = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX - 45, CenterY + 20, 82, 34), 0, SAVEREPLAY_SAVE, L"Save");
	IGUIButton *ButtonCancel = irrGUI->addButton(Interface::Instance().GetCenteredRect(CenterX + 45, CenterY + 20, 82, 34), 0, SAVEREPLAY_CANCEL, L"Cancel");
	ButtonSave->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
	ButtonSave->setUseAlphaChannel(true);
	ButtonSave->setDrawBorder(false);
	ButtonCancel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
	ButtonCancel->setUseAlphaChannel(true);
	ButtonCancel->setDrawBorder(false);

	irrGUI->setFocus(EditName);
	EditName->setMax(32);

	State = STATE_SAVEREPLAY;
}

// Draws the lose screen
void PlayState::InitLose() {
/*	
	// Update stats
	if(TestLevel == "") {
		Save::Instance().IncrementLevelLoseCount(Level::Instance().GetLevelFile());
		Save::Instance().SaveLevelStats();
	}

	// Draw interface
	irrGUI->clear();
	IGUIStaticText *TextWin = irrGUI->addStaticText(L"You Lose", Interface::Instance().GetCenteredTextRect(L"You Lose", 0.15f, 0.20f), false, false, 0, -1, false);
	IGUIButton *ButtonRestartLevel = irrGUI->addButton(Interface::Instance().GetAbsoluteRectWH(0.4f, 0.3f, 0.2f, 0.05f), 0, LOSE_RESTARTLEVEL, L"Restart Level");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface::Instance().GetAbsoluteRectWH(0.4f, 0.4f, 0.2f, 0.05f), 0, LOSE_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface::Instance().GetAbsoluteRectWH(0.4f, 0.5f, 0.2f, 0.05f), 0, LOSE_MAINMENU, L"Main Menu");

	Input::Instance().SetMouseLocked(false);
*/
	State = STATE_LOSE;
}

// Draws the win screen
void PlayState::InitWin() {
	
	// Skip stats if just testing a level
	bool LastLevelInCampaign = false;
	if(TestLevel == "") {

		// Increment win count
		Save::Instance().IncrementLevelWinCount(Level::Instance().GetLevelName());

		// Add high score
		Save::Instance().AddScore(Level::Instance().GetLevelName(), Timer);

		// Unlock next level
		int LevelCount = Campaign::Instance().GetLevelCount(Campaign);
		if(CampaignLevel+1 >= LevelCount) {
			LastLevelInCampaign = true;
		}
		else {
			const std::string &NextLevelFile = Campaign::Instance().GetLevel(Campaign, CampaignLevel+1);
			Save::Instance().UnlockLevel(NextLevelFile);
		}

		// Save stats to a file
		Save::Instance().SaveLevelStats(Level::Instance().GetLevelName());
	}
	else
		LastLevelInCampaign = true;

	// Get level stats
	WinStats = Save::Instance().GetLevelStats(Level::Instance().GetLevelName());
	
	// Clear interface
	Interface::Instance().Clear();

	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2, X, Y;
	X = CenterX;
	Y = CenterY + WIN_HEIGHT / 2 + 25;
	IGUIButton *ButtonRestartLevel = irrGUI->addButton(Interface::Instance().GetCenteredRect(X - 165, Y, 102, 34), 0, WIN_RESTARTLEVEL, L"Retry Level");
	IGUIButton *ButtonNextLevel = irrGUI->addButton(Interface::Instance().GetCenteredRect(X - 55, Y, 102, 34), 0, WIN_NEXTLEVEL, L"Next Level");
	IGUIButton *ButtonSaveReplay = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 55, Y, 102, 34), 0, WIN_SAVEREPLAY, L"Save Replay");
	IGUIButton *ButtonMainMenu = irrGUI->addButton(Interface::Instance().GetCenteredRect(X + 165, Y, 102, 34), 0, WIN_MAINMENU, L"Main Menu");
	ButtonRestartLevel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonRestartLevel->setUseAlphaChannel(true);
	ButtonRestartLevel->setDrawBorder(false);
	ButtonNextLevel->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonNextLevel->setUseAlphaChannel(true);
	ButtonNextLevel->setDrawBorder(false);
	ButtonSaveReplay->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonSaveReplay->setUseAlphaChannel(true);
	ButtonSaveReplay->setDrawBorder(false);
	ButtonMainMenu->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON100));
	ButtonMainMenu->setUseAlphaChannel(true);
	ButtonMainMenu->setDrawBorder(false);
	
	if(LastLevelInCampaign)
		ButtonNextLevel->setEnabled(false);

	Input::Instance().SetMouseLocked(false);

	State = STATE_WIN;
}

// Saves a replay
void PlayState::SaveReplay() {

	IGUIEditBox *EditName = static_cast<IGUIEditBox *>(irrGUI->getRootGUIElement()->getElementFromId(SAVEREPLAY_NAME));
	if(EditName != NULL) {
		irr::core::stringc ReplayTitle(EditName->getText());
		Replay::Instance().SaveReplay(ReplayTitle.c_str());
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
void PlayState::StartReset() {
	Fader::Instance().Start(-FADE_SPEED);
	Resetting = true;
}

// Resets the level
void PlayState::ResetLevel() {

	// Handle saves
	if(TestLevel == "") {
		Save::Instance().IncrementLevelLoadCount(Level::Instance().GetLevelName());
		Save::Instance().IncrementLevelPlayTime(Level::Instance().GetLevelName(), Timer);
		Save::Instance().SaveLevelStats(Level::Instance().GetLevelName());
	}

	// Stop recording
	Replay::Instance().StopRecording();

	// Set up GUI
	InitPlay();
	Timer = 0.0f;

	// Set up camera
	Camera->SetRotation(0.0f, 30.0f);
	Camera->SetDistance(5.0f);

	// Clear objects
	ObjectManager::Instance().ClearObjects();
	Physics::Instance().Reset();

	// Start replay recording
	Replay::Instance().StartRecording();

	// Load level objects
	Level::Instance().SpawnObjects();
	Level::Instance().RunScripts();

	// Get the player
	Player = static_cast<PlayerClass *>(ObjectManager::Instance().GetObjectByType(ObjectClass::PLAYER));
	if(Player == NULL) {
		Log.Write("PlayState::ResetLevel - Cannot find player object");
		return;
	}
	Player->SetCamera(Camera);

	// Record camera in replay
	btVector3 Position = Player->GetPosition();
	Camera->Update(vector3df(Position[0], Position[1], Position[2]));
	Camera->RecordReplay();

	// Reset game timer
	Game::Instance().ResetTimer();
	Fader::Instance().Start(FADE_SPEED);
	Resetting = false;
}
