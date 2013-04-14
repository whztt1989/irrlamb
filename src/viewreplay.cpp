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
#include <viewreplay.h>
#include <engine/globals.h>
#include <engine/input.h>
#include <engine/graphics.h>
#include <engine/config.h>
#include <engine/physics.h>
#include <engine/filestream.h>
#include <engine/level.h>
#include <engine/objectmanager.h>
#include <engine/replay.h>
#include <engine/interface.h>
#include <engine/camera.h>
#include <engine/game.h>
#include <engine/filestream.h>
#include <engine/constants.h>
#include <objects/orb.h>
#include <objects/template.h>
#include <play.h>
#include <menu.h>
#include <engine/namespace.h>

_ViewReplayState ViewReplayState;

// Initializes the state
int _ViewReplayState::Init() {

	// Set up state
	PauseSpeed = 1.0f;
	Game.SetTimeScale(1.0f);
	Timer = 0.0f;
	Input.SetMouseLocked(false);
	Interface.ChangeSkin(_Interface::SKIN_GAME);

	// Set up physics world
	Physics.SetEnabled(false);

	// Load replay
	if(!Replay.LoadReplay(CurrentReplay.c_str()))
		return 0;

	// Read first event
	Replay.ReadEvent(NextEvent);

	// Load the level
	if(!Level.Init(Replay.GetLevelName()))
		return 0;

	// Add camera
	Camera = new _Camera();

	// Turn off graphics until camera is positioned
	Graphics.SetDrawScene(false);

	// Initialize controls
	SetupGUI();

	return 1;
}

// Shuts the state down
int _ViewReplayState::Close() {

	// Stop replay
	Replay.StopReplay();

	// Clear objects
	delete Camera;
	Level.Close();
	ObjectManager.ClearObjects();
	Interface.Clear();
	irrScene->clear();
	
	return 1;
}

// Key presses
bool _ViewReplayState::HandleKeyPress(int Key) {

	bool Processed = true;
	switch(Key) {
		case KEY_ESCAPE:
			Menu.InitReplays();
			//Game.ChangeState(&Menu);
		break;
		case KEY_F1:
			//Game.ChangeState(&Menu);
		break;
		case KEY_F12:
			Graphics.SaveScreenshot();
		break;
		case KEY_SPACE:
			Pause();
		break;
		case KEY_RIGHT:
			Skip(1.0f);
		break;
		case KEY_UP:
			ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
		break;
		case KEY_DOWN:
			ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
		break;
		default:
			Processed = false;
		break;
	}

	return Processed;
}

// Handle action inputs
void _ViewReplayState::HandleAction(int Action, float Value) {
	if(Input.HasJoystick())
		Input.DriveMouse(Action, Value);
}

// Mouse wheel
void _ViewReplayState::HandleMouseWheel(float Direction) {

	if(Direction < 0)
		ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
	else
		ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
}

// GUI events
void _ViewReplayState::HandleGUI(irr::gui::EGUI_EVENT_TYPE EventType, IGUIElement *Element) {

	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_EXIT:
					Menu.InitReplays();
					//Game.ChangeState(&Menu);
				break;
				case MAIN_DECREASE:
					ChangeReplaySpeed(-REPLAY_TIME_INCREMENT);
				break;
				case MAIN_INCREASE:
					ChangeReplaySpeed(REPLAY_TIME_INCREMENT);
				break;
				case MAIN_PAUSE:
					Pause();
				break;
				case MAIN_RESTART:
					Game.ChangeState(this);
				break;
				case MAIN_SKIP:
					Skip(1.0f);
				break;
			}
		break;
	}
}

// Updates the current state
void _ViewReplayState::Update(float FrameTime) {

	// Update the replay
	Timer += FrameTime;
	while(!Replay.ReplayStopped() && Timer >= NextEvent.TimeStamp) {
		//printf("Processing header packet: type=%d time=%f\n", NextEvent.Type, NextEvent.TimeStamp);
		
		switch(NextEvent.Type) {
			case _Replay::PACKET_MOVEMENT:
				ObjectManager.UpdateFromReplay();
			break;
			case _Replay::PACKET_CREATE: {
				SpawnStruct Spawn;

				// Read replay
				_File &ReplayStream = Replay.GetReplayStream();
				int TemplateID = ReplayStream.ReadShortInt();
				int ObjectID = ReplayStream.ReadShortInt();
				ReplayStream.ReadData(Spawn.Position, sizeof(btScalar) * 3);
				ReplayStream.ReadData(Spawn.Rotation, sizeof(btScalar) * 3);

				// Create spawn object
				Spawn.Template = Level.GetTemplateFromID(TemplateID);
				if(Spawn.Template != NULL) {
					_Object *NewObject = Level.CreateObject(Spawn);
					NewObject->SetID(ObjectID);
				}
			}
			break;
			case _Replay::PACKET_DELETE: {

				// Read replay
				_File &ReplayStream = Replay.GetReplayStream();
				int ObjectID = ReplayStream.ReadShortInt();

				// Delete object
				ObjectManager.DeleteObjectByID(ObjectID);
			}
			break;
			case _Replay::PACKET_CAMERA: {
				
				// Read replay
				vector3df Position, LookAt;
				_File &ReplayStream = Replay.GetReplayStream();
				ReplayStream.ReadData(&Position.X, sizeof(float) * 3);
				ReplayStream.ReadData(&LookAt.X, sizeof(float) * 3);

				// Set camera orientation
				Camera->GetNode()->setPosition(Position);
				Camera->GetNode()->setTarget(LookAt);
				Graphics.SetDrawScene(true);
				
				//printf("Camera Position=%f %f %f Target=%f %f %f\n", Position.X, Position.Y, Position.Z, LookAt.X, LookAt.Y, LookAt.Z);
			}
			break;
			case _Replay::PACKET_ORBDEACTIVATE: {

				// Read replay
				_File &ReplayStream = Replay.GetReplayStream();
				int ObjectID = ReplayStream.ReadShortInt();
				float Length = ReplayStream.ReadFloat();

				// Deactivate orb
				_Orb *Orb = static_cast<_Orb *>(ObjectManager.GetObjectByID(ObjectID));
				Orb->StartDeactivation("", Length);
			}
			break;
			default:
			break;
		}

		Replay.ReadEvent(NextEvent);
	}
	
	ObjectManager.UpdateReplay(FrameTime);
	Interface.Update(FrameTime);
}

