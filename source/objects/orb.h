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
#ifndef ORB_H
#define ORB_H

// Libraries
#include "object.h"

// Classes
class OrbClass : public ObjectClass {

	public:

		enum OrbStateType {
			ORBSTATE_NORMAL,
			ORBSTATE_DEACTIVATING,
			ORBSTATE_DEACTIVATED,
		};

		OrbClass(const SpawnStruct &Object);
		~OrbClass();

		void Update(float FrameTime);
		void UpdateReplay(float FrameTime);

		void StartDeactivation(const std::string &TCallback, float Length);
		bool IsStillActive() const { return State == ORBSTATE_NORMAL; }

	private:

		void UpdateDeactivation(float FrameTime);

		// Graphics
		irr::video::SColor GlowColor;
		irr::scene::IBillboardSceneNode *InnerNode;

		// Deactivation
		std::string DeactivationCallback;
		int State;
		float OrbTime, DeactivateLength;

		// Audio
		AudioSourceClass *Sound;
};

#endif
