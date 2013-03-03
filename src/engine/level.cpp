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
#include "level.h"
#include "../tinyxml/tinyxml.h"
#include "globals.h"
#include "game.h"
#include "objectmanager.h"
#include "scripting.h"
#include "graphics.h"
#include "save.h"
#include "replay.h"
#include "log.h"
#include "physics.h"
#include "input.h"
#include "filestream.h"
#include "config.h"
#include "../objects/template.h"
#include "../objects/player.h"
#include "../objects/sphere.h"
#include "../objects/box.h"
#include "../objects/orb.h"
#include "../objects/cylinder.h"
#include "../objects/zone.h"
#include "../objects/collision.h"
#include "../objects/constraint.h"
#include "../objects/springjoint.h"
#include "namespace.h"

// Loads a level file
int LevelClass::Init(const std::string &LevelName) {
	IsCustomLevel = false;
	Scripts.clear();
	Scripts.push_back(Game::Instance().GetWorkingPath() + "scripts/default.lua");

	Log.Write("LevelClass::Init - Loading level: %s", LevelName.c_str());
	
	// Get paths
	this->LevelName = LevelName;
	std::string LevelFile = LevelName + "/" + LevelName + ".xml";
	std::string FilePath = Game::Instance().GetWorkingPath() + std::string("levels/") + LevelFile;
	std::string CustomFilePath = Save::Instance().GetCustomLevelsPath() + LevelFile;
	std::string DataPath = Game::Instance().GetWorkingPath() + std::string("levels/") + LevelName + "/";

	// See if custom level exists first
	std::ifstream CustomLevelExists(CustomFilePath.c_str());
	if(CustomLevelExists) {
		IsCustomLevel = true;
		FilePath = CustomFilePath;
		DataPath = Save::Instance().GetCustomLevelsPath() + LevelName + "/";
	}
	CustomLevelExists.close();

	// Open the XML file
	TiXmlDocument Document(FilePath.c_str());
	if(!Document.LoadFile()) {
		Log.Write("line %d column %d: %s", Document.ErrorRow(), Document.ErrorCol(), Document.ErrorDesc());
		Close();
		return 0;
	}

	// Check for level tag
	TiXmlElement *LevelElement = Document.FirstChildElement("level");
	if(!LevelElement) {
		Log.Write("Could not find level tag");
		Close();
		return 0;
	}

	// Level version
	if(LevelElement->QueryIntAttribute("version", &LevelVersion) != TIXML_SUCCESS) {
		Log.Write("Could not find level version");
		Close();
		return 0;
	}

	// Check required game version
	GameVersion = LevelElement->Attribute("gameversion");
	if(GameVersion == "") {
		Log.Write("Could not find game version attribute");
		Close();
		return 0;
	}

	// Options
	bool Fog = false;
	bool EmitLight = false;

	// Load options
	TiXmlElement *OptionsElement = LevelElement->FirstChildElement("options");
	if(OptionsElement) {
		int Value;

		// Does the player emit light?
		TiXmlElement *EmitLightElement = OptionsElement->FirstChildElement("emitlight");
		if(EmitLightElement) {
			EmitLightElement->QueryIntAttribute("enabled", &Value);
			EmitLight = Value!=0;
		}
	}

	// Load world
	TiXmlElement *ResourcesElement = LevelElement->FirstChildElement("resources");
	if(ResourcesElement) {

		// Load scenes
		for(TiXmlElement *SceneElement = ResourcesElement->FirstChildElement("scene"); SceneElement != 0; SceneElement = SceneElement->NextSiblingElement("scene")) {

			// Get file
			std::string File = SceneElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on scene");
				Close();
				return 0;
			}

			// Load scene
			if(IsCustomLevel) {
				irrFile->changeWorkingDirectoryTo(DataPath.c_str());
				irrScene->loadScene(File.c_str());
				irrFile->changeWorkingDirectoryTo(Game::Instance().GetWorkingPath().c_str());
			}
			else {
				irrScene->loadScene((DataPath + File).c_str());
			}

			// Set texture filters on meshes in the scene
			array<ISceneNode *> MeshNodes;
			irrScene->getSceneNodesFromType(ESNT_MESH, MeshNodes);
			for(u32 i = 0; i < MeshNodes.size(); i++) {
				if(EmitLight && Config::Instance().Shaders)
					MeshNodes[i]->setMaterialType((E_MATERIAL_TYPE)Graphics::Instance().GetCustomMaterial());

				MeshNodes[i]->setMaterialFlag(EMF_TRILINEAR_FILTER, Config::Instance().TrilinearFiltering);
				for(u32 j = 0; j < MeshNodes[i]->getMaterialCount(); j++) {
					for(int k = 0; k < 4; k++) {
						MeshNodes[i]->getMaterial(j).TextureLayer[k].AnisotropicFilter = Config::Instance().AnisotropicFiltering;
						if(MeshNodes[i]->getMaterial(j).FogEnable)
							Fog = true;
					}
				}
			}
		}

		// Load collision
		for(TiXmlElement *CollisionElement = ResourcesElement->FirstChildElement("collision"); CollisionElement != 0; CollisionElement = CollisionElement->NextSiblingElement("collision")) {

			// Get file
			std::string File = CollisionElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on collision");
				Close();
				return 0;
			}

			// Create template
			TemplateStruct *Template = new TemplateStruct;
			Template->CollisionFile = DataPath + File;
			Template->Type = ObjectClass::COLLISION;
			Template->Mass = 0.0f;
			Templates.push_back(Template);

			// Create spawn
			SpawnStruct *ObjectSpawn = new SpawnStruct;
			ObjectSpawn->Template = Template;
			ObjectSpawns.push_back(ObjectSpawn);
		}

		// Load scripts
		for(TiXmlElement *ScriptElement = ResourcesElement->FirstChildElement("script"); ScriptElement != 0; ScriptElement = ScriptElement->NextSiblingElement("script")) {
			
			// Get file
			std::string File = ScriptElement->Attribute("file");
			if(File == "") {
				Log.Write("Could not find file attribute on script");
				Close();
				return 0;
			}

			Scripts.push_back(DataPath + File);
		}
	}

	// Load templates
	TiXmlElement *TemplatesElement = LevelElement->FirstChildElement("templates");
	if(TemplatesElement) {
		int TemplateID = 0;
		for(TiXmlElement *TemplateElement = TemplatesElement->FirstChildElement(); TemplateElement != 0; TemplateElement = TemplateElement->NextSiblingElement()) {
			
			// Create a template
			TemplateStruct *Template = new TemplateStruct;

			// Get the template properties
			if(!GetTemplateProperties(TemplateElement, *Template))
				return 0;

			// Assign options
			Template->TemplateID = TemplateID;
			Template->Fog = Fog;
			if(EmitLight) {
				
				// Use shaders on materials that receive light
				if(Config::Instance().Shaders)
					Template->CustomMaterial = Graphics::Instance().GetCustomMaterial();

				// Set the player to emit light
				if(Template->Type == ObjectClass::PLAYER)
					Template->EmitLight = true;
			}
			TemplateID++;

			// Store for later
			Templates.push_back(Template);
		}
	}

	// Load object spawns
	TiXmlElement *ObjectsElement = LevelElement->FirstChildElement("objects");
	if(ObjectsElement) {
		for(TiXmlElement *ObjectElement = ObjectsElement->FirstChildElement(); ObjectElement != 0; ObjectElement = ObjectElement->NextSiblingElement()) {
			
			// Create an object spawn
			SpawnStruct *ObjectSpawn = new SpawnStruct;

			// Get the object properties
			if(!GetObjectSpawnProperties(ObjectElement, *ObjectSpawn))
				return 0;

			// Store for later
			ObjectSpawns.push_back(ObjectSpawn);
		}
	}

	return 1;
}

