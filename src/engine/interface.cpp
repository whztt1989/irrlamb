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
#include <engine/interface.h>
#include <engine/globals.h>
#include <engine/config.h>
#include <engine/log.h>
#include <engine/audio.h>
#include <engine/namespace.h>

_Interface Interface;

// Initializes the graphics system
int _Interface::Init() {
	DrawHUD = true;
		
	// Get skin
	IGUISkin *Skin = irrGUI->getSkin();

	// Set font
	Fonts[FONT_SMALL] = irrGUI->getFont("fonts/font_small.xml");
	if(!Fonts[FONT_SMALL]) {
		Log.Write("_Interface::Init - Unable to load font_small.xml");
		return 0;
	}
	Skin->setFont(Fonts[FONT_SMALL]);

	// Load alternate medium font
	Fonts[FONT_MEDIUM] = irrGUI->getFont("fonts/font_medium.xml");
	if(!Fonts[FONT_MEDIUM]) {
		Log.Write("_Interface::Init - Unable to load font_medium.xml");
		return 0;
	}

	// Load alternate large font
	Fonts[FONT_LARGE] = irrGUI->getFont("fonts/font_large.xml");
	if(!Fonts[FONT_LARGE]) {
		Log.Write("_Interface::Init - Unable to load font_large.xml");
		return 0;
	}

	// Load images
	Images[IMAGE_MOUSECURSOR] = irrDriver->getTexture("art/cursor.png");
	Images[IMAGE_FADE] = irrDriver->getTexture("art/fade.png");
	Images[IMAGE_BUTTON80] = irrDriver->getTexture("art/button_80.png");
	Images[IMAGE_BUTTON100] = irrDriver->getTexture("art/button_100.png");
	Images[IMAGE_BUTTON128] = irrDriver->getTexture("art/button_128.png");
	Images[IMAGE_TEXTBOXSHEET0] = irrDriver->getTexture("art/sheet_textbox0.png");
	Images[IMAGE_TEXTBOXSHEET1] = irrDriver->getTexture("art/sheet_textbox1.png");
	Images[IMAGE_TEXTBOXSHEET2] = irrDriver->getTexture("art/sheet_textbox2.png");
	Images[IMAGE_PAUSE] = irrDriver->getTexture("art/button_pause.png");
	Images[IMAGE_REWIND] = irrDriver->getTexture("art/button_rewind.png");
	Images[IMAGE_FASTFORWARD] = irrDriver->getTexture("art/button_ff.png");
	Images[IMAGE_FASTREVERSE] = irrDriver->getTexture("art/button_fr.png");
	Images[IMAGE_INCREASE] = irrDriver->getTexture("art/button_inc.png");
	Images[IMAGE_DECREASE] = irrDriver->getTexture("art/button_dec.png");

	// Set sounds to NULL
	for(int i = 0; i < SOUND_COUNT; i++)
		Sounds[i] = NULL;
	
	// Set up skins
	ChangeSkin(SKIN_MENU);
	Timer = 0.0f;

	return 1;
}

// Closes the graphics system
int _Interface::Close() {
	
	return 1;
}

// Changes irrlicht skins
void _Interface::ChangeSkin(SkinType Type) {

	IGUISkin *Skin = irrGUI->getSkin();
	Skin->setColor(EGDC_BUTTON_TEXT, SColor(255, 255, 255, 255));
	Skin->setColor(EGDC_WINDOW, SColor(255, 0, 0, 20));	
	Skin->setColor(EGDC_WINDOW_SYMBOL, SColor(255, 255, 255, 255));
	Skin->setColor(EGDC_GRAY_WINDOW_SYMBOL, SColor(255, 128, 128, 128));
	Skin->setColor(EGDC_GRAY_EDITABLE, SColor(255, 0, 0, 0));
	Skin->setColor(EGDC_FOCUSED_EDITABLE, SColor(255, 0, 0, 0));
	Skin->setColor(EGDC_EDITABLE, SColor(255, 0, 0, 0));
	
	switch(Type) {
		case SKIN_MENU:
			Skin->setColor(EGDC_3D_FACE, SColor(255, 32, 32, 32));
			Skin->setColor(EGDC_3D_SHADOW, SColor(255, 0, 0, 16));

			Skin->setColor(EGDC_3D_HIGH_LIGHT, SColor(255, 16, 16, 16));
			Skin->setColor(EGDC_3D_DARK_SHADOW, SColor(255, 0, 0, 16));
		break;
		case SKIN_GAME:
			Skin->setColor(EGDC_3D_FACE, SColor(0, 0, 0, 0));
			Skin->setColor(EGDC_3D_SHADOW, SColor(0, 0, 0, 0));

			Skin->setColor(EGDC_3D_HIGH_LIGHT, SColor(0, 0, 0, 0));
			Skin->setColor(EGDC_3D_DARK_SHADOW, SColor(0, 0, 0, 0));
		break;
	}
}

// Clear all the GUI elements
void _Interface::Clear() {
	irrGUI->clear();
	TutorialText.Text = "";
	Timer = 0.0f;
}

// Adds tutorial text to the screen
void _Interface::SetTutorialText(const std::string &Text, float Length) {
	TutorialText.Text = Text;
	TutorialText.DeleteTime = Length + Timer;
}

// Maintains all of the timed elements
void _Interface::Update(float FrameTime) {

	Timer += FrameTime;
	if(TutorialText.Text.size() > 0) {
		if(Timer >= TutorialText.DeleteTime) {
			TutorialText.Text = "";
		}
	}
}

