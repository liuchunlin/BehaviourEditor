
#pragma once
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class GWFrame : public wxFrame
{
public:
	GWFrame(wxWindow *parent,
		wxWindowID winid,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxFrameNameStr)
		: wxFrame(parent, winid, title, pos, size, style, name)
	{
	}
	virtual void Resize() {}
};
