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
#include "scripting.h"
#include "interface.h"
#include "random.h"
#include "log.h"
#include "objectmanager.h"
#include "level.h"
#include "campaign.h"
#include "../objects/template.h"
#include "../objects/player.h"
#include "../objects/orb.h"
#include "../objects/zone.h"
#include "../objects/constraint.h"
#include "../objects/springjoint.h"
#include "../play.h"

// Controls
luaL_Reg ScriptingClass::CameraFunctions[] = {
	{"SetYaw", &ScriptingClass::CameraSetYaw},
	{"SetPitch", &ScriptingClass::CameraSetPitch},
	{NULL, NULL}
};

// Functions for creating objects
luaL_Reg ScriptingClass::ObjectFunctions[] = {
	{"GetPointer", &ScriptingClass::ObjectGetPointer},
	{"GetName", &ScriptingClass::ObjectGetName},
	{"SetPosition", &ScriptingClass::ObjectSetPosition},
	{"GetPosition", &ScriptingClass::ObjectGetPosition},
	{"Stop", &ScriptingClass::ObjectStop},
	{"SetAngularVelocity", &ScriptingClass::ObjectSetAngularVelocity},
	{"SetLifetime", &ScriptingClass::ObjectSetLifetime},
	{"Delete", &ScriptingClass::ObjectDelete},
	{NULL, NULL}
};

// Functions for manipulating orbs
luaL_Reg ScriptingClass::OrbFunctions[] = {
	{"Deactivate", &ScriptingClass::OrbDeactivate},
	{NULL, NULL}
};


// Functions for timers
luaL_Reg ScriptingClass::TimerFunctions[] = {
	{"Stamp", &ScriptingClass::TimerStamp},
	{"DelayedFunction", &ScriptingClass::TimerDelayedFunction},
	{NULL, NULL}
};

// Functions for changing levels
luaL_Reg ScriptingClass::LevelFunctions[] = {
	{"Lose", &ScriptingClass::LevelLose},
	{"Win", &ScriptingClass::LevelWin},
	{"GetTemplate", &ScriptingClass::LevelGetTemplate},
	{"CreateObject", &ScriptingClass::LevelCreateObject},
	{"CreateConstraint", &ScriptingClass::LevelCreateConstraint},
	{"CreateSpring", &ScriptingClass::LevelCreateSpring},	
	{NULL, NULL}
};

// Functions for the GUI
luaL_Reg ScriptingClass::GUIFunctions[] = {
	{"TutorialText", &ScriptingClass::GUITutorialText},
	{NULL, NULL}
};

// Functions for random number generation
luaL_Reg ScriptingClass::RandomFunctions[] = {
	{"Seed", &ScriptingClass::RandomSeed},
	{"GetFloat", &ScriptingClass::RandomGetFloat},
	{"GetInt", &ScriptingClass::RandomGetInt},
	{NULL, NULL}
};

// Functions for manipulating zones
luaL_Reg ScriptingClass::ZoneFunctions[] = {
	
	{NULL, NULL}
};

// Constructor
ScriptingClass::ScriptingClass()
:	LuaObject(NULL) {
	
}

// Initializes the scripting interface
int ScriptingClass::Init() {

	// Restart scripting system
	Reset();

	return 1;
}

// Shuts down the scripting interface
int ScriptingClass::Close() {

	// Close old lua state
	if(LuaObject != NULL)
		lua_close(LuaObject);

	return 1;
}

// Resets the scripting state
void ScriptingClass::Reset() {
	
	// Close old lua state
	if(LuaObject != NULL)
		lua_close(LuaObject);
	
	// Initialize Lua object
	LuaObject = lua_open();
	luaopen_base(LuaObject);
	luaopen_math(LuaObject);

	// Register C++ functions used by Lua
	luaL_register(LuaObject, "Camera", CameraFunctions);
	luaL_register(LuaObject, "Object", ObjectFunctions);
	luaL_register(LuaObject, "Orb", OrbFunctions);
	luaL_register(LuaObject, "Timer", TimerFunctions);
	luaL_register(LuaObject, "Level", LevelFunctions);
	luaL_register(LuaObject, "GUI", GUIFunctions);
	luaL_register(LuaObject, "Random", RandomFunctions);
	luaL_register(LuaObject, "Zone", ZoneFunctions);

	// Clean up
	KeyCallbacks.clear();
	TimedCallbacks.clear();
}

