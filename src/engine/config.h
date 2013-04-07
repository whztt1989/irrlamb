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
#ifndef CONFIG_H
#define CONFIG_H

// Libraries
#include "actions.h"

// Classes
class ConfigClass {

	public:

		int Init();
		int Close();

		void Reset();
		int ReadConfig();
		int WriteConfig();

		// Video
		int DriverType;
		int ScreenWidth, ScreenHeight;
		bool Fullscreen;
		bool Shadows;
		bool TrilinearFiltering;
		bool Shaders;
		int AnisotropicFiltering;
		int AntiAliasing;		

		// Audio
		bool AudioEnabled;
		float SoundVolume, MusicVolume;

		// Input
		int Keys[_Actions::COUNT];
		float MouseScaleX, MouseScaleY;
		bool InvertMouse;

		// Replays
		float ReplayInterval;
	
	private:

};

// Singletons
extern ConfigClass Config;

#endif
