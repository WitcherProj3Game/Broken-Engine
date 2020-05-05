#pragma once
#include "Component.h"
#include "Math.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE

class GameObject;

class BROKEN_API UI_Element : public Component
{
public:
	UI_Element(GameObject* gameObject, Component::UIType Type) : Component(gameObject, Component::ComponentType::UI_Element), ui_type(Type) {}
	virtual ~UI_Element() {};

	UI_Element::UIType GetUIType() { return ui_type; }
	void Scale(float2 size) { size2D = size; }
	void Move(float2 pos) { position2D = pos; }
	void Rotate(float rot) { rotation2D = rot; }

	virtual void Draw() {};
	virtual json Save() const = 0;
	virtual void Load(json& node) {};
	virtual void CreateInspectorNode() {};

public:
	bool visible = true;
	bool draggable = true;
	bool interactable = true;

	float2 size2D = { 50,50 };
	float2 position2D = { 0,0 };
	float rotation2D = 0.0f;

	int priority = 0;

protected:
	Component::UIType ui_type = Component::UIType::UNKNOWN;
};

BE_END_NAMESPACE