#ifndef __COMPONENTCIRCULARBAR_H__
#define __COMPONENTCIRCULARBAR_H__
#pragma once

#include "UI_Element.h"
#include "Math.h"
#include "Color.h"

BE_BEGIN_NAMESPACE

class ComponentCanvas;
class ResourceTexture;

class BROKEN_API ComponentCircularBar : public UI_Element
{
public:
	ComponentCircularBar(GameObject* gameObject);
	virtual ~ComponentCircularBar();

	void Update() override;

	void Draw();
	void DrawCircle(Color color, bool axis = 0, float percentage = 100.0f); //axis 0(X), 1(Y)

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

	static inline Component::ComponentType GetType() { return Component::ComponentType::CircularBar; }

	// --- Scripting functions ---
	void SetPercentage(float p) { percentage = p; }
	float GetPercentage() { return percentage; }

	void SetTopColor(Color color) { colorP1 = color; }
	const Color GetTopColor() const { return colorP1; }
	void SetBotColor(Color color) { colorP2 = color; }
	const Color GetBotColor() const { return colorP2; }

public:
	bool axis = 0;

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