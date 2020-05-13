#ifndef __MODULEUI_H__
#define __MODULEUI_H__
#pragma once

#include "Module.h"
#include "Math.h"

struct SDL_Rect;

BE_BEGIN_NAMESPACE

class UI_Element;
class Component;
class ComponentCanvas;
class ComponentCamera;

class BROKEN_API ModuleUI : public Module
{
	struct PrioritySort {
		bool operator()(UI_Element* const& node1, UI_Element* const& node2);
	};

public:
	ModuleUI(bool start_enabled = true);
	~ModuleUI();

	bool Init(json& file) override;
	bool Start() override;
	void LoadFont(std::string& font);
	update_status PreUpdate(float dt) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp() override;

	void Draw() const;
	void AddElement(UI_Element* c) { elements.push_back(c); }
	void RemoveElement(UI_Element* c);
	
	void Clear();

	bool CheckMousePos(SDL_Rect* collider);
	bool CheckClick(bool draggable = false);

	void OrderElements();

public:

	ComponentCamera* ui_camera = nullptr;

	std::vector<UI_Element*> elements;

	float2 drag_start = float2::zero;
	float2 mouse_pos = float2::zero;

};

BE_END_NAMESPACE
#endif

