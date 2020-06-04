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
	struct PrioritySort {
		bool operator()(UI_Element* const& node1, UI_Element* const& node2);
	};

public:
	ComponentCanvas(GameObject* gameObject);
	virtual ~ComponentCanvas();

	void Update() override;

	void Draw();
	void AddElement(UI_Element* element) { elements.push_back(element); }
	void RemoveElement(UI_Element* element);

	float2 GetParentPos();
	float2 GetFinalPosition();

	static inline Component::ComponentType GetType() { return Component::ComponentType::Canvas; }


	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

	void OrderCanvas();

	//void UpdatePosition(float2& increment);
private:
	std::vector<UI_Element*> elements;

	float2 position2DLocal = float2::zero;
};

BE_END_NAMESPACE

#endif

