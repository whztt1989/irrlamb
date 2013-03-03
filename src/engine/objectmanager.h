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
#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

// Libraries
#include "singleton.h"
#include <irrlicht.h>
#include <string>
#include <list>

// Forward Declarations
class ObjectClass;

// Classes
class ObjectManagerClass {

	public:

		ObjectManagerClass();

		int Init();
		int Close();

		void Update(float FrameTime);
		void UpdateReplay(float FrameTime);
		void UpdateFromReplay();
		void InterpolatePositions(float BlendFactor);
		void BeginFrame();
		void EndFrame();

		ObjectClass *AddObject(ObjectClass *Object);
		void DeleteObject(ObjectClass *Object);
		void DeleteObjectByID(int ID);
		ObjectClass *GetObjectByName(const std::string &Name);
		ObjectClass *GetObjectByType(int Type);
		ObjectClass *GetObjectByID(int ID);

		void ClearObjects();
		size_t GetObjectCount() const { return Objects.size(); }
		const std::list<ObjectClass *> &GetObjects() const { return Objects; }

	private:

		std::list<ObjectClass *> Objects;
		irr::u16 NextObjectID;

};

// Singletons
typedef SingletonClass<ObjectManagerClass> ObjectManager;

#endif