// Closes the level
int LevelClass::Close() {

	// Clear scripts
	Scripts.clear();

	// Delete templates
	for(size_t i = 0; i < Templates.size(); i++) {
		delete Templates[i];
	}
	Templates.clear();

	// Delete object spawn data
	for(size_t i = 0; i < ObjectSpawns.size(); i++) {
		delete ObjectSpawns[i];
	}
	ObjectSpawns.clear();

	return 1;
}

// Processes a template tag
int LevelClass::GetTemplateProperties(TiXmlElement *TemplateElement, TemplateStruct &Object) {
	TiXmlElement *Element;
	const char *String;

	// Get type
	std::string ObjectType = TemplateElement->Value();

	// Get name
	String = TemplateElement->Attribute("name");
	if(!String) {
		Log.Write("Template is missing name");
		return 0;
	}
	Object.Name = String;

	// Get lifetime
	TemplateElement->QueryFloatAttribute("lifetime", &Object.Lifetime);

	// Get scale
	Element = TemplateElement->FirstChildElement("shape");
	if(Element) {
		Element->QueryFloatAttribute("w", &Object.Shape[0]);
		Element->QueryFloatAttribute("h", &Object.Shape[1]);
		Element->QueryFloatAttribute("l", &Object.Shape[2]);
		Element->QueryFloatAttribute("r", &Object.Radius);
	}

	// Get mesh properties
	Element = TemplateElement->FirstChildElement("mesh");
	if(Element) {
		String = Element->Attribute("file");
		if(String)
			Object.Mesh = String;

		Element->QueryFloatAttribute("scale", &Object.Scale);
	}

	// Get physical attributes
	Element = TemplateElement->FirstChildElement("physics");
	if(Element) {
		Element->QueryFloatAttribute("mass", &Object.Mass);
		Element->QueryFloatAttribute("friction", &Object.Friction);
	}

	// Get damping
	Element = TemplateElement->FirstChildElement("damping");
	if(Element) {
		Element->QueryFloatAttribute("linear", &Object.LinearDamping);
		Element->QueryFloatAttribute("angular", &Object.AngularDamping);
	}

	// Get limits for constraints. 0-1 is linear, 2-3 is angular
	Element = TemplateElement->FirstChildElement("limit");
	if(Element) {
		int Type = 0;
		Element->QueryIntAttribute("type", &Type);
		Element->QueryFloatAttribute("x", &Object.ConstraintData[Type][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[Type][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[Type][2]);
	}

	// Get pivot for constraints
	Element = TemplateElement->FirstChildElement("pivot");
	if(Element) {
		Element->QueryFloatAttribute("x", &Object.ConstraintData[0][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[0][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[0][2]);
	}

	// Get axis for constraints
	Element = TemplateElement->FirstChildElement("axis");
	if(Element) {
		Element->QueryFloatAttribute("x", &Object.ConstraintData[1][0]);
		Element->QueryFloatAttribute("y", &Object.ConstraintData[1][1]);
		Element->QueryFloatAttribute("z", &Object.ConstraintData[1][2]);
	}

	// Get texture
	Element = TemplateElement->FirstChildElement("texture");
	if(Element) {
		for(int i = 0; i < 4; i++) {
			char AttributeName[16];
			sprintf(AttributeName, "t%d", i+1);

			const char *Attribute = Element->Attribute(AttributeName);
			if(Attribute) {
				Object.Textures[i] = Game::Instance().GetWorkingPath() + std::string("textures/") + Attribute;
				if(!irrFile->existFile(Object.Textures[i].c_str())) {
					Log.Write("Texture file does not exist: %s", Object.Textures[i].c_str());
					return 0;
				}
			}
		}
	}

	// Validate objects
	if(ObjectType == "player") {
		Object.Type = ObjectClass::PLAYER;
	}
	else if(ObjectType == "orb") {
		Object.Type = ObjectClass::ORB;
	}
	else if(ObjectType == "sphere") {
		Object.Type = ObjectClass::SPHERE;
	}
	else if(ObjectType == "box") {
		Object.Type = ObjectClass::BOX;
	}
	else if(ObjectType == "cylinder") {
		Object.Type = ObjectClass::CYLINDER;
	}
	else if(ObjectType == "zone") {
		Object.Type = ObjectClass::ZONE;
		Object.Mass = 0.0f;
		Object.CollisionGroup = PhysicsClass::FILTER_ZONE;
		Object.CollisionMask = PhysicsClass::FILTER_RIGIDBODY | PhysicsClass::FILTER_KINEMATIC;
	}
	else if(ObjectType == "d6") {
		Object.Type = ObjectClass::CONSTRAINT_D6;
	}
	else if(ObjectType == "hinge") {
		Object.Type = ObjectClass::CONSTRAINT_HINGE;
	}

	// If a rigidbody's mass is zero, set group to static
	if(Object.Mass == 0.0f && Object.CollisionGroup == PhysicsClass::FILTER_RIGIDBODY) {
		Object.CollisionGroup = PhysicsClass::FILTER_STATIC | PhysicsClass::FILTER_CAMERA;

		// Prevent collision with other static object
		Physics::Instance().RemoveFilter(Object.CollisionMask, PhysicsClass::FILTER_STATIC);
	}
	
	return 1;
}

// Processes an object tag
int LevelClass::GetObjectSpawnProperties(TiXmlElement *ObjectElement, SpawnStruct &ObjectSpawn) {
	TiXmlElement *Element;

	// Get name
	ObjectSpawn.Name = ObjectElement->Attribute("name");
	if(ObjectSpawn.Name == "") {
		Log.Write("Object is missing name");
		return 0;
	}

	// Get template name
	std::string TemplateName = ObjectElement->Attribute("template");
	if(TemplateName == "") {
		Log.Write("Object is missing template name");
		return 0;
	}

	// Get template data
	TemplateStruct *Template = GetTemplate(TemplateName);
	if(Template == NULL) {
		Log.Write("Cannot find template %s", TemplateName.c_str());
		return 0;
	}
	ObjectSpawn.Template = Template;
	
	// Get position
	Element = ObjectElement->FirstChildElement("position");
	if(Element) {
		Element->QueryFloatAttribute("x", &ObjectSpawn.Position[0]);
		Element->QueryFloatAttribute("y", &ObjectSpawn.Position[1]);
		Element->QueryFloatAttribute("z", &ObjectSpawn.Position[2]);
	}

	// Get euler rotation
	Element = ObjectElement->FirstChildElement("rotation");
	if(Element) {
		Element->QueryFloatAttribute("x", &ObjectSpawn.Rotation[0]);
		Element->QueryFloatAttribute("y", &ObjectSpawn.Rotation[1]);
		Element->QueryFloatAttribute("z", &ObjectSpawn.Rotation[2]);
	}

	return 1;
}

// Spawns all of the objects in the level
void LevelClass::SpawnObjects() {

	for(size_t i = 0; i < ObjectSpawns.size(); i++) {
		SpawnStruct *Spawn = ObjectSpawns[i];
		CreateObject(*Spawn);
	}
}

// Creates an object from a spawn struct
ObjectClass *LevelClass::CreateObject(const SpawnStruct &Object) {
	
	// Add object
	ObjectClass *NewObject = NULL;
	switch(Object.Template->Type) {
		case ObjectClass::PLAYER:
			NewObject = ObjectManager::Instance().AddObject(new PlayerClass(Object));
		break;
		case ObjectClass::ORB:
			NewObject = ObjectManager::Instance().AddObject(new OrbClass(Object));
		break;
		case ObjectClass::COLLISION:
			NewObject = ObjectManager::Instance().AddObject(new CollisionClass(Object));
		break;
		case ObjectClass::SPHERE:
			NewObject = ObjectManager::Instance().AddObject(new SphereClass(Object));
		break;
		case ObjectClass::BOX:
			NewObject = ObjectManager::Instance().AddObject(new BoxClass(Object));
		break;
		case ObjectClass::CYLINDER:
			NewObject = ObjectManager::Instance().AddObject(new CylinderClass(Object));
		break;
		case ObjectClass::ZONE:
			NewObject = ObjectManager::Instance().AddObject(new ZoneClass(Object));
		break;
	}

	// Record replay event
	if(Replay::Instance().IsRecording() && Object.Template->TemplateID != -1) {
		
		// Write replay information
		FileClass &ReplayStream = Replay::Instance().GetReplayStream();
		Replay::Instance().WriteEvent(ReplayClass::PACKET_CREATE);
		ReplayStream.WriteShortInt(Object.Template->TemplateID);
		ReplayStream.WriteShortInt(NewObject->GetID());
		ReplayStream.WriteData((void *)&Object.Position, sizeof(btScalar) * 3);
		ReplayStream.WriteData((void *)&Object.Rotation, sizeof(btScalar) * 3);
	}
	
	return NewObject;
}

// Creates a constraint from a template
ObjectClass *LevelClass::CreateConstraint(const ConstraintStruct &Object) {
	
	// Add object
	ObjectClass *NewObject = ObjectManager::Instance().AddObject(new ConstraintClass(Object));

	return NewObject;
}

// Gets a template by name
TemplateStruct *LevelClass::GetTemplate(const std::string &Name) {

	// Search templates by name
	for(size_t i = 0; i < Templates.size(); i++) {
		if(Templates[i]->Name == Name) {
			return Templates[i];
		}
	}

	return NULL;
}

// Gets a template by name
TemplateStruct *LevelClass::GetTemplateFromID(int ID) {

	// Search templates by id
	for(size_t i = 0; i < Templates.size(); i++) {
		if(Templates[i]->TemplateID == ID) {
			return Templates[i];
		}
	}

	return NULL;
}

// Runs the level's scripts
void LevelClass::RunScripts() {
	
	// Reset Lua state
	Scripting::Instance().Reset();

	// Add key names to lua scope
	Scripting::Instance().DefineLuaVariable("KEY_FORWARD", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::MOVEFORWARD]));
	Scripting::Instance().DefineLuaVariable("KEY_BACK", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::MOVEBACK]));
	Scripting::Instance().DefineLuaVariable("KEY_LEFT", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::MOVELEFT]));
	Scripting::Instance().DefineLuaVariable("KEY_RIGHT", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::MOVERIGHT]));
	Scripting::Instance().DefineLuaVariable("KEY_RESET", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::RESET]));
	Scripting::Instance().DefineLuaVariable("KEY_JUMP", Input::Instance().GetKeyName(Config::Instance().Keys[ACTIONS::JUMP]));
	
	// Run scripts
	for(u32 i = 0; i < Scripts.size(); i++) {
		
		// Load a level
		Scripting::Instance().LoadFile(Scripts[i]);
	}
}
