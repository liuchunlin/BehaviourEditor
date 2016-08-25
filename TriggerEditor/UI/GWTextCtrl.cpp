#include "GWTextCtrl.h"

BEGIN_EVENT_TABLE(GWTextCtrl, wxTextCtrl)
EVT_CHAR(GWTextCtrl::OnChar)
END_EVENT_TABLE()


void GWTextCtrl::OnChar(wxKeyEvent& event)
{
	event.Skip();
}
