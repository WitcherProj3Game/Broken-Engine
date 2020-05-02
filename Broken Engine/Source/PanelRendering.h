#ifndef __PANEL_RENDERING_H__
#define __PANEL_RENDERING_H__

#include "ModuleRenderer3D.h"
#include "Panel.h"
#include "Math.h"

class PanelRendering : public Panel
{
public:

	PanelRendering(char* name);
	~PanelRendering();

	bool Draw();

private:

	float m_GammaCorretionValue = 1.0f;
	float3 m_AmbientColorValue = float3::one;

	//Alpha Functions
	std::vector<const char*> m_AlphaFunctionsVec;
	Broken::AlphaFunction m_CurrentAlphaFunc;
};

#endif