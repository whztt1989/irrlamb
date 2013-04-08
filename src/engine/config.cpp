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
#include "config.h"
#include <tinyxml/tinyxml2.h>
#include "globals.h"
#include "save.h"
#include "namespace.h"

_Config Config;

using namespace tinyxml2;
 
// Initializes the config system
int _Config::Init() {

	Reset();

	return 1;
}

// Closes the config system
int _Config::Close() {
	
	return 1;
}

// Resets the configuration to the default values
void _Config::Reset() {
	
	// Video
	DriverType = EDT_OPENGL;
	ScreenWidth = 800;
	ScreenHeight = 600;
	Fullscreen = false;
	Shadows = true;
	Shaders = false;
	TrilinearFiltering = true;
	AnisotropicFiltering = 0;
	AntiAliasing = 0;

	// Audio
	AudioEnabled = 1;
	SoundVolume = 1.0;
	MusicVolume = 1.0;

	// Input
	Keys[_Actions::MOVE_FORWARD] = KEY_KEY_E;
	Keys[_Actions::MOVE_BACK] = KEY_KEY_D;
	Keys[_Actions::MOVE_LEFT] = KEY_KEY_S;
	Keys[_Actions::MOVE_RIGHT] = KEY_KEY_F;
	Keys[_Actions::RESET] = KEY_KEY_X;
	Keys[_Actions::JUMP] = KEY_SPACE;

	MouseScaleX = 1.0f;
	MouseScaleY = 1.0f;

	InvertMouse = false;

	// Replays
	ReplayInterval = 0.02f;
}

// Reads the config file
int _Config::ReadConfig() {

	// Open the XML file
	XMLDocument Document;
	if(Document.LoadFile(Save.GetConfigFile().c_str()) != XML_NO_ERROR) {
		return 0;
	}

	// Check for config tag
	XMLElement *ConfigElement = Document.FirstChildElement("config");
	if(!ConfigElement)
		return 0;

	// Check for a video tag
	XMLElement *VideoElement = ConfigElement->FirstChildElement("video");
	if(VideoElement) {
		int Value = 0;
		XMLElement *Element;

		// Get driver
		//VideoElement->QueryIntAttribute("driver", (int *)(&DriverType));

		// Check for the screen tag
		Element = VideoElement->FirstChildElement("screen");
		if(Element) {

			// Get screen attributes
			Element->QueryIntAttribute("width", &ScreenWidth);
			Element->QueryIntAttribute("height", &ScreenHeight);
			Element->QueryIntAttribute("fullscreen", &Value);
			Fullscreen = !!Value;
		}

		// Check for the filtering tag
		Element = VideoElement->FirstChildElement("filtering");
		if(Element) {

			// Get screen attributes
			Element->QueryIntAttribute("trilinear", &Value);
			TrilinearFiltering = !!Value;
			Element->QueryIntAttribute("anisotropic", &AnisotropicFiltering);
			Element->QueryIntAttribute("antialiasing", &AntiAliasing);
		}

		// Check for the shadow tag
		Element = VideoElement->FirstChildElement("shadows");
		if(Element) {

			// Get screen attributes
			Element->QueryIntAttribute("enabled", &Value);
			Shadows = !!Value;
		}

		// Check for the shader tag
		Element = VideoElement->FirstChildElement("shaders");
		if(Element) {

			// Get screen attributes
			Element->QueryIntAttribute("enabled", &Value);
			Shaders = !!Value;
		}
	}

	// Check for the audio tag
	XMLElement *AudioElement = ConfigElement->FirstChildElement("audio");
	if(AudioElement) {
		int Value = 0;
		
		// Get audio attributes
		AudioElement->QueryIntAttribute("enabled", &Value);
		AudioEnabled = !!Value;
		
		/*
		// Get sound element
		XMLElement *SoundElement = AudioElement->FirstChildElement("sound");
		if(!SoundElement) {

			// Get sound attributes
			SoundElement->QueryFloatAttribute("volume", &SoundVolume);
		}

		// Get music element
		XMLElement *MusicElement = AudioElement->FirstChildElement("music");
		if(!MusicElement) {

			// Get music attributes
			MusicElement->QueryFloatAttribute("volume", &MusicVolume);
		}
		*/
	}

	// Get input element
	XMLElement *InputElement = ConfigElement->FirstChildElement("input");
	if(InputElement) {
		int Value = 0;

		// Get mouse attributes
		InputElement->QueryFloatAttribute("mousex", &MouseScaleX);

		// Get mouse attributes
		InputElement->QueryFloatAttribute("mousey", &MouseScaleY);

		// Get mouse invert
		InputElement->QueryIntAttribute("invert", &Value);
		InvertMouse = !!Value;
	}

	// Get keyboard mapping
	for(XMLElement *ActionElement = InputElement->FirstChildElement("action"); ActionElement != 0; ActionElement = ActionElement->NextSiblingElement("action")) {
	
		// Get action type
		int ActionType;
		if(ActionElement->QueryIntAttribute("type", &ActionType) != XML_NO_ERROR)
			continue;

		// Get key
		int KeyCode;
		if(ActionElement->QueryIntAttribute("key", &KeyCode) != XML_NO_ERROR)
			continue;

		// Assign key
		if(ActionType >= 0 && ActionType < _Actions::COUNT && KeyCode >= 0 && KeyCode < KEY_KEY_CODES_COUNT)
			Keys[ActionType] = (EKEY_CODE)KeyCode;
		
	}

	// Replays
	XMLElement *ReplayElement = ConfigElement->FirstChildElement("replay");
	if(ReplayElement) {
		ReplayElement->QueryFloatAttribute("interval", (float *)(&ReplayInterval));
	}

	return 1;
}

