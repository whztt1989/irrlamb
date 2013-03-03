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
#include "game.h"
#include "input.h"
#include "globals.h"
#include "graphics.h"
#include "interface.h"
#include "audio.h"
#include "state.h"
#include "log.h"
#include "fader.h"
#include "random.h"
#include "scripting.h"
#include "physics.h"
#include "objectmanager.h"
#include "config.h"
#include "save.h"
#include "campaign.h"
#include "constants.h"
#include "../play.h"
#include "../viewreplay.h"
#include "../menu.h"
#include "namespace.h"

// Processes parameters and initializes the game
int GameClass::Init(int Count, char **Arguments) {

	// Defaults
	SleepRate = 120.0f;
	TimeStep = PHYSICS_TIMESTEP;
	TimeStepAccumulator = 0.0f;
	TimeStamp = 0;
	WindowActive = true;
	MouseWasLocked = false;
	Done = false;
	StateClass *FirstState = MenuState::Instance();
	E_DRIVER_TYPE DriverType = EDT_NULL;
	bool AudioEnabled = true;
	PlayState::Instance()->SetCampaign(-1);
	PlayState::Instance()->SetCampaignLevel(-1);

	// Process arguments
	std::string Token;
	int TokensRemaining;
	for(int i = 1; i < Count; i++) {
		Token = std::string(Arguments[i]);
		TokensRemaining = Count - i - 1;
		if(Token == "-level" && TokensRemaining > 0) {
			PlayState::Instance()->SetTestLevel(Arguments[++i]);
			FirstState = PlayState::Instance();
		}
		else if(Token == "-replay" && TokensRemaining > 0) {
			ViewReplayState::Instance()->SetCurrentReplay(Arguments[++i]);
			FirstState = ViewReplayState::Instance();
		}
		else if(Token == "-noaudio") {
			AudioEnabled = false;
		}
	}

	// Set up the save system
	if(!Save::Instance().Init())
		return 0;

	// Initialize logging system
	Log.Init();
	Log.Write("irrlamb %s", GAME_VERSION);

	// Set up config system
	if(!Config::Instance().Init())
		return 0;

	// Read the config file
	int HasConfigFile = Config::Instance().ReadConfig();

	// Set up the graphics
	DriverType = (E_DRIVER_TYPE)Config::Instance().DriverType;
	if(!Graphics::Instance().Init(Config::Instance().ScreenWidth, Config::Instance().ScreenHeight, Config::Instance().Fullscreen, DriverType, &Input::Instance()))
		return 0;

	// Save working path
	WorkingPath = std::string(irrFile->getWorkingDirectory().c_str()) + "/";

	// Write a config file if none exists
	if(!HasConfigFile)
		Config::Instance().WriteConfig();

	// Initialize level stats
	if(!Save::Instance().InitStatsDatabase())
		return 0;

	// Set random seed
	Random::Instance().SetSeed(irrTimer->getRealTime());

	// Set up the interface system
	if(!Interface::Instance().Init())
		return 0;

	// Set up audio
	if(AudioEnabled && Config::Instance().AudioEnabled)
		EnableAudio();

	// Set up the scripting system
	if(!Scripting::Instance().Init())
		return 0;

	// Set up the object manager
	if(!ObjectManager::Instance().Init())
		return 0;

	// Set up the object manager
	if(!Campaign::Instance().Init())
		return 0;

	// Set up physics world
	if(!Physics::Instance().Init())
		return 0;

	// Set up fader
	if(!Fader::Instance().Init())
		return 0;

	// Load stats file
	Save::Instance().LoadLevelStats();

	// Start the state timer
	ResetTimer();

	// Set the first state
	State = FirstState;
	NewState = NULL;
	ManagerState = STATE_INIT;
	Fader::Instance().Start(FADE_SPEED);

	return 1;
}

// Requests a state change
void GameClass::ChangeState(StateClass *State) {
	Fader::Instance().Start(-FADE_SPEED);

	NewState = State;
	ManagerState = STATE_CLOSE;
}

// Updates the current state and runs the game engine
void GameClass::Update() {

	// Run irrlicht engine
	if(!irrDevice->run())
		Done = true;

	// Get time difference from last frame
	float FrameTime = (irrTimer->getTime() - TimeStamp) * 0.001f;	
	TimeStamp = irrTimer->getTime();

	// Limit frame rate
	float ExtraTime = 1.0f / SleepRate - FrameTime;
	if(ExtraTime > 0.0f) {
		irrDevice->sleep((u32)(ExtraTime * 1000));
	}

	// Check for window activity
	PreviousWindowActive = WindowActive;
	WindowActive = irrDevice->isWindowActive();

	// Check for window focus/blur events
	if(PreviousWindowActive != WindowActive) {
		Input::Instance().ResetInputState();

		if(!WindowActive) {
			MouseWasLocked = Input::Instance().GetMouseLocked();
			Input::Instance().SetMouseLocked(false);
		}
		else {
			Input::Instance().SetMouseLocked(MouseWasLocked);
		}
	}

	// Update fader
	Fader::Instance().Update(FrameTime);
	Graphics::Instance().BeginFrame();

	// Update the current state
	switch(ManagerState) {
		case STATE_INIT:
			ResetGraphics();
			Input::Instance().ResetInputState();
			if(!State->Init()) {
				Done = true;
				return;
			}

			Fader::Instance().Start(FADE_SPEED);
			ResetTimer();
			ManagerState = STATE_UPDATE;
		break;
		case STATE_UPDATE:
			TimeStepAccumulator += FrameTime;
			while(TimeStepAccumulator >= TimeStep) {
				State->Update(TimeStep);
				TimeStepAccumulator -= TimeStep;
			}
			
			State->UpdateRender(TimeStepAccumulator);
		break;
		case STATE_CLOSE:
			if(Fader::Instance().IsDoneFading()) {
				State->Close();
				State = NewState;
				ManagerState = STATE_INIT;
			}
		break;
	}

	State->Draw();
	Graphics::Instance().EndFrame();
}

// Shuts down the system
void GameClass::Close() {
	
	// Close the state
	State->Close();

	// Shut down the system
	Campaign::Instance().Close();
	Fader::Instance().Close();
	Physics::Instance().Close();
	ObjectManager::Instance().Close();
	Scripting::Instance().Close();
	Interface::Instance().Close();
	Audio::Instance().Close();
	Graphics::Instance().Close();
	Config::Instance().Close();
	Save::Instance().Close();
	Log.Close();
}

// Resets the game timer
void GameClass::ResetTimer() {
	
	irrTimer->setTime(0);
	TimeStamp = irrTimer->getTime();
}

// Resets the graphics for a state
void GameClass::ResetGraphics() {
	Graphics::Instance().SetClearColor(SColor(0, 0, 0, 0));
	Graphics::Instance().SetDrawScene(true);
	Interface::Instance().Clear();
}

// Initialize the audio system and load basic buffers
void GameClass::EnableAudio() {
	Audio::Instance().Init(true);
	Audio::Instance().LoadBuffer("confirm.ogg");
	Audio::Instance().LoadBuffer("orb.ogg");
	Audio::Instance().LoadBuffer("player.ogg");
	Interface::Instance().LoadSounds();
}

// Disable audio system
void GameClass::DisableAudio() {
	Audio::Instance().Close();
	Interface::Instance().UnloadSounds();
}
