#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/dialog.h"
#include "wx/artprov.h"
#include "wx/listctrl.h"

class wxListCtrl;
class wxListEvent;
class GWTextCtrl;
class wxChoice;
class wxCheckBox;

namespace Space
{
	class SNode;
}
namespace TEE
{
	class NodeBase;
}

class SelOptionDlg : public wxDialog
{
public:
	SelOptionDlg(wxWindow *parent, Space::SNode* pOrgNode);
	~SelOptionDlg();

	Space::SNode*	EditShowModal();
private:
	void			OnVarTypeChoice( wxCommandEvent &event );
	void			OnButton( wxCommandEvent &event );

	void			InitContents();
	void			InitControls();

	wxButton		*m_pOk;
	wxChoice		*m_pVarType;

	//½Úµã
	Space::SNode	*m_pOrgNode;
	Space::SNode	*m_pWorkNode;
	TEE::NodeBase	*m_pTEETmp;
	DECLARE_EVENT_TABLE()
};

