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
#include <font/CGUITTFont.h>
#include <menu.h>

_Interface Interface;

const int MESSAGE_WIDTH = 470;
const int MESSAGE_HEIGHT = 115;
const int MESSAGE_PADDING = 15;

// Constructor for empty element
CGUIEmptyElement::CGUIEmptyElement(IGUIEnvironment *Environment, irr::gui::IGUIElement *Parent)
	:	irr::gui::IGUIElement((EGUI_ELEMENT_TYPE)MGUIET_EMPTY, Environment, Parent, -1, recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height)) { }

// Initializes the graphics system
int _Interface::Init() {
	DrawHUD = true;
		
	// Get skin
	IGUISkin *Skin = irrGUI->getSkin();

	// Set font
	Fonts[FONT_SMALL] = CGUITTFont::createTTFont(irrGUI, "fonts/Arimo-Regular.ttf", 16);
	if(!Fonts[FONT_SMALL]) {
		Log.Write("_Interface::Init - Unable to load FONT_SMALL");
		return 0;
	}

	// Load alternate medium font
	Fonts[FONT_MEDIUM] = CGUITTFont::createTTFont(irrGUI, "fonts/Arimo-Regular.ttf", 24);
	if(!Fonts[FONT_MEDIUM]) {
		Log.Write("_Interface::Init - Unable to load FONT_MEDIUM");
		return 0;
	}

	// Load alternate large font
	Fonts[FONT_LARGE] = CGUITTFont::createTTFont(irrGUI, "fonts/RobotoCondensed-Regular.ttf", 48);
	if(!Fonts[FONT_LARGE]) {
		Log.Write("_Interface::Init - Unable to load FONT_LARGE");
		return 0;
	}

	// Load alternate large font
	Fonts[FONT_BUTTON] = CGUITTFont::createTTFont(irrGUI, "fonts/FjallaOne-Regular.ttf", 24);
	if(!Fonts[FONT_BUTTON]) {
		Log.Write("_Interface::Init - Unable to load FONT_BUTTON");
		return 0;
	}
	Skin->setFont(Fonts[FONT_MEDIUM]);
	
	// Load images
	Images[IMAGE_MOUSECURSOR] = irrDriver->getTexture("art/cursor.png");
	Images[IMAGE_FADE] = irrDriver->getTexture("art/fade.png");
	Images[IMAGE_BUTTON_SMALL] = irrDriver->getTexture("art/button_small.png");
	Images[IMAGE_BUTTON_MEDIUM] = irrDriver->getTexture("art/button_medium.png");
	Images[IMAGE_BUTTON_BIG] = irrDriver->getTexture("art/button_big.png");
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

	CenterX = irrDriver->getScreenSize().Width / 2;
	CenterY = irrDriver->getScreenSize().Height / 2;

	TutorialText.MessageX = CenterX;
	TutorialText.MessageY = irrDriver->getScreenSize().Height - MESSAGE_HEIGHT / 2 - 30;

	Menu.ClearCurrentLayout();

	return 1;
}

// Closes the graphics system
int _Interface::Close() {
	
	for(int i = 0; i < FONT_COUNT; i++)
		Fonts[i]->drop();
	
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
	if(TutorialText.Text) {
		TutorialText.Text->remove();
		TutorialText.Text = NULL;
	}
	Timer = 0.0f;
}

// Adds tutorial text to the screen
void _Interface::SetTutorialText(const std::string &Text, float Length) {
	if(TutorialText.Text) {
		TutorialText.Text->remove();
		TutorialText.Text = NULL;
	}
		
	TutorialText.Text = irrGUI->addStaticText(stringw(Text.c_str()).c_str(), GetCenteredRect(TutorialText.MessageX, TutorialText.MessageY, MESSAGE_WIDTH - MESSAGE_PADDING, MESSAGE_HEIGHT), false, true);
	TutorialText.Text->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
	TutorialText.DeleteTime = Length + Timer;
}

// Maintains all of the timed elements
void _Interface::Update(float FrameTime) {

	Timer += FrameTime;
	if(TutorialText.Text) {
		if(Timer >= TutorialText.DeleteTime) {
			TutorialText.Text->remove();
			TutorialText.Text = NULL;
		}
	}
}

// Draw interface elements
void _Interface::Draw(float Time) {
	
	// Draw timer
	char TimeString[32];
	ConvertSecondsToString(Time, TimeString);
	if(DrawHUD)
		RenderText(TimeString, 10, 10, _Interface::ALIGN_LEFT, _Interface::FONT_MEDIUM);

	// Draw tutorial text
	if(TutorialText.Text) {
		TutorialText.Text->setVisible(DrawHUD);

		// Get tutorial text alpha
		SColor TextColor(255, 255, 255, 255), BoxColor(160, 255, 255, 255);
		float TimeLeft = TutorialText.DeleteTime - Timer;
		if(TimeLeft < 2.0f) {
			TextColor.setAlpha((u32)(255 * TimeLeft / 2.0));
			BoxColor.setAlpha((u32)(160 * TimeLeft / 2.0));
		}

		// Update tutorial text color
		TutorialText.Text->setOverrideColor(TextColor);

		// Draw tutorial text
		if(DrawHUD)
			DrawTextBox(TutorialText.MessageX, TutorialText.MessageY, MESSAGE_WIDTH, MESSAGE_HEIGHT, BoxColor);
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

// Gets a rectangle aligned right
recti _Interface::GetRightRect(int PositionX, int PositionY, int Width, int Height) {

	return recti(PositionX - Width, PositionY - (Height >> 1), PositionX, PositionY + (Height >> 1));
}

// Gets a rectangle
recti _Interface::GetRect(int PositionX, int PositionY, int Width, int Height) {

	return recti(PositionX, PositionY, PositionX + Width, PositionY + Height);
}

// Fades the screen
void _Interface::FadeScreen(float Amount) {
	irrDriver->draw2DImage(Images[IMAGE_FADE], position2di(0, 0), recti(0, 0, irrDriver->getScreenSize().Width, irrDriver->getScreenSize().Height), 0, SColor((u32)(Amount * 255), 255, 255, 255), true);
	if(TutorialText.Text) {
		SColor TextColor = TutorialText.Text->getOverrideColor();
		TextColor.setAlpha((u32)(TextColor.getAlpha() * (1.0f - Amount)));
		TutorialText.Text->setOverrideColor(TextColor);
	}
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