// Loads a Lua file
int ScriptingClass::LoadFile(const std::string &FilePath) {
	if(FilePath == "")
		return 0;

	// Load the file
	if(luaL_dofile(LuaObject, FilePath.c_str()) != 0) {
		Log.Write("ScriptingClass::LoadFile - failed to load script %s", FilePath.c_str());
		Log.Write("%s", lua_tostring(LuaObject, -1));
		return 0;
	}

	return 1;
}

// Defines a variable in Lua
void ScriptingClass::DefineLuaVariable(const char *VariableName, const char *Value) {
	
	lua_pushstring(LuaObject, Value);
	lua_setglobal(LuaObject, VariableName);
}

// Checks for a valid argument count
bool ScriptingClass::CheckArguments(lua_State *LuaObject, int Required) {
	int ArgumentCount = lua_gettop(LuaObject);

	// Check for arguments
	if(ArgumentCount != Required) {

		lua_Debug Record;
		lua_getstack(LuaObject, 0, &Record);
		lua_getinfo(LuaObject, "nl", &Record);

		Log.Write("Function %s requires %d arguments\n", Record.name, Required);
		return false;
	}

	return true;
}

// Calls a Lua function by name
void ScriptingClass::CallFunction(const std::string &FunctionName) {
	
	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;	
	}

	lua_call(LuaObject, 0, 0);
}

// Passes collision events to Lua
void ScriptingClass::CallCollisionHandler(const std::string &FunctionName, ObjectClass *BaseObject, ObjectClass *OtherObject) {
	
	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;	
	}
	
	lua_pushlightuserdata(LuaObject, static_cast<void *>(BaseObject));
	lua_pushlightuserdata(LuaObject, static_cast<void *>(OtherObject));
	lua_call(LuaObject, 2, 0);
}

// Calls a zone enter/exit event
void ScriptingClass::CallZoneHandler(const std::string &FunctionName, int Type, ObjectClass *Zone, ObjectClass *Object) {
	
	// Check for the function name
	lua_getglobal(LuaObject, FunctionName.c_str());
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;	
	}
	
	// Set parameters
	lua_pushinteger(LuaObject, Type);
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Zone));
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));
	lua_call(LuaObject, 3, 1);

	// Get return value for disabling the zone
	int Disable = (int)lua_tointeger(LuaObject, -1);
	if(Disable) {
		ZoneClass *ZoneObject = static_cast<ZoneClass *>(Zone);
		ZoneObject->SetActive(false);
	}
}

// Calls a Lua function from a given keyname
bool ScriptingClass::HandleKeyPress(int Key) {

	// Find function
	KeyCallbacksIterator = KeyCallbacks.find(Key);
	if(KeyCallbacksIterator != KeyCallbacks.end()) {
		CallFunction(KeyCallbacksIterator->second);
		return true;
	}

	return false;
}

// Passes mouse clicks to Lua
void ScriptingClass::HandleMousePress(int Button, int MouseX, int MouseY) {

	// Get Lua function
	lua_getglobal(LuaObject, "OnMousePress");
	if(!lua_isfunction(LuaObject, -1)) {
		lua_pop(LuaObject, 1);
		return;
	}

	// Pass parameters and call function
	lua_pushnumber(LuaObject, Button);
	lua_pushnumber(LuaObject, MouseX);
	lua_pushnumber(LuaObject, MouseY);
	lua_call(LuaObject, 3, 0);		
}

// Sets the camera's yaw value
int ScriptingClass::CameraSetYaw(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;
	
	float Yaw = (float)lua_tonumber(LuaObject, 1);

	if(PlayState::Instance()->GetCamera())
		PlayState::Instance()->GetCamera()->SetYaw(Yaw);

	return 0;
}

// Sets the camera's yaw value
int ScriptingClass::CameraSetPitch(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;
	
	float Pitch = (float)lua_tonumber(LuaObject, 1);

	if(PlayState::Instance()->GetCamera())
		PlayState::Instance()->GetCamera()->SetPitch(Pitch);

	return 0;
}

// Gets a pointer to an object from a name
int ScriptingClass::ObjectGetPointer(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameters
	std::string Name = lua_tostring(LuaObject, 1);
	ObjectClass *Object = ObjectManager::Instance().GetObjectByName(Name);

	// Pass pointer
	lua_pushlightuserdata(LuaObject, (void *)Object);

	return 1;
}

// Gets the name of an object
int ScriptingClass::ObjectGetName(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	
	if(Object != NULL)
		lua_pushstring(LuaObject, Object->GetName().c_str());

	return 1;
}

