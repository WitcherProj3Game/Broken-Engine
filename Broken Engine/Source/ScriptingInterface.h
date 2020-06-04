#ifndef __SCRIPTINGINTERFACE_H__
#define __SCRIPTINGINTERFACE_H__

#include "BrokenCore.h"

class lua_State;

namespace luabridge
{
	class LuaRef;
}

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingInterface {
public:
	ScriptingInterface() {}
	~ScriptingInterface() {}

public:

	void MakeUIComponentVisible(const char* comp_type, uint go_UUID);
	void MakeUIComponentInvisible(const char* comp_type, uint go_UUID);

	void SetBarPercentage(float percentage, uint go_UUID);
	void SetCircularBarPercentage(float percentage, uint go_UUID);
	void SetUIText(const char* text, uint go_UUID);
	void SetUITextAndNumber(const char* text, float number, uint go_UUID);
	void SetUITextNumber(float number, uint go_UUID);
	void SetUIElementInteractable(const char* comp_type, uint go_UUID, bool value);

	void ChangeUIComponentColor(const char* comp_type, float r, float g, float b, float a, uint go_UUID);
	void ChangeUIBarColor(const char* comp_type, bool topBarPlane, float r, float g, float b, float a, uint go_UUID);
	void ChangeUIComponentAlpha(const char* comp_type, float alpha, uint go_UUID);
	void ChangeUIBarAlpha(const char* comp_type, bool topBarPlane, float alpha, uint go_UUID);

	luabridge::LuaRef GetUIComponentColor(const char* comp_type, uint gameobject_UUID, lua_State* L) const;
	luabridge::LuaRef GetUIBarColor(const char* comp_type, bool topBarPlane, uint gameobject_UUID, lua_State* L) const;
	float GetUIComponentAlpha(const char* comp_type, uint gameobject_UUID) const;
	float GetUIBarAlpha(const char* comp_type, bool topBarPlane, uint gameobject_UUID) const;

	void SetUIElementPosition(const char* comp_type, float x, float y, uint go_UUID);


	void PlayUIAnimation(uint gameobject_UUID);
	bool UIAnimationFinished(uint gameobject_UUID) const;

};
BE_END_NAMESPACE
#endif // __SCRIPTINGINTERFACE_H__
