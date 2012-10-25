/*************************************************************************************
*	irrlamb - http://irrlamb.googlecode.com
*	Copyright (C) 2011  Alan Witkowski
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
#include "campaign.h"
#include "globals.h"
#include "log.h"
#include "game.h"
#include "../tinyxml/tinyxml.h"

// Loads the campaign data
int CampaignClass::Init() {
	Campaigns.clear();

	Log.Write("CampaignClass::Init - Loading file irrlamb.xml");

	// Open the XML file
	std::string LevelFile = std::string("levels/main.xml");
	TiXmlDocument Document(LevelFile.c_str());
	if(!Document.LoadFile()) {
		Log.Write("line %d column %d: %s", Document.ErrorRow(), Document.ErrorCol(), Document.ErrorDesc());
		Close();
		return 0;
	}

	// Check for level tag
	TiXmlElement *CampaignsElement = Document.FirstChildElement("campaigns");
	if(!CampaignsElement) {
		Log.Write("Could not find campaigns tag");
		return 0;
	}

	// Load campaigns
	TiXmlElement *CampaignElement = CampaignsElement->FirstChildElement("campaign");
	for(; CampaignElement != 0; CampaignElement = CampaignElement->NextSiblingElement("campaign")) {

		CampaignStruct Campaign;
		Campaign.Name = CampaignElement->Attribute("name");

		// Get levels
		TiXmlElement *LevelElement = CampaignElement->FirstChildElement("level");
		for(; LevelElement != 0; LevelElement = LevelElement->NextSiblingElement("level")) {
			LevelStruct Level;
			Level.File = LevelElement->GetText();
			Level.DataPath = Game::Instance().GetWorkingPath() + "levels/" + Level.File + "/";
			Level.Unlocked = 0;
			LevelElement->Attribute("unlocked", &Level.Unlocked);

			Campaign.Levels.push_back(Level);
		}

		Campaigns.push_back(Campaign);
	}

	return 1;
}

// Closes the campaign system
int CampaignClass::Close() {

	Campaigns.clear();

	return 1;
}
