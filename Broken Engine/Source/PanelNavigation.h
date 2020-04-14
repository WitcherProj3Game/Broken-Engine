#ifndef __PANELNAVIGATION_H__
#define __PANELNAVIGATION_H__
#pragma once

#include "Panel.h"

class PanelNavigation : public Panel {
public:

	PanelNavigation(char* name);
	~PanelNavigation();

	bool Draw();

	void SetOnOff(bool status) override;
	void OnOff() override;

private:

	bool manualVoxel = false;
	bool navigationStatic = false;
	bool popupNavigationFlag = false;
	unsigned int popupArea = 0;
	bool openPopup = false;
	bool affectChilds = false;
	bool doChange = false;


};

#endif

