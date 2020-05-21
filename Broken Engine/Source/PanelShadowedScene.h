#ifndef __PANEL_SHADOWED_SCENE_H__
#define __PANEL_SHADOWED_SCENE_H__

#include "Panel.h"

class PanelShadowedScene : public Panel
{
public:

	PanelShadowedScene(char* name);
	~PanelShadowedScene();

	bool Draw();

private:

	float width = 0.0f, height = 0.0f;
	float posX = 0.0f, posY = 0.0f;
};

#endif