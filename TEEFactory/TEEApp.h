
#pragma once

#include "TEEFrame.h"

class TEEApp : public wxApp
{
public:
	// override base class virtuals
	// ----------------------------

	// this one is called on application startup and is a good place for the app
	// initialization (doing it here and not in the ctor allows to have an error
	// return: if OnInit() returns false, the application terminates)
	virtual bool OnInit();
	virtual int OnExit();
	void CreateStyles();
private:
	bool m_showImages, m_showButtons;
};

DECLARE_APP(TEEApp)

