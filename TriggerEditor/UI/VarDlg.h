#include "wx/wxprec.h"
#include "wx/wx.h"

#include "wx/dialog.h"
#include "wx/artprov.h"
#include "wx/listctrl.h"
#include "GWHyperlink.h"
#include "CancelableWindow.h"

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
class VarListDlg : public wxDialog, public CancelableWindow
{
public:
    VarListDlg(wxWindow *parent);
	~VarListDlg();

	void			RefreshLayout( Space::SNode* pNode );
	virtual intptr_t GetDocumentID();

private:
    void			OnSelectItem(wxListEvent &event);
    void			OnChooseClient(wxCommandEvent &event);
    void			OnContextMenu(wxContextMenuEvent& event);
	void			OnListItemActivated(wxListEvent& event);
	void			OnProcessAny(wxCommandEvent& event);
	//
	void			InitContents();
	void			InitControls();
private:
	wxListCtrl		*m_pList;
	wxChoice		*m_TypeChoice;
    DECLARE_EVENT_TABLE()
};


class VarEditDlg : public wxDialog
{
public:
	VarEditDlg(wxWindow *parent, Space::SNode* pOrgNode, Space::SNode* pVarRoot);
	~VarEditDlg();

	Space::SNode*	EditShowModal();
private:
	void			OnVarNameText( wxCommandEvent& event );
	void			OnVarTypeChoice( wxCommandEvent &event );
	void			OnButton( wxCommandEvent &event );
	void			OnHyperLinkEvent( GWHyperlinkEvent& event );
	void			OnAryCheck( wxCommandEvent& event );

	void			InitContents();
	void			InitControls();
	void			RefreshLayout();

	wxButton		*m_pOk;
	GWTextCtrl		*m_pVarName;
	wxChoice		*m_pVarType;
	wxCheckBox		*m_pAryCheck;
	wxSpinCtrl		*m_pArySize;
	GWHyperlinkCtrl	*m_pDefaultValue;

	//½Úµã
	Space::SNode	*m_pOrgNode;
	Space::SNode	*m_pVarRoot;
	Space::SNode	*m_pWorkNode;
	TEE::NodeBase	*m_pTEETmp;
	DECLARE_EVENT_TABLE()
};

