#ifndef __COMPONENTCANVAS_H__
#define __COMPONENTCANVAS_H__
#pragma once

#include "UI_Element.h"
#include "Color.h"
#include "Math.h"
#include <vector>

BE_BEGIN_NAMESPACE

class BROKEN_API ComponentCanvas : public UI_Element
{
public:
	ComponentCanvas(GameObject* gameObject);
	virtual ~ComponentCanvas();

	void Update() override;

	void Draw() const;
	void AddElement(UI_Element* element) { elements.push_back(element); }
	static inline Component::ComponentType GetType() { return Component::ComponentType::UI_Element; }
	static inline Component::UIType GetUIType() { return Component::UIType::Canvas; }

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

	void UpdatePosition(float2& increment);

public:
	ComponentCanvas* canvas = nullptr;

private:
	std::vector<UI_Element*> elements;
};

BE_END_NAMESPACE

#endif

