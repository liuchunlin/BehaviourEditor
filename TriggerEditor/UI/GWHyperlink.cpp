//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "GWHyperlink.h"
#include <Vek/Base/Types.h>

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_DYNAMIC_CLASS(GWHyperlinkCtrl, wxControl)
IMPLEMENT_DYNAMIC_CLASS(GWHyperlinkEvent, wxCommandEvent)
DEFINE_EVENT_TYPE(GWEVT_COMMAND_HYPERLINK)

// reserved for internal use only
#define GWHyperlinkCtrl_POPUP_COPY_ID           16384


// ----------------------------------------------------------------------------
// GWHyperlinkCtrl
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(GWHyperlinkCtrl, wxControl)
    EVT_PAINT(GWHyperlinkCtrl::OnPaint)
    EVT_LEFT_DOWN(GWHyperlinkCtrl::OnLeftDown)
    EVT_LEFT_UP(GWHyperlinkCtrl::OnLeftUp)
    EVT_RIGHT_UP(GWHyperlinkCtrl::OnRightUp)
    EVT_MOTION(GWHyperlinkCtrl::OnMotion)
    EVT_LEAVE_WINDOW(GWHyperlinkCtrl::OnLeaveWindow)
    EVT_SIZE(GWHyperlinkCtrl::OnSize)

    // for the context menu
    EVT_MENU(GWHyperlinkCtrl_POPUP_COPY_ID, GWHyperlinkCtrl::OnPopUpCopy)
END_EVENT_TABLE()

bool GWHyperlinkCtrl::Create(wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style)
{
	if (!wxControl::Create(parent, id, pos, size, style, wxDefaultValidator))
		return false;

    SetLabel(label);

    m_rollover = false;
    m_clicking = false;
    m_visited = false;

    // colours
    m_normalColour = *wxBLUE;
    m_hoverColour = *wxRED;
    SetForegroundColour(m_normalColour);

    // by default the font of an hyperlink control is underlined
    wxFont f = GetFont();
    f.SetUnderlined(true);
    SetFont(f);

    SetInitialSize(size);
    
    return true;
}

wxSize GWHyperlinkCtrl::DoGetBestSize() const
{
    int w, h;

    wxClientDC dc((wxWindow *)this);
    dc.SetFont(GetFont());
    dc.GetTextExtent(GetLabel(), &w, &h);

    wxSize best(w, h);
    CacheBestSize(best);
    return best;
}


void GWHyperlinkCtrl::SetNormalColour(const wxColour &colour)
{
    m_normalColour = colour;
    if (!m_visited)
    {
        SetForegroundColour(m_normalColour);
        Refresh();
    }
}

void GWHyperlinkCtrl::SetVisitedColour(const wxColour &colour)
{
    m_visitedColour = colour;
    if (m_visited)
    {
        SetForegroundColour(m_visitedColour);
        Refresh();
    }
}

void GWHyperlinkCtrl::DoContextMenu(const wxPoint &pos)
{
    wxMenu *menuPopUp = new wxMenu(wxEmptyString, wxMENU_TEAROFF);
    menuPopUp->Append(GWHyperlinkCtrl_POPUP_COPY_ID, _("&Copy URL"));
    PopupMenu( menuPopUp, pos );
	SAFE_DELETE( menuPopUp );
}

wxRect GWHyperlinkCtrl::GetLabelRect() const
{
    // our best size is always the size of the label without borders
    wxSize c(GetClientSize()), b(GetBestSize());
    wxPoint offset;

    // the label is always centered vertically
    offset.y = (c.GetHeight()-b.GetHeight())/2;

    if (HasFlag(wxHL_ALIGN_CENTRE))
        offset.x = (c.GetWidth()-b.GetWidth())/2;
    else if (HasFlag(wxHL_ALIGN_RIGHT))
        offset.x = c.GetWidth()-b.GetWidth();
    else if (HasFlag(wxHL_ALIGN_LEFT))
        offset.x = 0;
    return wxRect(offset, b);
}



// ----------------------------------------------------------------------------
// GWHyperlinkCtrl - event handlers
// ----------------------------------------------------------------------------

void GWHyperlinkCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.SetTextForeground(GetForegroundColour());
    dc.SetTextBackground(GetBackgroundColour());

	wxPoint topLeft = GetLabelRect().GetTopLeft();
	if( m_clicking )
	{
		topLeft += wxPoint(1,1);
	}
    dc.DrawText( GetLabel(), topLeft );
}

void GWHyperlinkCtrl::OnLeftDown(wxMouseEvent& event)
{
    // the left click must start from the hyperlink rect
    m_clicking = GetLabelRect().Contains(event.GetPosition());
	Refresh();
}

void GWHyperlinkCtrl::OnLeftUp(wxMouseEvent& event)
{
    // the click must be started and ended in the hyperlink rect
    if (!m_clicking || !GetLabelRect().Contains(event.GetPosition())) 
        return;

    //SetForegroundColour(m_visitedColour);
    //m_visited = true;
    m_clicking = false;
	Refresh();
	GWHyperlinkEvent linkEvent( this, GetId() );
	linkEvent.SetClientData( static_cast<void*>(this->GetClientData()) );
	GetEventHandler()->ProcessEvent(linkEvent);
}

void GWHyperlinkCtrl::OnRightUp(wxMouseEvent& event)
{
    if( GetWindowStyle() & wxHL_CONTEXTMENU )
        if ( GetLabelRect().Contains(event.GetPosition()) )
            DoContextMenu(wxPoint(event.m_x, event.m_y));
}

void GWHyperlinkCtrl::OnMotion(wxMouseEvent& event)
{
    wxRect textrc = GetLabelRect();

    if (textrc.Contains(event.GetPosition()))
    {
        SetCursor(wxCursor(wxCURSOR_HAND));
        SetForegroundColour(m_hoverColour);
        m_rollover = true;
        Refresh();
    }
    else if (m_rollover)
    {
        SetCursor(*wxSTANDARD_CURSOR);
        SetForegroundColour(!m_visited ? m_normalColour : m_visitedColour);
        m_rollover = false;
        Refresh();
    }
}

void GWHyperlinkCtrl::OnLeaveWindow(wxMouseEvent& WXUNUSED(event) )
{
    // NB: when the label rect and the client size rect have the same
    //     height this function is indispensable to remove the "rollover"
    //     effect as the OnMotion() event handler could not be called
    //     in that case moving the mouse out of the label vertically...

	if (m_rollover)
    {
        SetCursor(*wxSTANDARD_CURSOR);
        SetForegroundColour(!m_visited ? m_normalColour : m_visitedColour);
        m_rollover = false;
		m_clicking = false;
		Refresh();
    }
}

void GWHyperlinkCtrl::OnPopUpCopy( wxCommandEvent& WXUNUSED(event) )
{

}

void GWHyperlinkCtrl::OnSize(wxSizeEvent& WXUNUSED(event))
{
    // update the position of the label in the screen respecting
    // the selected align flag
    Refresh();
}

GWHyperlinkEvent::GWHyperlinkEvent(wxObject *generator, wxWindowID id)
: wxCommandEvent(GWEVT_COMMAND_HYPERLINK, id)
{
	SetEventObject(generator);
}
