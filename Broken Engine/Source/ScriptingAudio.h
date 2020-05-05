#ifndef __SCRIPTINGAUDIO_H__
#define __SCRIPTINGAUDIO_H__

#include "BrokenCore.h"

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingAudio {
public:
	ScriptingAudio() {}
	~ScriptingAudio() {}

public:
	void SetVolume(float volume, uint UID);

	void PlayAudioEventGO(std::string event, uint UID);
	void StopAudioEventGO(std::string event, uint UID);
	void PauseAudioEventGO(std::string event, uint UID);
	void ResumeAudioEventGO(std::string event,uint UID);

	void PlayAudioEvent(std::string event);
	void StopAudioEvent(std::string event);
	void PauseAudioEvent(std::string event);
	void ResumeAudioEvent(std::string event);
	void SetAudioTrigger(std::string trigger, uint GOUID);
	void SetAudioSwitch(std::string SwitchGroup, std::string Switchstate, uint GOUID);
	void SetAudioState(std::string StateGroup, std::string State);
	void SetAudioRTPCValue(std::string RTPCName, int value, uint wwiseGOID);

};
BE_END_NAMESPACE
#endif // __SCRIPTINGAUDIO_H__
