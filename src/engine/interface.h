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
#ifndef INTERFACE_H
#define INTERFACE_H

// Libraries
#include "singleton.h"
#include <string>
#include <rect.h>
#include <ITexture.h>
#include <IGUIFont.h>

// Forward Declarations
class AudioSourceClass;

// Structures
struct TutorialTextStruct {
	std::string Text;
	float DeleteTime;
};

// Classes
class InterfaceClass {

	public:

		enum SkinType {
			SKIN_MENU,
			SKIN_GAME,
		};

		enum FontType {
			FONT_SMALL,
			FONT_MEDIUM,
			FONT_LARGE,
			FONT_COUNT,
		};

		enum AlignType {
			ALIGN_LEFT,
			ALIGN_CENTER,
			ALIGN_RIGHT,
		};

		enum ImageType {
			IMAGE_MOUSECURSOR,
			IMAGE_FADE,
			IMAGE_BUTTON80,
			IMAGE_BUTTON100,
			IMAGE_BUTTON128,
			IMAGE_TEXTBOXSHEET0,
			IMAGE_TEXTBOXSHEET1,
			IMAGE_TEXTBOXSHEET2,
			IMAGE_PAUSE,
			IMAGE_REWIND,
			IMAGE_FASTFORWARD,
			IMAGE_FASTREVERSE,
			IMAGE_INCREASE,
			IMAGE_DECREASE,
			IMAGE_COUNT,
		};
		
		enum SoundType {
			SOUND_CONFIRM,
			SOUND_COUNT,
		};

		int Init();
		int Close();
		void Update(float FrameTime);
		void Draw();
		void Clear();
		void ChangeSkin(SkinType Type);
	
		void SetTutorialText(const std::string &Text, float Length);
		void ConvertSecondsToString(float Time, char *String);

		irr::core::recti GetCenteredRect(int PositionX, int PositionY, int Width, int Height);
		irr::core::recti GetRect(int PositionX, int PositionY, int Width, int Height);
		
		irr::video::ITexture *GetImage(ImageType Image) { return Images[Image]; }
		irr::gui::IGUIFont *GetFont(FontType Font) { return Fonts[Font]; }

		void FadeScreen(float Amount);
		void RenderText(const char *Text, int PositionX, int PositionY, AlignType AlignType, FontType FontType=FONT_SMALL, const irr::video::SColor &Color=irr::video::SColor(255, 255, 255, 255));
		void DrawImage(ImageType Type, int PositionX, int PositionY, int Width, int Height, const irr::video::SColor &Color=irr::video::SColor(255, 255, 255, 255));
		void DrawTextBox(int PositionX, int PositionY, int Width, int Height, const irr::video::SColor &Color=irr::video::SColor(255, 255, 255, 255));

		void LoadSounds();
		void UnloadSounds();
		void PlaySound(SoundType Sound);

	private:

		TutorialTextStruct TutorialText;
		float Timer;

		irr::gui::IGUIFont *Fonts[FONT_COUNT];
		irr::video::ITexture *Images[IMAGE_COUNT];
		AudioSourceClass *Sounds[SOUND_COUNT];

};

// Singletons
typedef SingletonClass<InterfaceClass> Interface;

#endif
