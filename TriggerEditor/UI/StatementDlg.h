#include "wx/wxprec.h"
#include "wx/wx.h"
#include "GWHyperlink.h"

#include "EnumType.h"
#include <vector>
using namespace std;

class wxSearchCtrl;
namespace Space
{
	class SNode;
	class ICommand;
	enum SPACE_NODE_TYPE;
}
namespace TEE
{
	enum NODE_TYPE;
}

// Action, Function, condition, Event的修改和创建对话框
class StatementDlg: public wxDialog
{
public:
	StatementDlg(wxWindow *parent, Space::SNode *pStatement, Space::SPACE_NODE_TYPE eCreationType, TEE::NODE_TYPE eTEEType );
	~StatementDlg();

	//事件
	void				OnSize(wxSizeEvent& event);
	void				OnIdle(wxIdleEvent& event);
	void				OnSearchEditChanged( wxCommandEvent &event );
	void				OnStatementChoiceChanged( wxCommandEvent &event );
	void				OnHyperLinkEvent( GWHyperlinkEvent &event );
	void				OnButton( wxCommandEvent& event );

	//逻辑
	Space::SNode*		DoShowModal();
	intptr_t			GetDocumentID();
protected:
	DECLARE_EVENT_TABLE()
protected:
	//逻辑
	void				UpdateParameterEditor();
	void				InitContents();
	void				UpdateStatementsChoice( wxString tag );
	void				RecreateNewStatement();
	void				InitControls(wxWindow *parent);
	void				ResizeControls();

protected:
	//控件
	wxSearchCtrl*		m_searchEdit;
	wxChoice*			m_pStatementsChoice;
	wxButton*			m_btnOk;
	wxStaticBox*		m_pContextBox;

	//可选 文本
	vector<wxControl*>	m_ContextCtrls;
	bool				m_bRequireResize;

	//树信息
	Space::SNode*		m_pOrgNode;
	Space::SNode*		m_pNewNode;
	DWORD				m_eOpState;
	Space::SPACE_NODE_TYPE m_eCreationType;
	TEE::NODE_TYPE		m_eTEEType;
	wxString			m_TypeName;
};
