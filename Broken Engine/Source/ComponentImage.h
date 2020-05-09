#ifndef __COMPONENTIMAGE_H__
#define __COMPONENTIMAGE_H__
#pragma once

#include "UI_Element.h"
#include "Math.h"

BE_BEGIN_NAMESPACE

class ComponentCanvas;
class ResourceTexture;

class BROKEN_API ComponentImage : public UI_Element
{
public:
	ComponentImage(GameObject* gameObject);
	virtual ~ComponentImage();

	void Update() override;

	void Draw();

	static inline Component::ComponentType GetType() { return Component::ComponentType::Image; }

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

public:
	bool resize = true;
	float4 img_color = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
	ComponentCanvas* canvas = nullptr;
	ResourceTexture* texture = nullptr;
};

BE_END_NAMESPACE
#endif
