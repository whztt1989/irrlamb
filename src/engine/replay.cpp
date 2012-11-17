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
#include "replay.h"
#include "save.h"
#include "log.h"
#include "config.h"
#include "constants.h"
#include "level.h"
#include "namespace.h"
#include <ctime>
#include <sstream>

// Start recording a replay
void ReplayClass::StartRecording() {
	if(State != STATE_NONE)
		return;

	// Set up state
	State = STATE_RECORDING;
	NextPacketTime = 1.0f;
	Time = 0;

	// Get header information
	ReplayVersion = REPLAY_VERSION;
	RecordInterval = Config::Instance().ReplayInterval;
	LevelVersion = Level::Instance().GetLevelVersion();
	LevelName = Level::Instance().GetLevelName();

	// Create replay file for object data
	ReplayDataFile = Save::Instance().GetReplayPath() + "replay.dat";
	if(!ReplayStream.OpenForWrite(ReplayDataFile.c_str())) {
		Log.Write("ReplayClass::StartRecording - Unable to open %s!", ReplayDataFile.c_str());
	}
}

// Stops the recording process
void ReplayClass::StopRecording() {
	
	if(State == STATE_RECORDING) {
		State = STATE_NONE;
		ReplayStream.Close();
		remove(ReplayDataFile.c_str());
	}
}

// Saves the current replay out to a file
bool ReplayClass::SaveReplay(const std::string &PlayerDescription) {
	Description = PlayerDescription;
	TimeStamp = time(NULL);
	FinishTime = Time;

	// Flush current replay file
	ReplayStream.Flush();
	
	// Get new file name
	std::stringstream ReplayFilePath;
	ReplayFilePath << Save::Instance().GetReplayPath() << (u32)TimeStamp << ".replay";

	// Open new file
	FileClass ReplayFile;
	if(!ReplayFile.OpenForWrite(ReplayFilePath.str().c_str())) {
		Log.Write("ReplayClass::SaveReplay - Unable to open %s for writing!", ReplayFilePath.str().c_str());
		return false;
	}

	// Write replay version
	ReplayFile.WriteChar(PACKET_REPLAYVERSION);
	ReplayFile.WriteInt(ReplayVersion);

	// Write level version
	ReplayFile.WriteChar(PACKET_LEVELVERSION);
	ReplayFile.WriteInt(LevelVersion);

	// Write record interval
	ReplayFile.WriteChar(PACKET_INTERVAL);
	ReplayFile.WriteFloat(RecordInterval);

	// Write level file
	ReplayFile.WriteChar(PACKET_LEVELFILE);
	ReplayFile.WriteString(LevelName.c_str(), REPLAY_STRINGSIZE);

	// Write player's description of replay
	ReplayFile.WriteChar(PACKET_DESCRIPTION);
	ReplayFile.WriteString(Description.c_str(), REPLAY_STRINGSIZE);

	// Write time stamp
	ReplayFile.WriteChar(PACKET_DATE);
	ReplayFile.WriteInt((int)TimeStamp);

	// Write finish time
	ReplayFile.WriteChar(PACKET_FINISHTIME);
	ReplayFile.WriteFloat(FinishTime);

	// Finished with header
	ReplayFile.WriteChar(PACKET_OBJECTDATA);

	// Copy current data to new replay file
	std::ifstream CurrentReplayFile(ReplayDataFile.c_str(), std::ios::in | std::ios::binary);
	char Buffer[4096];
	std::streamsize BytesRead;
	while(!CurrentReplayFile.eof()) {
		CurrentReplayFile.read(Buffer, 4096);
		BytesRead = CurrentReplayFile.gcount();

		if(BytesRead) {
			//printf("BytesRead=%d\n", BytesRead);
			ReplayFile.WriteData(Buffer, (u32)BytesRead);
		}
	}
	CurrentReplayFile.close();
	ReplayFile.Close();

	return true;
}

// Load header data
void ReplayClass::LoadHeader() {

	// Write replay version
	int PacketType;
	bool Done = false;
	char Buffer[256];
	while(!ReplayStream.Eof() && !Done) {
		PacketType = ReplayStream.ReadChar();
		switch(PacketType) {
			case PACKET_REPLAYVERSION:
				ReplayVersion = ReplayStream.ReadInt();
			break;
			case PACKET_LEVELVERSION:
				LevelVersion = ReplayStream.ReadInt();
			break;
			case PACKET_INTERVAL:
				RecordInterval = ReplayStream.ReadFloat();
			break;
			case PACKET_LEVELFILE:
				ReplayStream.ReadString(Buffer, REPLAY_STRINGSIZE);
				LevelName = Buffer;
			break;
			case PACKET_DESCRIPTION:
				ReplayStream.ReadString(Buffer, REPLAY_STRINGSIZE);
				Description = Buffer;
			break;
			case PACKET_DATE:
				TimeStamp = ReplayStream.ReadInt();
			break;
			case PACKET_FINISHTIME:
				FinishTime = ReplayStream.ReadFloat();
			break;
			case PACKET_OBJECTDATA:
				Done = true;
			break;
		}
	}
}

// Updates the replay timer
void ReplayClass::Update(float FrameTime) {
	Time += FrameTime;
	NextPacketTime += FrameTime;
}

// Determines if a packet is required
bool ReplayClass::NeedsPacket() {
	
	return State == STATE_RECORDING && NextPacketTime >= RecordInterval;
}

// Resets the next packet timer
void ReplayClass::ResetNextPacketTimer() {

	if(NeedsPacket())
		NextPacketTime = 0;
}

// Starts replay
bool ReplayClass::LoadReplay(const std::string &ReplayFile, bool HeaderOnly) {
	LevelName = "";
	LevelVersion = 0;
	ReplayVersion = 0;

	// Get file name
	std::string FilePath = Save::Instance().GetReplayPath() + ReplayFile;

	// Open the replay
	if(!ReplayStream.OpenForRead(FilePath.c_str()))
		return false;

	// Read header
	LoadHeader();

	// Read only the header
	if(HeaderOnly)
		ReplayStream.Close();

	return true;
}

// Stops replay
void ReplayClass::StopReplay() {
	
	State = STATE_NONE;
	ReplayStream.Close();
}

// Returns true if the replay is done playing
bool ReplayClass::ReplayStopped() {

	return ReplayStream.Eof();
}

// Write replay event
void ReplayClass::WriteEvent(int Type) {
	ReplayStream.WriteChar(Type);
	ReplayStream.WriteFloat(Time);

	//printf("WriteEvent Type=%d Time=%f\n", Type, Time);
}

// Reads a packet header
void ReplayClass::ReadEvent(ReplayEventStruct &Packet) {
	Packet.Type = ReplayStream.ReadChar();
	Packet.TimeStamp = ReplayStream.ReadFloat();
}
