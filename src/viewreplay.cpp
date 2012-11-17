/*************************************************************************************
*	irrlamb - http://irrlamb.googlecode.com
*	Copyright (C) 2011  Alan Witkowski
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
#include "viewreplay.h"
#include "engine/globals.h"
#include "engine/input.h"
#include "engine/graphics.h"
#include "engine/config.h"
#include "engine/physics.h"
#include "engine/filestream.h"
#include "engine/level.h"
#include "engine/objectmanager.h"
#include "engine/replay.h"
#include "engine/interface.h"
#include "engine/camera.h"
#include "engine/game.h"
#include "engine/filestream.h"
#include "objects/orb.h"
#include "objects/template.h"
#include "play.h"
#include "menu.h"
#include "engine/namespace.h"

// Initializes the state
int ViewReplayState::Init() {

	// Set up state
	ReplaySpeed = PauseSpeed = 1.0f;
	Timer = 0.0f;
	Input::Instance().SetMouseLocked(false);
	Interface::Instance().ChangeSkin(InterfaceClass::SKIN_GAME);

	// Set up physics world
	Physics::Instance().SetEnabled(false);

	// Load replay
	if(!Replay::Instance().LoadReplay(CurrentReplay.c_str()))
		return 0;

	// Read first event
	Replay::Instance().ReadEvent(NextEvent);

	// Load the level
	if(!Level::Instance().Init(Replay::Instance().GetLevelName()))
		return 0;

	// Add camera
	Camera = new CameraClass();

	// Turn off graphics until camera is positioned
	Graphics::Instance().SetDrawScene(false);

	// Initialize controls
	SetupGUI();

	return 1;
}

// Shuts the state down
int ViewReplayState::Close() {

	// Stop replay
	Replay::Instance().StopReplay();

	// Clear objects
	delete Camera;
	Level::Instance().Close();
	ObjectManager::Instance().ClearObjects();
	Interface::Instance().Clear();
	irrScene->clear();
	
	return 1;
}

// Key presses
bool ViewReplayState::HandleKeyPress(int Key) {

	bool Processed = true;
	switch(Key) {
		case KEY_ESCAPE:
			MenuState::Instance()->SetTargetState(MenuState::STATE_INITREPLAYS);
			Game::Instance().ChangeState(MenuState::Instance());
		break;
		case KEY_F1:
			Game::Instance().ChangeState(MenuState::Instance());
		break;
		case KEY_F12:
			Graphics::Instance().SaveScreenshot();
		break;
		case KEY_SPACE:
			Pause();
		break;
		case KEY_RIGHT:
			Skip(1.0f);
		break;
		case KEY_UP:
			ChangeReplaySpeed(0.25f);
		break;
		case KEY_DOWN:
			ChangeReplaySpeed(-0.25f);
		break;
		default:
			Processed = false;
		break;
	}

	return Processed;
}

// Mouse wheel
void ViewReplayState::HandleMouseWheel(float Direction) {

	if(Direction < 0)
		ChangeReplaySpeed(-0.25f);
	else
		ChangeReplaySpeed(0.25f);
}

// GUI events
void ViewReplayState::HandleGUI(int EventType, IGUIElement *Element) {

	switch(EventType) {
		case EGET_BUTTON_CLICKED:
			switch(Element->getID()) {
				case MAIN_EXIT:
					MenuState::Instance()->SetTargetState(MenuState::STATE_INITREPLAYS);
					Game::Instance().ChangeState(MenuState::Instance());
				break;
				case MAIN_DECREASE:
					ChangeReplaySpeed(-0.25f);
				break;
				case MAIN_INCREASE:
					ChangeReplaySpeed(0.25f);
				break;
				case MAIN_PAUSE:
					Pause();
				break;
				case MAIN_RESTART:
					Game::Instance().ChangeState(this);
				break;
				case MAIN_SKIP:
					Skip(1.0f);
				break;
			}
		break;
	}
}

// Updates the current state
void ViewReplayState::Update(float FrameTime) {

	// Update the replay
	Timer += FrameTime * ReplaySpeed;
	while(!Replay::Instance().ReplayStopped() && Timer >= NextEvent.TimeStamp) {
		//printf("Processing header packet: type=%d time=%f\n", NextEvent.Type, NextEvent.TimeStamp);
		
		switch(NextEvent.Type) {
			case ReplayClass::PACKET_MOVEMENT:
				ObjectManager::Instance().UpdateFromReplay();
			break;
			case ReplayClass::PACKET_CREATE: {
				SpawnStruct Spawn;

				// Read replay
				FileClass &ReplayStream = Replay::Instance().GetReplayStream();
				int TemplateID = ReplayStream.ReadShortInt();
				int ObjectID = ReplayStream.ReadShortInt();
				ReplayStream.ReadData(Spawn.Position, sizeof(btScalar) * 3);
				ReplayStream.ReadData(Spawn.Rotation, sizeof(btScalar) * 3);

				// Create spawn object
				Spawn.Template = Level::Instance().GetTemplateFromID(TemplateID);
				if(Spawn.Template != NULL) {
					ObjectClass *NewObject = Level::Instance().CreateObject(Spawn);
					NewObject->SetID(ObjectID);
				}
			}
			break;
			case ReplayClass::PACKET_DELETE: {

				// Read replay
				FileClass &ReplayStream = Replay::Instance().GetReplayStream();
				int ObjectID = ReplayStream.ReadShortInt();

				// Delete object
				ObjectManager::Instance().DeleteObjectByID(ObjectID);
			}
			break;
			case ReplayClass::PACKET_CAMERA: {
				
				// Read replay
				vector3df Position, LookAt;
				FileClass &ReplayStream = Replay::Instance().GetReplayStream();
				ReplayStream.ReadData(&Position.X, sizeof(float) * 3);
				ReplayStream.ReadData(&LookAt.X, sizeof(float) * 3);

				// Set camera orientation
				Camera->GetNode()->setPosition(Position);
				Camera->GetNode()->setTarget(LookAt);
				Graphics::Instance().SetDrawScene(true);
				
				//printf("Camera Position=%f %f %f Target=%f %f %f\n", Position.X, Position.Y, Position.Z, LookAt.X, LookAt.Y, LookAt.Z);
			}
			break;
			case ReplayClass::PACKET_ORBDEACTIVATE: {

				// Read replay
				FileClass &ReplayStream = Replay::Instance().GetReplayStream();
				int ObjectID = ReplayStream.ReadShortInt();
				float Length = ReplayStream.ReadFloat();

				// Deactivate orb
				OrbClass *Orb = static_cast<OrbClass *>(ObjectManager::Instance().GetObjectByID(ObjectID));
				Orb->StartDeactivation("", Length);
			}
			break;
			default:
			break;
		}

		Replay::Instance().ReadEvent(NextEvent);
	}
	
	ObjectManager::Instance().UpdateReplay(FrameTime);
	Interface::Instance().Update(FrameTime);
}

// Draws the current state
void ViewReplayState::Draw() {
	char Buffer[256];
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2;

	// Draw box
	int Left = 5, Top = 5, Width = 175, Height = 70;
	Interface::Instance().DrawTextBox(Left + Width/2, Top + Height/2, Width, Height, SColor(150, 255, 255, 255));

	// Draw timer
	float DisplayTime;
	if(!Replay::Instance().ReplayStopped())
		DisplayTime = Timer;
	else
		DisplayTime = Replay::Instance().GetFinishTime();

	// Draw time
	int X = Left + Width/2 - 10, Y = Top + 15;
	Interface::Instance().RenderText("Time", X - 5, Y, InterfaceClass::ALIGN_RIGHT);
	Interface::Instance().ConvertSecondsToString(DisplayTime, Buffer);
	Interface::Instance().RenderText(Buffer, X + 5, Y, InterfaceClass::ALIGN_LEFT);

	// Draw controls
	Y += 17;
	Interface::Instance().RenderText("Speed", X - 5, Y, InterfaceClass::ALIGN_RIGHT);
	sprintf(Buffer, "%.2f", ReplaySpeed);
	Interface::Instance().RenderText(Buffer, X + 5, Y, InterfaceClass::ALIGN_LEFT);

	irrGUI->drawAll();
}

// Setup GUI controls
void ViewReplayState::SetupGUI() {
	int Right = irrDriver->getScreenSize().Width;

	// Restart replay
	int X = Right - 285, Y = 19;
	IGUIButton *ButtonRewind = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 34, 34), 0, MAIN_RESTART);
	ButtonRewind->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_REWIND));
	ButtonRewind->setUseAlphaChannel(true);
	ButtonRewind->setDrawBorder(false);

	// Decrease replay speed
	X += 45;
	IGUIButton *ButtonDecrease = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 34, 34), 0, MAIN_DECREASE);
	ButtonDecrease->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_DECREASE));
	ButtonDecrease->setUseAlphaChannel(true);
	ButtonDecrease->setDrawBorder(false);

	// Increase replay speed
	X += 37;
	IGUIButton *ButtonIncrease = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 34, 34), 0, MAIN_INCREASE);
	ButtonIncrease->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_INCREASE));
	ButtonIncrease->setUseAlphaChannel(true);
	ButtonIncrease->setDrawBorder(false);

	// Pause
	X += 45;
	IGUIButton *ButtonPause = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 34, 34), 0, MAIN_PAUSE);
	ButtonPause->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_PAUSE));
	ButtonPause->setUseAlphaChannel(true);
	ButtonPause->setDrawBorder(false);

	// Skip ahead
	X += 37;
	IGUIButton *ButtonSkip = irrGUI->addButton(Interface::Instance().GetCenteredRect(X, Y, 34, 34), 0, MAIN_SKIP);
	ButtonSkip->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_FASTFORWARD));
	ButtonSkip->setUseAlphaChannel(true);
	ButtonSkip->setDrawBorder(false);

	// Exit
	X += 45;
	IGUIButton *ButtonExit = irrGUI->addButton(Interface::Instance().GetCenteredRect(Right - 50, Y, 82, 34), 0, MAIN_EXIT, L"Exit");
	ButtonExit->setImage(Interface::Instance().GetImage(InterfaceClass::IMAGE_BUTTON80));
	ButtonExit->setUseAlphaChannel(true);
	ButtonExit->setDrawBorder(false);
}

// Change replay speed
void ViewReplayState::ChangeReplaySpeed(float Amount) {
	
	ReplaySpeed += Amount;
	if(ReplaySpeed >= 10.0f)
		ReplaySpeed = 10.0f;
	else if(ReplaySpeed <= 0.0f)
		ReplaySpeed = 0.0f;
}

// Pause the replay
void ViewReplayState::Pause() {

	// Pause or play
	if(ReplaySpeed == 0.0f) {
		ReplaySpeed = PauseSpeed;
	}
	else {
		PauseSpeed = ReplaySpeed;
		ReplaySpeed = 0.0f;
	}
}

// Skip ahead
void ViewReplayState::Skip(float Amount) {
	Timer += Amount;
	Replay::Instance().Update(Amount);
}
