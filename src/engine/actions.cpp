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
#include "actions.h"
#include "game.h"
#include "state.h"
#include <tinyxml/tinyxml2.h>

_Actions Actions;

using namespace tinyxml2;

// Constructor
_Actions::_Actions() {
	ResetState();
}

// Reset the state
void _Actions::ResetState() {
	for(int i = 0; i < ACTIONS_MAX; i++) {
		State[i] = 0.0f;
	}
}

// Clear key and mouse map
void _Actions::ClearMappings() {
	for(int i = 0; i < ACTIONS_MAXKEYS; i++)
		KeyMap[i].clear();

	for(int i = 0; i < ACTIONS_MAXMOUSEBUTTONS; i++)
		MouseButtonMap[i].clear();

	for(int i = 0; i < ACTIONS_MAXMOUSEAXIS; i++)
		MouseAxisMap[i].clear();

	for(int i = 0; i < ACTIONS_MAXJOYSTICKBUTTONS; i++)
		JoystickButtonMap[i].clear();

	for(int i = 0; i < ACTIONS_MAXJOYSTICKAXIS; i++)
		JoystickAxisMap[i].clear();
}

// Get action
float _Actions::GetState(int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX)
		return 0.0f;

	return State[Action];
}

// Add to the key=>action map
void _Actions::AddKeyMap(int Key, int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX || Key < 0 || Key >= ACTIONS_MAXKEYS)
		return;

	KeyMap[Key].push_back(Action);
}

// Add to the mouse button=>action map
void _Actions::AddMouseButtonMap(int Button, int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX || Button < 0 || Button >= ACTIONS_MAXMOUSEBUTTONS)
		return;

	MouseButtonMap[Button].push_back(Action);
}

// Map a mouse axis to an action
void _Actions::AddMouseAxisMap(int Axis, int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX || Axis < 0 || Axis >= ACTIONS_MAXMOUSEAXIS)
		return;

	MouseAxisMap[Axis].push_back(Action);
}

// Add to the joystick button=>action map
void _Actions::AddJoystickButtonMap(int Button, int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX || Button < 0 || Button >= ACTIONS_MAXJOYSTICKBUTTONS)
		return;

	JoystickButtonMap[Button].push_back(Action);
}

// Map a joystick axis to an action
void _Actions::AddJoystickAxisMap(int Axis, int Action) {
	if(Action < 0 || Action >= ACTIONS_MAX || Axis < 0 || Axis >= ACTIONS_MAXJOYSTICKAXIS)
		return;

	JoystickAxisMap[Axis].push_back(Action);
}

// Handle keys
void _Actions::KeyEvent(int Key, bool Pressed) {
	if(Key < 0 || Key >= ACTIONS_MAXKEYS)
		return;

	for(MapIterator = KeyMap[Key].begin(); MapIterator != KeyMap[Key].end(); MapIterator++) {
		State[*MapIterator] = (float)Pressed;
		Game.GetState()->HandleAction(*MapIterator, Pressed);
	}
}

// Handle mouse
void _Actions::MouseButtonEvent(int Button, bool Pressed) {
	if(Button < 0 || Button >= ACTIONS_MAXMOUSEBUTTONS)
		return;

	for(MapIterator = MouseButtonMap[Button].begin(); MapIterator != MouseButtonMap[Button].end(); MapIterator++) {
		State[*MapIterator] = (float)Pressed;
		Game.GetState()->HandleAction(*MapIterator, Pressed);
	}
}

// Handle mouse movement
void _Actions::MouseAxisEvent(int Axis, float Value) {
	if(Axis < 0 || Axis >= ACTIONS_MAXMOUSEAXIS)
		return;

	for(MapIterator = MouseAxisMap[Axis].begin(); MapIterator != MouseAxisMap[Axis].end(); MapIterator++) {
		State[*MapIterator] = Value;
		Game.GetState()->HandleAction(*MapIterator, Value);
	}
}

// Handle joystick button
void _Actions::JoystickButtonEvent(int Button, bool Pressed) {
	if(Button < 0 || Button >= ACTIONS_MAXJOYSTICKBUTTONS)
		return;

	for(MapIterator = JoystickButtonMap[Button].begin(); MapIterator != JoystickButtonMap[Button].end(); MapIterator++) {
		State[*MapIterator] = (float)Pressed;
		Game.GetState()->HandleAction(*MapIterator, Pressed);
	}
}