// Writes the config file
int _Config::WriteConfig() {

	XMLDocument Document;
	Document.InsertEndChild(Document.NewDeclaration());
	
	// Config
	XMLElement *ConfigElement = Document.NewElement("config");
	ConfigElement->SetAttribute("version", "1.0");
	Document.InsertEndChild(ConfigElement);
	
	// Create video element
	XMLElement *VideoElement = Document.NewElement("video");
	//VideoElement->SetAttribute("driver", DriverType);
	ConfigElement->LinkEndChild(VideoElement);

	// Screen settings
	XMLElement *ScreenElement = Document.NewElement("screen");
	ScreenElement->SetAttribute("width", ScreenWidth);
	ScreenElement->SetAttribute("height", ScreenHeight);
	ScreenElement->SetAttribute("fullscreen", Fullscreen);
	ConfigElement->InsertEndChild(ScreenElement);

	// Filtering
	XMLElement *FilteringElement = Document.NewElement("filtering");
	FilteringElement->SetAttribute("trilinear", TrilinearFiltering);
	FilteringElement->SetAttribute("anisotropic", AnisotropicFiltering);
	FilteringElement->SetAttribute("antialiasing", AntiAliasing);
	VideoElement->LinkEndChild(FilteringElement);

	// Shadows
	XMLElement *ShadowsElement = Document.NewElement("shadows");
	ShadowsElement->SetAttribute("enabled", Shadows);
	VideoElement->LinkEndChild(ShadowsElement);

	// Shaders
	XMLElement *ShadersElement = Document.NewElement("shaders");
	ShadersElement->SetAttribute("enabled", Shaders);
	VideoElement->LinkEndChild(ShadersElement);

	// Create audio element
	XMLElement *AudioElement = Document.NewElement("audio");
	AudioElement->SetAttribute("enabled", AudioEnabled);
	ConfigElement->LinkEndChild(AudioElement);

	// Sound
	//XMLElement *SoundElement = Document.NewElement("sound");
	//SoundElement->SetDoubleAttribute("volume", SoundVolume);
	//AudioElement->LinkEndChild(SoundElement);

	// Music
	//XMLElement *MusicElement = Document.NewElement("music");
	//MusicElement->SetDoubleAttribute("volume", MusicVolume);
	//AudioElement->LinkEndChild(MusicElement);

	// Input
	XMLElement *InputElement = Document.NewElement("input");
	//Document.pus
	InputElement->SetAttribute("mousex", 1.0);
	InputElement->SetAttribute("mousey", 1.0);
	InputElement->SetAttribute("invert", InvertMouse);
	ConfigElement->LinkEndChild(InputElement);

	// Actions
	for(int i = 0; i < _Actions::COUNT; i++) {
		XMLElement *ActionElement = Document.NewElement("action");
		ActionElement->SetAttribute("type", i);
		ActionElement->SetAttribute("key", Keys[i]);
		InputElement->LinkEndChild(ActionElement);
	}

	// Replays
	XMLElement *ReplayElement = Document.NewElement("replay");
	ReplayElement->SetAttribute("interval", ReplayInterval);
	ConfigElement->LinkEndChild(ReplayElement);

	// Write file
	Document.SaveFile(Save.GetConfigFile().c_str());
	
	return 1;
}
