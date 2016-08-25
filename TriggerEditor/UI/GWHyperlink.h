#pragma once
#include "wx/wxprec.h"
#include "wx/wx.h"

#include "wx/control.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#define wxHL_CONTEXTMENU        0x0001
#define wxHL_ALIGN_LEFT         0x0002
#define wxHL_ALIGN_RIGHT        0x0004
#define wxHL_ALIGN_CENTRE       0x0008
#define wxHL_DEFAULT_STYLE      (wxHL_CONTEXTMENU|wxNO_BORDER|wxHL_ALIGN_CENTRE)


// ----------------------------------------------------------------------------
// GWHyperlinkCtrl
// ----------------------------------------------------------------------------

// A static text control that emulates a hyperlink. The link is displayed
// in an appropriate text style, derived from the control's normal font.
// When the mouse rolls over the link, the cursor changes to a hand and the
// link's color changes to the active color.
//
// Clicking on the link does not launch a web browser; instead, a
// HyperlinkEvent is fired. The event propagates upward until it is caught,
// just like a wxCommandEvent.
//
// Use the EVT_HYPERLINK() to catch link events.
class GWHyperlinkCtrl : public wxControl
{
public:
    // Default constructor (for two-step construction).
    GWHyperlinkCtrl() { }

    // Constructor.
    GWHyperlinkCtrl(wxWindow *parent,
                    wxWindowID id,
                    const wxString& label,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxHL_DEFAULT_STYLE)
    {
        (void)Create(parent, id, label, pos, size, style);
    }

    // Creation function (for two-step construction).
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxHL_DEFAULT_STYLE);


    // get/set
    wxColour GetHoverColour() const { return m_hoverColour; }
    void SetHoverColour(const wxColour &colour) { m_hoverColour = colour; }

    wxColour GetNormalColour() const { return m_normalColour; }
    void SetNormalColour(const wxColour &colour);

    wxColour GetVisitedColour() const { return m_visitedColour; }
    void SetVisitedColour(const wxColour &colour);

    void SetVisited(bool visited = true) { m_visited=visited; }
    bool GetVisited() const { return m_visited; }

    // NOTE: also wxWindow::Set/GetLabel, wxWindow::Set/GetBackgroundColour,
    //       wxWindow::Get/SetFont, wxWindow::Get/SetCursor are important !


protected:
    // event handlers

    // Renders the hyperlink.
    void OnPaint(wxPaintEvent& event);

    // Returns the wxRect of the label of this hyperlink.
    // This is different from the clientsize's rectangle when
    // clientsize != bestsize and this rectangle is influenced
    // by the alignment of the label (wxHL_ALIGN_*).
    wxRect GetLabelRect() const;

    // If the click originates inside the bounding box of the label,
    // a flag is set so that an event will be fired when the left
    // button is released.
    void OnLeftDown(wxMouseEvent& event);

    // If the click both originated and finished inside the bounding box
    // of the label, a HyperlinkEvent is fired.
    void OnLeftUp(wxMouseEvent& event);
    void OnRightUp(wxMouseEvent& event);

    // Changes the cursor to a hand, if the mouse is inside the label's
    // bounding box.
    void OnMotion(wxMouseEvent& event);

    // Changes the cursor back to the default, if necessary.
    void OnLeaveWindow(wxMouseEvent& event);

    // handles "Copy URL" menuitem
    void OnPopUpCopy(wxCommandEvent& event);

    // Refreshes the control to update label's position if necessary
    void OnSize(wxSizeEvent& event);


    // overridden base class virtuals

    // Returns the best size for the window, which is the size needed
    // to display the text label.
    virtual wxSize DoGetBestSize() const;

    // creates a context menu with "Copy URL" menuitem
    virtual void DoContextMenu(const wxPoint &);

private:
    // Foreground colours for various link types.
    // NOTE: wxWindow::m_backgroundColour is used for background,
    //       wxWindow::m_foregroundColour is used to render non-visited links
    wxColour m_hoverColour;
    wxColour m_normalColour;
    wxColour m_visitedColour;

    // True if the mouse cursor is inside the label's bounding box.
    bool m_rollover;

    // True if the link has been clicked before.
    bool m_visited;

    // True if a click is in progress (left button down) and the click
    // originated inside the label's bounding box.
    bool m_clicking;

private:
    DECLARE_DYNAMIC_CLASS(GWHyperlinkCtrl)
    DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
// GWHyperlinkEvent
// ----------------------------------------------------------------------------

// Declare an event identifier.
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_LOCAL_EVENT_TYPE(GWEVT_COMMAND_HYPERLINK, 3700)
END_DECLARE_EVENT_TYPES()

//
// An event fired when the user clicks on the label in a hyperlink control.
// See HyperlinkControl for details.
//
class GWHyperlinkEvent : public wxCommandEvent
{
public:
    GWHyperlinkEvent() {}
    GWHyperlinkEvent(wxObject *generator, wxWindowID id);

    // default copy ctor, assignment operator and dtor are ok
    virtual wxEvent *Clone() const { return new GWHyperlinkEvent(*this); }

private:

    // URL associated with the hyperlink control that the used clicked on.
    wxString m_url;

    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(GWHyperlinkEvent)
};


// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*GWHyperlinkEventFunction)(GWHyperlinkEvent&);

#define GWHyperlinkEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(GWHyperlinkEventFunction, &func)

#define EVT_HYPERLINK(id, fn) \
    wx__DECLARE_EVT1(GWEVT_COMMAND_HYPERLINK, id, GWHyperlinkEventHandler(fn))

#ifdef _WX_DEFINE_DATE_EVENTS_
    DEFINE_EVENT_TYPE(GWEVT_COMMAND_HYPERLINK)

    IMPLEMENT_DYNAMIC_CLASS(GWHyperlinkEvent, wxCommandEvent)
#endif



