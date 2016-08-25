#pragma once
#include "wx/wxprec.h"
#include "wx/wx.h"

#include "wx/control.h"

class GWTextCtrl : public wxTextCtrl
{
public:
	GWTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value,
		const wxPoint &pos, const wxSize &size, int style = 0, const wxValidator& rValidator = wxDefaultValidator)
		: wxTextCtrl(parent, id, value, pos, size, style, rValidator)
	{
	}

	void OnChar(wxKeyEvent& event);
private:
	DECLARE_EVENT_TABLE()
};