// Sets the object's position
int ScriptingClass::ObjectSetPosition(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Get parameters
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	float PositionX = (float)lua_tonumber(LuaObject, 2);
	float PositionY = (float)lua_tonumber(LuaObject, 3);
	float PositionZ = (float)lua_tonumber(LuaObject, 4);

	// Set position
	if(Object != NULL)
		Object->SetPosition(btVector3(PositionX, PositionY, PositionZ));

	return 0;
}

// Gets the object's position
int ScriptingClass::ObjectGetPosition(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get object
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	if(Object == NULL)
		return 0;

	// Send position to Lua
	lua_pushnumber(LuaObject, Object->GetPosition()[0]);
	lua_pushnumber(LuaObject, Object->GetPosition()[1]);
	lua_pushnumber(LuaObject, Object->GetPosition()[2]);

	return 3;
}

// Sets the object's lifetime
int ScriptingClass::ObjectSetLifetime(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	float Lifetime = (float)lua_tonumber(LuaObject, 2);

	// Set lifetime
	if(Object != NULL)
		Object->SetLifetime(Lifetime);

	return 0;
}

// Stops an object's movement
int ScriptingClass::ObjectStop(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Stop object
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	if(Object != NULL)
		Object->Stop();

	return 0;
}

// Sets an object's angular velocity
int ScriptingClass::ObjectSetAngularVelocity(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Get parameters
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	float X = (float)lua_tonumber(LuaObject, 2);
	float Y = (float)lua_tonumber(LuaObject, 3);
	float Z = (float)lua_tonumber(LuaObject, 4);

	if(Object != NULL)
		Object->SetAngularVelocity(btVector3(X, Y, Z));

	return 0;
}

// Deletes an object
int ScriptingClass::ObjectDelete(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Delete object
	ObjectClass *Object = (ObjectClass *)(lua_touserdata(LuaObject, 1));
	if(Object != NULL)
		ObjectManager::Instance().DeleteObject(Object);

	return 0;
}

// Deactivates an orb
int ScriptingClass::OrbDeactivate(lua_State *LuaObject) {
	
	// Validate arguments
	if(!CheckArguments(LuaObject, 3))
		return 0;

	// Get parameters
	OrbClass *Orb = (OrbClass *)(lua_touserdata(LuaObject, 1));
	std::string FunctionName = lua_tostring(LuaObject, 2);
	float Time = (float)lua_tonumber(LuaObject, 3);

	// Deactivate orb
	if(Orb != NULL && Orb->IsStillActive()) {
		Orb->StartDeactivation(FunctionName, Time);
	}

	return 0;
}

// Returns a time stamp from the start of the level
int ScriptingClass::TimerStamp(lua_State *LuaObject) {

	lua_pushnumber(LuaObject, PlayState::Instance()->GetTimer());

	return 1;
}

// Adds a timed callback
int ScriptingClass::TimerDelayedFunction(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	std::string FunctionName = lua_tostring(LuaObject, 1);
	float Time = (float)lua_tonumber(LuaObject, 2);

	// Add function to list
	Scripting::Instance().AddTimedCallback(FunctionName, Time);

	return 0;
}

// Restarts the level
int ScriptingClass::LevelLose(lua_State *LuaObject) {

	//PlayState::Instance()->InitLose();
	
	return 0;
}

// Wins the level
int ScriptingClass::LevelWin(lua_State *LuaObject) {

	PlayState::Instance()->InitWin();
	
	return 0;
}

// Gets a template from a name
int ScriptingClass::LevelGetTemplate(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameters
	std::string TemplateName = lua_tostring(LuaObject, 1);

	// Send template to Lua
	lua_pushlightuserdata(LuaObject, Level::Instance().GetTemplate(TemplateName));
	
	return 1;
}

// Creates an object from a template
int ScriptingClass::LevelCreateObject(lua_State *LuaObject) {

	// Get argument count
	int ArgumentCount = lua_gettop(LuaObject);

	// Check for arguments
	if(ArgumentCount != 5 && ArgumentCount != 8) {
		Log.Write("Function Level.CreateObject requires either 5 or 8 arguments\n");
		return false;
	}

	// Get parameters
	std::string ObjectName = lua_tostring(LuaObject, 1);
	TemplateStruct *Template = (TemplateStruct *)(lua_touserdata(LuaObject, 2));
	float PositionX = (float)lua_tonumber(LuaObject, 3);
	float PositionY = (float)lua_tonumber(LuaObject, 4);
	float PositionZ = (float)lua_tonumber(LuaObject, 5);
	float RotationX = 0.0f;
	float RotationY = 0.0f;
	float RotationZ = 0.0f;
	if(ArgumentCount > 5) {
		RotationX = (float)lua_tonumber(LuaObject, 6);
		RotationY = (float)lua_tonumber(LuaObject, 7);
		RotationZ = (float)lua_tonumber(LuaObject, 8);
	}

	// Set up spawn struct
	SpawnStruct Spawn;
	Spawn.Name = ObjectName;
	Spawn.Template = Template;
	Spawn.Position.setValue(PositionX, PositionY, PositionZ);
	Spawn.Rotation.setValue(RotationX, RotationY, RotationZ);

	// Create object
	ObjectClass *Object = Level::Instance().CreateObject(Spawn);

	// Send new object to Lua
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));

	return 1;
}

