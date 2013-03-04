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
#ifndef PLAYER_H
#define PLAYER_H

// Libraries
#include "object.h"
#include "../engine/camera.h"
#include <ILightSceneNode.h>

// Classes
class PlayerClass : public ObjectClass {

	public:

		PlayerClass(const SpawnStruct &Object);
		~PlayerClass();

		void Update(float FrameTime);
		bool ProcessKeyPress(int Key);
		bool HandleMousePress(int Button, int MouseX, int MouseY);
		bool HandleMouseLift(int Button, int MouseX, int MouseY);
		void HandleInput();

		void Jump();
		void SetCamera(CameraClass *Camera) { this->Camera = Camera; }

	private:

		// Camera
		CameraClass *Camera;

		// Jumping
		float JumpTimer;
		float TorqueFactor;

		// Graphics
		irr::scene::ILightSceneNode *Light;

		// Audio
		AudioSourceClass *Sound;
};

#endif
