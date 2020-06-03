#pragma once
#include "Component.h"
#include "Math.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE

class GameObject;
class ComponentCanvas;

class BROKEN_API UI_Element : public Component
{
public:
	UI_Element(GameObject* gameObject, Component::ComponentType Type) : Component(gameObject, Type) {}
	virtual ~UI_Element() {};

	void Scale(float2 size) { size2D = size; }
	void Move(float2 pos) { position2D = pos; }
	void Rotate(float rot) { rotation2D = rot; }

	virtual void Draw() = 0;
	virtual json Save() const = 0;
	virtual void Load(json& node) = 0;
	virtual void CreateInspectorNode() = 0;
	void SetInteractable(bool value) { interactable = value; };

public:
	bool visible = true;
	bool draggable = true;
	bool interactable = true;

	float2 size2D = { 50,50 };
	float2 position2D = { 0,0 };
	float rotation2D = 0.0f;
	ComponentCanvas* canvas = nullptr;

	int priority = 0;
};

BE_END_NAMESPACE