// Creates a constraint
int ScriptingClass::LevelCreateConstraint(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 4))
		return 0;

	// Set up constraint struct
	ConstraintStruct Constraint;
	Constraint.Name = lua_tostring(LuaObject, 1);
	Constraint.Template = (TemplateStruct *)(lua_touserdata(LuaObject, 2));
	Constraint.BodyA = (ObjectClass *)(lua_touserdata(LuaObject, 3));
	Constraint.BodyB = (ObjectClass *)(lua_touserdata(LuaObject, 4));

	// Create object
	ObjectClass *Object = Level::Instance().CreateConstraint(Constraint);

	// Send new object to Lua
	lua_pushlightuserdata(LuaObject, static_cast<void *>(Object));

	return 1;
}

// Creates a spring constraint
int ScriptingClass::LevelCreateSpring(lua_State *LuaObject) {

	return 1;
}

// Sets the tutorial text
int ScriptingClass::GUITutorialText(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	std::string Text(lua_tostring(LuaObject, 1));
	float Length = (float)lua_tonumber(LuaObject, 2);

	// Show text
	Interface::Instance().SetTutorialText(Text, Length);

	return 0;
}

// Sets the random seed
int ScriptingClass::RandomSeed(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 1))
		return 0;

	// Get parameter
	ulong Seed = (ulong)lua_tointeger(LuaObject, 1);

	// Set seed
	Random::Instance().SetSeed(Seed);

	return 0;
}

// Generates a random float
int ScriptingClass::RandomGetFloat(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	float Min = (float)lua_tonumber(LuaObject, 1);
	float Max = (float)lua_tonumber(LuaObject, 2);

	// Send random number to Lua
	lua_pushnumber(LuaObject, Random::Instance().GenerateRange(Min, Max));

	return 1;
}

// Generates a random integer
int ScriptingClass::RandomGetInt(lua_State *LuaObject) {

	// Validate arguments
	if(!CheckArguments(LuaObject, 2))
		return 0;

	// Get parameters
	int Min = (int)lua_tointeger(LuaObject, 1);
	int Max = (int)lua_tointeger(LuaObject, 2);

	// Send random number to Lua
	lua_pushnumber(LuaObject, Random::Instance().GenerateRange(Min, Max));

	return 1;
}

// Adds a timed callback function to the list
void ScriptingClass::AddTimedCallback(const std::string &FunctionName, float Time) {

	// Check time
	if(Time <= 0.0)
		return;

	// Create callback structure
	TimedCallbackStruct Callback;
	Callback.TimeStamp = PlayState::Instance()->GetTimer() + Time;
	Callback.Function = FunctionName;
	
	// Insert in order
	std::list<TimedCallbackStruct>::iterator Iterator;
	for(Iterator = TimedCallbacks.begin(); Iterator != TimedCallbacks.end(); ++Iterator) {
		if(Callback.TimeStamp < (*Iterator).TimeStamp)
			break;
	}

	// Add callback
	TimedCallbacks.insert(Iterator, Callback);
}

// Updates the timed callback functions
void ScriptingClass::UpdateTimedCallbacks() {

	for(std::list<TimedCallbackStruct>::iterator Iterator(TimedCallbacks.begin()); Iterator != TimedCallbacks.end(); ++Iterator) {
		if(PlayState::Instance()->GetTimer() >= (*Iterator).TimeStamp) {
			Scripting::Instance().CallFunction((*Iterator).Function);

			// Remove callback
			Iterator = TimedCallbacks.erase(Iterator);
			if(Iterator == TimedCallbacks.end())
				break;
		}
		else
			return;
	}
}

// Attaches a key to a Lua function
void ScriptingClass::AttachKeyToFunction(int Key, const std::string &FunctionName) {

	// Install callback
	KeyCallbacksIterator = KeyCallbacks.find(Key);
	if(KeyCallbacksIterator == KeyCallbacks.end())
		KeyCallbacks.insert(std::pair<int, std::string>(Key, FunctionName));
}
