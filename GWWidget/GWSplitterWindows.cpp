
#include "GWSplitterWindows.h"
#include "GWFrame.h"

// ----------------------------------------------------------------------------
// MySplitterWindow
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(GWSplitterWindow, wxSplitterWindow)
EVT_SPLITTER_SASH_POS_CHANGED(wxID_ANY, GWSplitterWindow::OnPositionChanged)
EVT_SPLITTER_SASH_POS_CHANGING(wxID_ANY, GWSplitterWindow::OnPositionChanging)

EVT_SPLITTER_DCLICK(wxID_ANY, GWSplitterWindow::OnDClick)

EVT_SPLITTER_UNSPLIT(wxID_ANY, GWSplitterWindow::OnUnsplitEvent)
END_EVENT_TABLE()

GWSplitterWindow::GWSplitterWindow(wxWindow *parent, GWFrame *frame)
: wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NO_XP_THEME|wxSP_3D|wxSP_LIVE_UPDATE )
{
	m_frame = frame;
}

void GWSplitterWindow::OnPositionChanged(wxSplitterEvent& event)
{
	m_frame->Resize();
	event.Skip();
}

void GWSplitterWindow::OnPositionChanging(wxSplitterEvent& event)
{
	m_frame->Resize();
	event.Skip();
}

void GWSplitterWindow::OnDClick(wxSplitterEvent& event)
{
	event.Skip();
}

void GWSplitterWindow::OnUnsplitEvent(wxSplitterEvent& event)
{
	event.Skip();
}