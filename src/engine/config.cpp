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
#include "../tinyxml/tinyxml.h"
#include "globals.h"
#include "save.h"
#include "namespace.h"

_Config Config;
 
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
	TiXmlDocument Document(Save.GetConfigFile().c_str());
	if(!Document.LoadFile()) {
		return 0;
	}

	// Check for config tag
	TiXmlElement *ConfigElement = Document.FirstChildElement("config");
	if(!ConfigElement)
		return 0;

	// Check for a video tag
	TiXmlElement *VideoElement = ConfigElement->FirstChildElement("video");
	if(VideoElement) {
		int Value = 0;
		TiXmlElement *Element;

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
	TiXmlElement *AudioElement = ConfigElement->FirstChildElement("audio");
	if(AudioElement) {
		int Value = 0;
		
		// Get audio attributes
		AudioElement->QueryIntAttribute("enabled", &Value);
		AudioEnabled = !!Value;
		
		/*
		// Get sound element
		TiXmlElement *SoundElement = AudioElement->FirstChildElement("sound");
		if(!SoundElement) {

			// Get sound attributes
			SoundElement->QueryFloatAttribute("volume", &SoundVolume);
		}

		// Get music element
		TiXmlElement *MusicElement = AudioElement->FirstChildElement("music");
		if(!MusicElement) {

			// Get music attributes
			MusicElement->QueryFloatAttribute("volume", &MusicVolume);
		}
		*/
	}

	// Get input element
	TiXmlElement *InputElement = ConfigElement->FirstChildElement("input");
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
	for(TiXmlElement *ActionElement = InputElement->FirstChildElement("action"); ActionElement != 0; ActionElement = ActionElement->NextSiblingElement("action")) {
	
		// Get action type
		int ActionType;
		if(ActionElement->QueryIntAttribute("type", &ActionType) != TIXML_SUCCESS)
			continue;

		// Get key
		int KeyCode;
		if(ActionElement->QueryIntAttribute("key", &KeyCode) != TIXML_SUCCESS)
			continue;

		// Assign key
		if(ActionType >= 0 && ActionType < _Actions::COUNT && KeyCode >= 0 && KeyCode < KEY_KEY_CODES_COUNT)
			Keys[ActionType] = (EKEY_CODE)KeyCode;
		
	}

	// Replays
	TiXmlElement *ReplayElement = ConfigElement->FirstChildElement("replay");
	if(ReplayElement) {
		ReplayElement->QueryFloatAttribute("interval", (float *)(&ReplayInterval));
	}

	return 1;
}

// Writes the config file
int _Config::WriteConfig() {

	TiXmlDocument Document;

	// Create header
	Document.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));
	
	// Create config element
	TiXmlElement *ConfigElement = new TiXmlElement("config");
	ConfigElement->SetAttribute("version", "1.0");
	Document.LinkEndChild(ConfigElement);

	// Create video element
	TiXmlElement *VideoElement = new TiXmlElement("video");
	//VideoElement->SetAttribute("driver", DriverType);
	ConfigElement->LinkEndChild(VideoElement);

	// Screen settings
	TiXmlElement *ScreenElement = new TiXmlElement("screen");
	ScreenElement->SetAttribute("width", ScreenWidth);
	ScreenElement->SetAttribute("height", ScreenHeight);
	ScreenElement->SetAttribute("fullscreen", Fullscreen);
	VideoElement->LinkEndChild(ScreenElement);

	// Filtering
	TiXmlElement *FilteringElement = new TiXmlElement("filtering");
	FilteringElement->SetAttribute("trilinear", TrilinearFiltering);
	FilteringElement->SetAttribute("anisotropic", AnisotropicFiltering);
	FilteringElement->SetAttribute("antialiasing", AntiAliasing);
	VideoElement->LinkEndChild(FilteringElement);

	// Shadows
	TiXmlElement *ShadowsElement = new TiXmlElement("shadows");
	ShadowsElement->SetAttribute("enabled", Shadows);
	VideoElement->LinkEndChild(ShadowsElement);

	// Shaders
	TiXmlElement *ShadersElement = new TiXmlElement("shaders");
	ShadersElement->SetAttribute("enabled", Shaders);
	VideoElement->LinkEndChild(ShadersElement);

	// Create audio element
	TiXmlElement *AudioElement = new TiXmlElement("audio");
	AudioElement->SetAttribute("enabled", AudioEnabled);
	ConfigElement->LinkEndChild(AudioElement);

	// Sound
	//TiXmlElement *SoundElement = new TiXmlElement("sound");
	//SoundElement->SetDoubleAttribute("volume", SoundVolume);
	//AudioElement->LinkEndChild(SoundElement);

	// Music
	//TiXmlElement *MusicElement = new TiXmlElement("music");
	//MusicElement->SetDoubleAttribute("volume", MusicVolume);
	//AudioElement->LinkEndChild(MusicElement);

	// Input
	TiXmlElement *InputElement = new TiXmlElement("input");
	InputElement->SetDoubleAttribute("mousex", 1.0);
	InputElement->SetDoubleAttribute("mousey", 1.0);
	InputElement->SetAttribute("invert", InvertMouse);
	ConfigElement->LinkEndChild(InputElement);

	// Actions
	for(int i = 0; i < _Actions::COUNT; i++) {
		TiXmlElement *ActionElement = new TiXmlElement("action");
		ActionElement->SetAttribute("type", i);
		ActionElement->SetAttribute("key", Keys[i]);
		InputElement->LinkEndChild(ActionElement);
	}

	// Replays
	TiXmlElement *ReplayElement = new TiXmlElement("replay");
	ReplayElement->SetDoubleAttribute("interval", ReplayInterval);
	ConfigElement->LinkEndChild(ReplayElement);

	// Write file
	Document.SaveFile(Save.GetConfigFile().c_str());

	return 1;
}
