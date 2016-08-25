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

#include <set>
using namespace std;
namespace TEE
{
	class NodeBase;
}
class IncludeDialog: public wxDialog
{
	DECLARE_CLASS(TEEPropertyDialogBase)
public:
	IncludeDialog( const set<wstring>& includes );
	~IncludeDialog();

	void				OnButton(wxCommandEvent& event);
	void				Init( );
	const set<wstring>& GetIncludes(){ return m_Includes; }

	void				OnCheckboxToggle(wxCommandEvent& event);

protected:

	DECLARE_EVENT_TABLE()
protected:
	wxCheckListBox		*m_pTagListBox;
	set<wstring>		m_Includes;
};
