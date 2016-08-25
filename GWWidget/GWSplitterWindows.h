#pragma once


#include "wx/splitter.h"

class GWFrame;
class GWSplitterWindow : public wxSplitterWindow
{
public:
	GWSplitterWindow(wxWindow *parent, GWFrame *frame);

	// event handlers
	void OnPositionChanged(wxSplitterEvent& event);
	void OnPositionChanging(wxSplitterEvent& event);
	void OnDClick(wxSplitterEvent& event);
	void OnUnsplitEvent(wxSplitterEvent& event);

private:
	GWFrame *m_frame;

	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(GWSplitterWindow)
};