#pragma once
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wx/propdlg.h"

#include "wx/datetime.h"
#include "wx/image.h"
#include "wx/bookctrl.h"
#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "wx/sysopt.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include "wx/choice.h"
#include  "wx/checklst.h"
namespace TEE
{
	class NodeBase;
}
class TagDialog: public wxDialog
{
	DECLARE_CLASS(TEEPropertyDialogBase)
public:
	TagDialog( );
	~TagDialog();

	void				OnButton(wxCommandEvent& event);
	const wxString&		GetTagResults(){ return m_Tag; }		
	void				Init( const wxString& tags );

protected:
	void				OnCheckboxToggle(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

	wxCheckListBox		*m_pTagListBox;
	wxString			m_Tag;
};