// Draws the current state
void _ViewReplayState::Draw() {
	char Buffer[256];
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;

	// Draw box
	int Left = 5, Top = 5, Width = 175, Height = 70;
	Interface.DrawTextBox(Left + Width/2, Top + Height/2, Width, Height, SColor(150, 255, 255, 255));

	// Draw timer
	float DisplayTime;
	if(!Replay.ReplayStopped())
		DisplayTime = Timer;
	else
		DisplayTime = Replay.GetFinishTime();

	// Draw time
	int X = Left + Width/2 - 10, Y = Top + 15;
	Interface.RenderText("Time", X - 5, Y, _Interface::ALIGN_RIGHT);
	Interface.ConvertSecondsToString(DisplayTime, Buffer);
	Interface.RenderText(Buffer, X + 5, Y, _Interface::ALIGN_LEFT);

	// Draw controls
	Y += 17;
	Interface.RenderText("Speed", X - 5, Y, _Interface::ALIGN_RIGHT);
	sprintf(Buffer, "%.2f", Game.GetTimeScale());
	Interface.RenderText(Buffer, X + 5, Y, _Interface::ALIGN_LEFT);

	irrGUI->drawAll();
}

// Setup GUI controls
void _ViewReplayState::SetupGUI() {
	int Right = irrDriver->getScreenSize().Width;

	// Restart replay
	int X = Right - 285, Y = 19;
	IGUIButton *ButtonRewind = irrGUI->addButton(Interface.GetCenteredRect(X, Y, 34, 34), 0, MAIN_RESTART);
	ButtonRewind->setImage(Interface.GetImage(_Interface::IMAGE_REWIND));
	ButtonRewind->setUseAlphaChannel(true);
	ButtonRewind->setDrawBorder(false);

	// Decrease replay speed
	X += 45;
	IGUIButton *ButtonDecrease = irrGUI->addButton(Interface.GetCenteredRect(X, Y, 34, 34), 0, MAIN_DECREASE);
	ButtonDecrease->setImage(Interface.GetImage(_Interface::IMAGE_DECREASE));
	ButtonDecrease->setUseAlphaChannel(true);
	ButtonDecrease->setDrawBorder(false);

	// Increase replay speed
	X += 37;
	IGUIButton *ButtonIncrease = irrGUI->addButton(Interface.GetCenteredRect(X, Y, 34, 34), 0, MAIN_INCREASE);
	ButtonIncrease->setImage(Interface.GetImage(_Interface::IMAGE_INCREASE));
	ButtonIncrease->setUseAlphaChannel(true);
	ButtonIncrease->setDrawBorder(false);

	// Pause
	X += 45;
	IGUIButton *ButtonPause = irrGUI->addButton(Interface.GetCenteredRect(X, Y, 34, 34), 0, MAIN_PAUSE);
	ButtonPause->setImage(Interface.GetImage(_Interface::IMAGE_PAUSE));
	ButtonPause->setUseAlphaChannel(true);
	ButtonPause->setDrawBorder(false);

	// Skip ahead
	X += 37;
	IGUIButton *ButtonSkip = irrGUI->addButton(Interface.GetCenteredRect(X, Y, 34, 34), 0, MAIN_SKIP);
	ButtonSkip->setImage(Interface.GetImage(_Interface::IMAGE_FASTFORWARD));
	ButtonSkip->setUseAlphaChannel(true);
	ButtonSkip->setDrawBorder(false);

	// Exit
	X += 45;
	IGUIButton *ButtonExit = irrGUI->addButton(Interface.GetCenteredRect(Right - 50, Y, 82, 34), 0, MAIN_EXIT, L"Exit");
	ButtonExit->setImage(Interface.GetImage(_Interface::IMAGE_BUTTON80));
	ButtonExit->setUseAlphaChannel(true);
	ButtonExit->setDrawBorder(false);
}

// Change replay speed
void _ViewReplayState::ChangeReplaySpeed(float Amount) {
	float Scale = Game.GetTimeScale();

	Scale += Amount;
	if(Scale >= 10.0f)
		Scale = 10.0f;
	else if(Scale <= 0.0f)
		Scale = 0.0f;

	Game.SetTimeScale(Scale);
}

// Pause the replay
void _ViewReplayState::Pause() {

	// Pause or play
	if(Game.GetTimeScale() == 0.0f) {
		Game.SetTimeScale(PauseSpeed);
	}
	else {
		PauseSpeed = Game.GetTimeScale();
		Game.SetTimeScale(0.0f);
	}
}

// Skip ahead
void _ViewReplayState::Skip(float Amount) {
	Game.UpdateTimeStepAccumulator(Amount);
}