// Handle joystick axis
void _Actions::JoystickAxisEvent(int Axis, float Value) {
	if(Axis < 0 || Axis >= ACTIONS_MAXJOYSTICKAXIS)
		return;

	for(MapIterator = JoystickAxisMap[Axis].begin(); MapIterator != JoystickAxisMap[Axis].end(); MapIterator++) {
		State[*MapIterator] = Value;
		Game.GetState()->HandleAction(*MapIterator, Value);
	}
}

// Write to config file
void _Actions::Serialize(XMLDocument &Document, XMLElement *InputElement) {
	for(int i = 0; i < ACTIONS_MAXKEYS; i++) {
		for(MapIterator = KeyMap[i].begin(); MapIterator != KeyMap[i].end(); MapIterator++) {
			XMLElement *Element = Document.NewElement("key");
			Element->SetAttribute("action", *MapIterator);
			Element->SetAttribute("key", i);
			InputElement->InsertEndChild(Element);
		}
	}

	for(int i = 0; i < ACTIONS_MAXMOUSEBUTTONS; i++) {
		for(MapIterator = MouseButtonMap[i].begin(); MapIterator != MouseButtonMap[i].end(); MapIterator++) {
			XMLElement *Element = Document.NewElement("mousebutton");
			Element->SetAttribute("action", *MapIterator);
			Element->SetAttribute("button", i);
			InputElement->InsertEndChild(Element);
		}
	}

	for(int i = 0; i < ACTIONS_MAXMOUSEAXIS; i++) {
		for(MapIterator = MouseAxisMap[i].begin(); MapIterator != MouseAxisMap[i].end(); MapIterator++) {
			XMLElement *Element = Document.NewElement("mouseaxis");
			Element->SetAttribute("action", *MapIterator);
			Element->SetAttribute("axis", i);
			InputElement->InsertEndChild(Element);
		}
	}

	for(int i = 0; i < ACTIONS_MAXJOYSTICKBUTTONS; i++) {
		for(MapIterator = JoystickButtonMap[i].begin(); MapIterator != JoystickButtonMap[i].end(); MapIterator++) {
			XMLElement *Element = Document.NewElement("joybutton");
			Element->SetAttribute("action", *MapIterator);
			Element->SetAttribute("button", i);
			InputElement->InsertEndChild(Element);
		}
	}

	for(int i = 0; i < ACTIONS_MAXJOYSTICKAXIS; i++) {
		for(MapIterator = JoystickAxisMap[i].begin(); MapIterator != JoystickAxisMap[i].end(); MapIterator++) {
			XMLElement *Element = Document.NewElement("joyaxis");
			Element->SetAttribute("action", *MapIterator);
			Element->SetAttribute("axis", i);
			InputElement->InsertEndChild(Element);
		}
	}
}

// Unserialize
void _Actions::Unserialize(XMLElement *InputElement) {
	int Action, Value;

	// Get keyboard mapping
	for(XMLElement *Element = InputElement->FirstChildElement("key"); Element != 0; Element = Element->NextSiblingElement("key")) {
		if(Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR || Element->QueryIntAttribute("key", &Value) != XML_NO_ERROR)
			continue;

		Actions.AddKeyMap(Value, Action);
	}

	// Get mouse button mapping
	for(XMLElement *Element = InputElement->FirstChildElement("mousebutton"); Element != 0; Element = Element->NextSiblingElement("mousebutton")) {
		if(Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR || Element->QueryIntAttribute("button", &Value) != XML_NO_ERROR)
			continue;

		Actions.AddMouseButtonMap(Value, Action);
	}

	// Get mouse axis mapping
	for(XMLElement *Element = InputElement->FirstChildElement("mouseaxis"); Element != 0; Element = Element->NextSiblingElement("mouseaxis")) {
		if(Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR || Element->QueryIntAttribute("axis", &Value) != XML_NO_ERROR)
			continue;

		Actions.AddMouseAxisMap(Value, Action);
	}

	// Get joystick button mapping
	for(XMLElement *Element = InputElement->FirstChildElement("joybutton"); Element != 0; Element = Element->NextSiblingElement("joybutton")) {
		if(Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR || Element->QueryIntAttribute("button", &Value) != XML_NO_ERROR)
			continue;

		Actions.AddJoystickButtonMap(Value, Action);
	}

	// Get joystick axis mapping
	for(XMLElement *Element = InputElement->FirstChildElement("joyaxis"); Element != 0; Element = Element->NextSiblingElement("joyaxis")) {
		if(Element->QueryIntAttribute("action", &Action) != XML_NO_ERROR || Element->QueryIntAttribute("axis", &Value) != XML_NO_ERROR)
			continue;

		Actions.AddJoystickAxisMap(Value, Action);
	}
}
