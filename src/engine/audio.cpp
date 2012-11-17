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
#include "audio.h"
#include "log.h"
#include "config.h"
#include "../stb_vorbis/stb_vorbis.h"

// Initializes the audio system
int AudioClass::Init(bool Enabled) {

	// Set audio enabled
	this->Enabled = Enabled;
	if(!Enabled)
		return 1;

	Log.Write("AudioClass::Init - Initializing audio");
	
	// Create device
	#ifdef _WIN32
		ALCdevice *Device = alcOpenDevice(NULL);
	#else
		ALCdevice *Device = alcOpenDevice(NULL);
	#endif
	if(Device == NULL) {
		Log.Write("AudioClass::Init - Unable to create audio device");
		Enabled = false;
		return 0;
	}

	// Create context
	ALCcontext *Context = alcCreateContext(Device, NULL);

	// Set active context
	alcMakeContextCurrent(Context);

	// Clear code
	alGetError();

	return 1;
}

// Closes the audio system
int AudioClass::Close() {
	if(!Enabled)
		return 1;

	// Free loaded sounds
	FreeAllBuffers();
	
	// Get active context
	ALCcontext *Context = alcGetCurrentContext();

	// Get device for active context
	ALCdevice *Device = alcGetContextsDevice(Context);

	// Disable context
	alcMakeContextCurrent(NULL);

	// Free context
	alcDestroyContext(Context);

	// Close device
	alcCloseDevice(Device);

	Enabled = false;

	return 1;
}

// Loads an ogg file into memory
bool AudioClass::LoadBuffer(const std::string &File) {
	if(!Enabled)
		return true;

	// Get path
	std::string Path = std::string("sounds/") + File;

	// Find existing buffer in map
	if(Buffers.find(Path) != Buffers.end())
		return true;

	// Load file
	stb_vorbis *Stream = stb_vorbis_open_filename((char*)Path.c_str(), NULL, NULL);
	if(!Stream) {
		Log.Write("AudioClass::LoadBuffer - Unable to load %s", Path.c_str());
		return false;
	}

	// Get audio information
	stb_vorbis_info Info = stb_vorbis_get_info(Stream);

	// Create new buffer
	AudioBufferStruct Buffer;
	switch(Info.channels) {
		case 1:
			Buffer.Format = AL_FORMAT_MONO16;
		break;
		case 2:
			Buffer.Format = AL_FORMAT_STEREO16;
		break;
		default:
			Log.Write("AudioClass::LoadBuffer - Unsupported # of channels %d for %s", Path.c_str());
			return false;
		break;
	}

	// Get sample count
	unsigned int SampleCount = stb_vorbis_stream_length_in_samples(Stream) * Info.channels;

	// Alloc some memory for the samples
	ALshort *Data = new ALshort[SampleCount];

	// Get samples from ogg file
	int Read = stb_vorbis_get_samples_short_interleaved(Stream, Info.channels, Data, (int)SampleCount);

	// Create buffer
	alGenBuffers(1, &Buffer.ID);
	alBufferData(Buffer.ID, Buffer.Format, Data, SampleCount * sizeof(ALshort), Info.sample_rate);

	// Free memory
	stb_vorbis_close(Stream);
	delete Data;

	// Add to map
	Buffers[Path] = Buffer;

	return true;
}

// Get a loaded buffer
const AudioBufferStruct *AudioClass::GetBuffer(const std::string &File) {
	if(!Enabled)
		return NULL;

	// Get path
	std::string Path = std::string("sounds/") + File;

	// Find buffer in map
	BuffersIterator = Buffers.find(Path);
	if(BuffersIterator == Buffers.end())
		return NULL;

	return &BuffersIterator->second;
}

// Free all loaded buffers
void AudioClass::FreeAllBuffers() {
	if(!Enabled)
		return;

	// Iterate over map
	for(BuffersIterator = Buffers.begin(); BuffersIterator != Buffers.end(); ++BuffersIterator) {
		AudioBufferStruct &Buffer = BuffersIterator->second;

		alDeleteBuffers(1, &Buffer.ID);
	}

	Buffers.clear();
}

// Set position of listener
void AudioClass::SetPosition(float X, float Y, float Z) {
	if(!Enabled)
		return;

	alListener3f(AL_POSITION, X, Y, -Z);
}

// Sets the listener direction
void AudioClass::SetDirection(float X, float Y, float Z) {
	if(!Enabled)
		return;

	float Orientation[6] = { X, Y, -Z, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, Orientation);
}

// Set gain
void AudioClass::SetGain(float Value) {
	if(!Enabled)
		return;
	
	alListenerf(AL_GAIN, Value);
}

// Create an audio source
AudioSourceClass::AudioSourceClass(const AudioBufferStruct *Buffer, bool Loop, float MinGain, float MaxGain, float ReferenceDistance, float RollOff) {
	Loaded = false;
	if(Buffer) {

		// Create source
		alGenSources(1, &ID);

		// Assign buffer to source
		alSourcei(ID, AL_BUFFER, Buffer->ID);
		alSourcef(ID, AL_MIN_GAIN, MinGain);
		alSourcef(ID, AL_MAX_GAIN, MaxGain);
		alSourcef(ID, AL_REFERENCE_DISTANCE, ReferenceDistance);
		alSourcef(ID, AL_ROLLOFF_FACTOR, RollOff);
		alSourcei(ID, AL_LOOPING, Loop);
		Loaded = true;
	}
}

// Free audio source
AudioSourceClass::~AudioSourceClass() {
	if(Loaded) {

		// Create source
		alDeleteSources(1, &ID);
		Loaded = false;
	}
}

// Play
void AudioSourceClass::Play() {
	if(Loaded) {
		
		// Get state
		ALint State;
		alGetSourcei(ID, AL_SOURCE_STATE, &State);
		
		// If already playing, stop
		if(State == AL_PLAYING)
			alSourceStop(ID);
		
		// Play sound
		alSourcePlay(ID);
	}
}

// Set pitch
void AudioSourceClass::SetPitch(float Value) {
	if(Loaded) {
		alSourcef(ID, AL_PITCH, Value);
	}
}

// Set gain
void AudioSourceClass::SetGain(float Value) {
	if(Loaded) {
		alSourcef(ID, AL_GAIN, Value);
	}
}

// Set position
void AudioSourceClass::SetPosition(float X, float Y, float Z) {
	if(Loaded) {
		alSource3f(ID, AL_POSITION, X, Y, -Z);
	}
}
