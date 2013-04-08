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
#include "object.h"
#include "../engine/camera.h"

// Classes
class _Player : public _Object {

	public:

		_Player(const SpawnStruct &Object);
		~_Player();

		void Update(float FrameTime);
		void HandleInput();

		void Jump();
		void SetCamera(_Camera *Camera) { this->Camera = Camera; }

	private:

		// Camera
		_Camera *Camera;

		// Jumping
		float JumpTimer;
		float TorqueFactor;

		// Graphics
		irr::scene::ILightSceneNode *Light;

		// Audio
		_AudioSource *Sound;
};
