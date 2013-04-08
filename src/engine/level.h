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
#include <stdafx.h>
#pragma once

// Forward Declarations
class TiXmlElement;
class _Object;
struct TemplateStruct;
struct SpawnStruct;
struct ConstraintStruct;

// Classes
class _Level {

	public:

		int Init(const std::string &LevelName, bool HeaderOnly=false);
		int Close();

		// Properties
		const std::string &GetLevelName() { return LevelName; }
		const std::string &GetLevelNiceName() { return LevelNiceName; }
		int GetLevelVersion() { return LevelVersion; }

		// Objects
		void SpawnObjects();
		_Object *CreateObject(const SpawnStruct &Object);
		_Object *CreateConstraint(const ConstraintStruct &Object);

		// Templates
		TemplateStruct *GetTemplate(const std::string &Name);
		TemplateStruct *GetTemplateFromID(int ID);

		// Scripts
		void RunScripts();

	private:

		// Loading
		int GetTemplateProperties(TiXmlElement *TemplateElement, TemplateStruct &Object);
		int GetObjectSpawnProperties(TiXmlElement *ObjectElement, SpawnStruct &ObjectSpawn);

		// Level
		std::string LevelName, LevelNiceName;
		int LevelVersion;
		bool IsCustomLevel;
		std::string GameVersion;

		// Resources
		std::vector<std::string> Scripts;

		// Objects
		std::vector<TemplateStruct *> Templates;
		std::vector<SpawnStruct *> ObjectSpawns;
};

// Singletons
extern _Level Level;
