#ifndef __COMPONENTPROGRESSBAR_H__
#define __COMPONENTPROGRESSBAR_H__
#pragma once

#include "UI_Element.h"
#include "Math.h"
#include "Color.h"

BE_BEGIN_NAMESPACE

class ComponentCanvas;
class ResourceTexture;

class BROKEN_API ComponentProgressBar : public UI_Element
{
public:
	ComponentProgressBar(GameObject* gameObject);
	virtual ~ComponentProgressBar();

	void Update() override;

	void Draw();
	void DrawPlane(Color color, float percentage = 100.0f);

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

	static inline Component::ComponentType GetType() { return Component::ComponentType::ProgressBar; }

	//Scripting function
	void SetPercentage(float p) { percentage = p; }
	float GetPercentage() { return percentage; }

private:
	Color colorP1 = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color colorP2 = { 1.0f, 1.0f, 1.0f, 1.0f };
	float percentage = 100.0f;

public:
	ComponentCanvas* canvas = nullptr;
	ResourceTexture* texture = nullptr;
};

BE_END_NAMESPACE
#endif
