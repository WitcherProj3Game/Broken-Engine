#ifndef __COMPONENTBUTTON_H__
#define __COMPONENTBUTTON_H__
#pragma once

#include "UI_Element.h"
#include "Color.h"
#include "Math.h"
#include "SDL/include/SDL_rect.h"

#include <string>
#include <vector>

BE_BEGIN_NAMESPACE

class ResourceTexture;
class ComponentCanvas;
class ComponentScript;
class ComponentImage;

class BROKEN_API ComponentButton : public UI_Element
{
public:
	enum State
	{
		NOTHING = 0,
		IDLE,
		HOVERED,
		SELECTED,
		LOCKED,
		DRAGGING
	};

	ComponentButton(GameObject* gameObject);
	virtual ~ComponentButton();

	void Update() override;

	void Draw();

	State GetState() { return state; }
	void UpdateState();
	void OnClick();
	void ChangeStateTo(State new_state) { state = new_state; }
	void ChangeColorTo(Color new_color);
	const Color GetButtonColor() const { return color; }

	static inline Component::ComponentType GetType() { return Component::ComponentType::Button; }

	float2 GetParentPos();
	float2 GetFinalPosition();

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;
	void SetNullptr();

public:
	bool resize = true;
	State state = NOTHING;

public:
	ComponentScript* script = nullptr;
	GameObject* script_obj = nullptr;
	ComponentImage* image = nullptr;

private:
	SDL_Rect collider;
	bool collider_visible = true;

	std::string func_name;
	std::vector<const char*> func_list;
	uint func_pos = 0;

	Color color;
	Color idle_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color hovered_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	Color selected_color = { 0.25f, 0.25f, 1.0f, 1.0f };
	Color locked_color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

BE_END_NAMESPACE
#endif
