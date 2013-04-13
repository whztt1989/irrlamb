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
#include <ui.h>
#include <engine/globals.h>
#include <engine/namespace.h>
#include <engine/interface.h>

enum GUIElements {
	MAIN_SINGLEPLAYER,
	MAIN_REPLAYS,
	MAIN_OPTIONS,
	MAIN_QUIT,
	SINGLEPLAYER_BACK,
	LEVELS_GO,
	LEVELS_BUY,
	LEVELS_HIGHSCORES,
	LEVELS_BACK,
	LEVELS_SELECTEDLEVEL,
	LEVELINFO_DESCRIPTION,
	LEVELINFO_ATTEMPTS,
	LEVELINFO_WINS,
	LEVELINFO_LOSSES,
	LEVELINFO_PLAYTIME,
	LEVELINFO_BESTTIME,
	REPLAYS_FILES,
	REPLAYS_GO,
	REPLAYS_DELETE,
	REPLAYS_BACK,
	OPTIONS_VIDEO,
	OPTIONS_AUDIO,
	OPTIONS_CONTROLS,
	OPTIONS_BACK,
	VIDEO_SAVE,
	VIDEO_CANCEL,
	VIDEO_VIDEOMODES,
	VIDEO_FULLSCREEN,
	VIDEO_SHADOWS,
	VIDEO_SHADERS,
	AUDIO_ENABLED,
	AUDIO_SAVE,
	AUDIO_CANCEL,
	CONTROLS_SAVE,
	CONTROLS_CANCEL,
	CONTROLS_INVERTMOUSE,
	CONTROLS_MOVEFORWARD,
	CONTROLS_MOVEBACK,
	CONTROLS_MOVELEFT,
	CONTROLS_MOVERIGHT,
	CONTROLS_MOVERESET,
	CONTROLS_MOVEJUMP,
};

_UI UI;

void _UI::InitTitleScreen() {
	int CenterX = irrDriver->getScreenSize().Width / 2, CenterY = irrDriver->getScreenSize().Height / 2, X, Y;
	// Logo
	irrGUI->addImage(irrDriver->getTexture("art/logo.jpg"), position2di(CenterX - 256, CenterY - 215));
	//IGUIStaticText *TextVersion = irrGUI->addStaticText(stringw(GAME_VERSION).c_str(), Interface.GetCenteredRect(40, irrDriver->getScreenSize().Height - 20, 50, 15), false, false);

	// Button
	Y = CenterY - 50;
	IGUIButton *ButtonSinglePlayer = irrGUI->addButton(Interface.GetCenteredRect(CenterX, Y, 130, 34), 0, MAIN_SINGLEPLAYER, L"Single Player");
	IGUIButton *ButtonReplays = irrGUI->addButton(Interface.GetCenteredRect(CenterX, Y + 50, 130, 34), 0, MAIN_REPLAYS, L"Replays");
	IGUIButton *ButtonOptions = irrGUI->addButton(Interface.GetCenteredRect(CenterX, Y + 100, 130, 34), 0, MAIN_OPTIONS, L"Options");
	IGUIButton *ButtonQuit = irrGUI->addButton(Interface.GetCenteredRect(CenterX, Y + 150, 130, 34), 0, MAIN_QUIT, L"Quit");
	ButtonSinglePlayer->setImage(Interface.GetImage(_Interface::IMAGE_BUTTON128));
	ButtonSinglePlayer->setUseAlphaChannel(true);
	ButtonSinglePlayer->setDrawBorder(false);
	//ButtonSinglePlayer->set
	//ButtonSinglePlayer->setPressedImage(Interface.GetImage(_Interface::IMAGE_BUTTON128));
	ButtonReplays->setImage(Interface.GetImage(_Interface::IMAGE_BUTTON128));
	ButtonReplays->setUseAlphaChannel(true);
	ButtonReplays->setDrawBorder(false);
	ButtonOptions->setImage(Interface.GetImage(_Interface::IMAGE_BUTTON128));
	ButtonOptions->setUseAlphaChannel(true);
	ButtonOptions->setDrawBorder(false);
	ButtonQuit->setImage(Interface.GetImage(_Interface::IMAGE_BUTTON128));
	ButtonQuit->setUseAlphaChannel(true);
	ButtonQuit->setDrawBorder(false);

	//Image->
}