#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/dialog.h"
#include "wx/artprov.h"
#include "wx/listctrl.h"
#include "GWHyperlink.h"
#include "CancelableWindow.h"

class wxRichTextCtrl;
class wxRichTextEvent;
namespace Space
{
	class SNode;
}
namespace TEE
{
	class NodeBase;
}

class OptionEditDlg : public wxDialog
{
public:
	OptionEditDlg(wxWindow *parent, TEE::NodeBase* pOrgNode);
	~OptionEditDlg();
	bool DoShowModal();
	void OnEditNotesChanged(wxRichTextEvent& event);
private:
	void			InitContents();
	void			InitControls();
private:
	DECLARE_EVENT_TABLE()

	wxRichTextCtrl* m_pECAEdit;

	TEE::NodeBase* m_pOrgNode;
	TEE::NodeBase* m_pEditCopy;
};
