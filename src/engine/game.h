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
#ifndef GAME_H
#define GAME_H

// Libraries
#include <irrTypes.h>
#include <string>

// Forward Declarations
class _State;

class _Game {

	public:

		enum ManagerStateType {
			STATE_INIT,
			STATE_UPDATE,
			STATE_CLOSE
		};

		int Init(int Count, char **Arguments);
		void Update();
		void Close();
		
		void ChangeState(_State *State);
		_State *GetState() { return State; }

		bool IsDone() { return Done; }
		void SetDone(bool Value) { Done = Value; }
		float GetTimeStep() { return TimeStep; }
		void ResetTimer();
		ManagerStateType GetManagerState() { return ManagerState; }

		void EnableAudio();
		void DisableAudio();

		const std::string &GetWorkingPath() { return WorkingPath; }

	private:

		void ResetGraphics();

		// States
		ManagerStateType ManagerState;
		_State *State, *NewState;
		bool PreviousWindowActive, WindowActive;

		// Flags
		bool Done, MouseWasLocked;

		// Time
		irr::u32 TimeStamp;
		float SleepRate;

		// Physics
		float TimeStep, TimeStepAccumulator;

		// Misc
		std::string WorkingPath;
};

// Singletons
extern _Game Game;

#endif