// Draw interface elements
void _Interface::Draw(float Time) {
	if(!DrawHUD)
		return;
	
	// Draw timer
	char TimeString[32];
	ConvertSecondsToString(Time, TimeString);
	RenderText(TimeString, 10, 10, _Interface::ALIGN_LEFT, _Interface::FONT_LARGE);

	// Draw tutorial text
	if(TutorialText.Text.size() > 0) {

		// Get tutorial text alpha
		SColor TextColor(255, 255, 255, 255), BoxColor(160, 255, 255, 255);
		float TimeLeft = TutorialText.DeleteTime - Timer;
		if(TimeLeft < 2.0f) {
			TextColor.setAlpha((u32)(255 * TimeLeft / 2.0));
			BoxColor.setAlpha((u32)(160 * TimeLeft / 2.0));
		}

		// Draw tutorial text
		int Width = 370, Height = 115;
		DrawTextBox(irrDriver->getScreenSize().Width / 2, irrDriver->getScreenSize().Height - Height / 2 - 30, Width, Height, BoxColor);
		Interface.RenderText(TutorialText.Text.c_str(), irrDriver->getScreenSize().Width / 2, irrDriver->getScreenSize().Height - Height - 13, _Interface::ALIGN_CENTER, _Interface::FONT_MEDIUM, TextColor);
	}
}

// Converts milliseconds to a time string
void _Interface::ConvertSecondsToString(float Time, char *String) {
	u32 Minutes = (u32)(Time) / 60;
	u32 Seconds = (u32)(Time - Minutes * 60);
	u32 Centiseconds = (u32)((Time - (u32)(Time)) * 100);
	sprintf(String, "%.2d:%.2d:%.2d", Minutes, Seconds, Centiseconds);
}

// Gets a rectangle centered around a point
recti _Interface::GetCenteredRect(int PositionX, int PositionY, int Width, int Height) {

	return recti(PositionX - (Width >> 1), PositionY - (Height >> 1), PositionX + (Width >> 1), PositionY + (Height >> 1));
}

// Gets a rectangle
recti _Interface::GetRect(int PositionX, int PositionY, int Width, int Height) {

	return recti(PositionX, PositionY, PositionX + Width, PositionY + Height);
}

// Fades the screen
void _Interface::FadeScreen(float Amount) {
	irrDriver->draw2DImage(Images[IMAGE_FADE], position2di(0, 0), recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height), 0, SColor((u32)(Amount * 255), 255, 255, 255), true);
}

// Draws text to the screen
void _Interface::RenderText(const char *Text, int PositionX, int PositionY, AlignType AlignType, FontType FontType, const SColor &Color) {

	// Convert string
	stringw String(Text);

	// Get dimensions
	dimension2d<u32> TextArea = Fonts[FontType]->getDimension(String.c_str());

	switch(AlignType) {
		case ALIGN_LEFT:
		break;
		case ALIGN_CENTER:
			PositionX -= TextArea.Width >> 1;
		break;
		case ALIGN_RIGHT:
			PositionX -= TextArea.Width;
		break;
	}

	// Draw text
	Fonts[FontType]->draw(String.c_str(), recti(PositionX, PositionY, PositionX + TextArea.Width, PositionY + TextArea.Height), Color);
}

// Draws an interface image centered around a position
void _Interface::DrawImage(ImageType Type, int PositionX, int PositionY, int Width, int Height, const SColor &Color) {

	irrDriver->draw2DImage(Images[Type], position2di(PositionX - (Width >> 1), PositionY - (Height >> 1)), recti(0, 0, Width, Height), 0, Color, true);
}

// Draws a text box
void _Interface::DrawTextBox(int PositionX, int PositionY, int Width, int Height, const SColor &Color) {
	PositionX -= Width >> 1;
	PositionY -= Height >> 1;

	// Draw corners
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], position2di(PositionX, PositionY), recti(0, 0, 10, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], position2di(PositionX + Width - 10, PositionY), recti(10, 0, 20, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], position2di(PositionX, PositionY + Height - 10), recti(0, 10, 10, 20), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET0], position2di(PositionX + Width - 10, PositionY + Height - 10), recti(10, 10, 20, 20), 0, Color, true);

	// Draw middle
	irrDriver->draw2DImage(Images[IMAGE_FADE], position2di(PositionX + 10, PositionY + 10), recti(0, 0, Width - 20, Height - 20), 0, Color, true);

	// Draw edges
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET1], position2di(PositionX + 10, PositionY), recti(0, 0, Width - 20, 10), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET1], position2di(PositionX + 10, PositionY + Height - 10), recti(0, 6, Width - 20, 16), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET2], position2di(PositionX, PositionY + 10), recti(0, 0, 10, Height - 20), 0, Color, true);
	irrDriver->draw2DImage(Images[IMAGE_TEXTBOXSHEET2], position2di(PositionX + Width - 10, PositionY + 10), recti(6, 0, 16, Height - 20), 0, Color, true);
}

// Load GUI sounds
void _Interface::LoadSounds() {
	Sounds[SOUND_CONFIRM] = new _AudioSource(Audio.GetBuffer("confirm.ogg"), false, 0.0f, 0.70f);
}

// Unload GUI sounds
void _Interface::UnloadSounds() {

	for(int i = 0; i < SOUND_COUNT; i++) {
		delete Sounds[i];
		Sounds[i] = NULL;
	}
}

// Play a GUI sound
void _Interface::PlaySound(SoundType Sound) {
	if(Sounds[Sound])
		Sounds[Sound]->Play();
}
