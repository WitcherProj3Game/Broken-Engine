#ifndef __COMPONENTIMAGE_H__
#define __COMPONENTIMAGE_H__
#pragma once

#include "UI_Element.h"
#include "Math.h"

BE_BEGIN_NAMESPACE

class ComponentCanvas;
class ResourceTexture;
class ResourceMesh;

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

	// anims
	void PlayAnimation();
	void NextAnimationFrame();
	void CreateAnimation(uint w, uint h);

	// UI Functions
	void Scale(float2 size) { size2D = size; }
	void Move(float2 pos) { position2D = pos; }
	void Rotate(float rot) { rotation2D = rot; }

public:
	bool resize = true;
	bool fullscreen = false;
	float4 img_color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// anims
	// animation_frames [FRAME][VERTEX]
	std::vector<ResourceMesh*> animation_frames;
	bool animation = false;
	bool animation_created = false;

	int tile_division_x = 0;
	int tile_division_y = 0;
	int start_frame = 0;
	int end_frame = 1;
	bool loop = false;
	bool animation_finished = false;

	float time = 0;
	int current_frame = 0;
	float animation_time = 1;

	bool play_animation_editor = false;
	bool controlled_by_sripting = false;

	ResourceMesh* plane = nullptr;

	// Progress bar inspector
	bool is_progress_bar = false;

public:
	ResourceTexture* texture = nullptr;
};

BE_END_NAMESPACE
#endif
