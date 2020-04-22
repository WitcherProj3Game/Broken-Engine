#ifndef __SCRIPTINGAUDIO_H__
#define __SCRIPTINGAUDIO_H__

#include "BrokenCore.h"

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingAudio {
public:
	ScriptingAudio();
	~ScriptingAudio();

public:
	void SetVolume(float volume, uint UID);
	void PlayAudioEvent(std::string event, uint UID);
	void StopAudioEvent(std::string event, uint UID);
	void PauseAudioEvent(std::string event, uint UID);
	void ResumeAudioEvent(std::string event,uint UID);

};
BE_END_NAMESPACE
#endif // __SCRIPTINGAUDIO_H__