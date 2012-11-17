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
#include "orb.h"
#include "../engine/globals.h"
#include "../engine/physics.h"
#include "../engine/scripting.h"
#include "../engine/replay.h"
#include "../engine/constants.h"
#include "../engine/audio.h"
#include "template.h"
#include "../engine/namespace.h"

// Constructor
OrbClass::OrbClass(const SpawnStruct &Object)
:	ObjectClass(),
	DeactivationCallback(""),
	Sound(NULL),
	State(ORBSTATE_NORMAL),
	OrbTime(0.0f),
	DeactivateLength(ORB_DEACTIVATETIME) {

	TemplateStruct *Template = Object.Template;

	// Graphics
	Node = irrScene->addSphereSceneNode(Template->Radius, 24);
	if(Template->Textures[0] != "")
		Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
	else
		Node->setMaterialTexture(0, irrDriver->getTexture("textures/orb_outer0.png"));
	Node->setMaterialType(EMT_ONETEXTURE_BLEND);
	Node->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(EBF_ONE, EBF_ONE);
	
	// Create glow
	//GlowColor = SColor(255, 255, 240, 130);
	GlowColor = SColor(255, 255, 255, 255);
	InnerNode = irrScene->addBillboardSceneNode(Node, dimension2df(ORB_GLOWSIZE, ORB_GLOWSIZE));
	InnerNode->setColor(GlowColor);
	InnerNode->setMaterialFlag(EMF_LIGHTING, false);
	InnerNode->setMaterialFlag(EMF_ZBUFFER, false);
	InnerNode->setMaterialType(EMT_ONETEXTURE_BLEND);
	InnerNode->getMaterial(0).MaterialTypeParam = pack_textureBlendFunc(EBF_ONE, EBF_ONE);
	if(Template->Textures[1] != "")
		InnerNode->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[1].c_str()));
	else
		InnerNode->setMaterialTexture(0, irrDriver->getTexture("textures/orb_glow0.png"));

	// Set up physics
	if(Physics::Instance().IsEnabled()) {
	
		// Create shape
		btSphereShape *Shape = new btSphereShape(Template->Radius);

		// Set up physics
		CreateRigidBody(Object, Shape);

		// Audio
		Sound = new AudioSourceClass(Audio::Instance().GetBuffer("orb.ogg"), true, 0.0f, 0.45f, 8.0f, 16.0f);
		Sound->SetPitch(ORB_PITCH);
		Sound->SetPosition(Object.Position[0], Object.Position[1], Object.Position[2]);
		Sound->Play();
	}

	SetProperties(Object);
	if(CollisionCallback == "")
		CollisionCallback = "OnHitOrb";
}

// Destructor
OrbClass::~OrbClass() {

	delete Sound;
}

// Deactivates the object
void OrbClass::StartDeactivation(const std::string &Callback, float Length) {

	if(State == ORBSTATE_NORMAL) {
		State = ORBSTATE_DEACTIVATING;
		DeactivationCallback = Callback;
		DeactivateLength = Length;

		// Save the event on the replay
		if(Replay::Instance().IsRecording()) {
			FileClass &ReplayStream = Replay::Instance().GetReplayStream();
			Replay::Instance().WriteEvent(ReplayClass::PACKET_ORBDEACTIVATE);
			ReplayStream.WriteShortInt(ID);
			ReplayStream.WriteFloat(DeactivateLength);
		}
	}
}

// Updates the orb
void OrbClass::Update(float FrameTime) {

	// Update object
	ObjectClass::Update(FrameTime);

	// Update audio
	const btVector3 &Position = GetPosition();
	Sound->SetPosition(Position[0], Position[1], Position[2]);

	// Update orb
	UpdateDeactivation(FrameTime);
}

// Updates the replay
void OrbClass::UpdateReplay(float FrameTime) {
	ObjectClass::UpdateReplay(FrameTime);
	UpdateDeactivation(FrameTime);
}

// Updates the deactivation
void OrbClass::UpdateDeactivation(float FrameTime) {

	switch(State) {
		case ORBSTATE_NORMAL:
		break;
		case ORBSTATE_DEACTIVATING: {
			OrbTime += FrameTime;

			// Set glow size
			float PercentLeft = 1.0f - OrbTime / DeactivateLength;
			InnerNode->setSize(dimension2df(ORB_GLOWSIZE * PercentLeft, ORB_GLOWSIZE * PercentLeft));
			if(Sound)
				Sound->SetPitch(ORB_PITCH * PercentLeft);

			// Change states
			if(OrbTime >= DeactivateLength) {
				Scripting::Instance().CallFunction(DeactivationCallback);
				InnerNode->setVisible(false);
				State = ORBSTATE_DEACTIVATED;
				
				if(Sound)
					Sound->SetGain(0.0f);
			}
		} break;
		case ORBSTATE_DEACTIVATED:
		break;
	}
}
