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

// Action, Function, condition, Event���޸ĺʹ����Ի���
class StatementDlg: public wxDialog
{
public:
	StatementDlg(wxWindow *parent, Space::SNode *pStatement, Space::SPACE_NODE_TYPE eCreationType, TEE::NODE_TYPE eTEEType );
	~StatementDlg();

	//�¼�
	void				OnSize(wxSizeEvent& event);
	void				OnIdle(wxIdleEvent& event);
	void				OnSearchEditChanged( wxCommandEvent &event );
	void				OnStatementChoiceChanged( wxCommandEvent &event );
	void				OnHyperLinkEvent( GWHyperlinkEvent &event );
	void				OnButton( wxCommandEvent& event );

	//�߼�
	Space::SNode*		DoShowModal();
	intptr_t			GetDocumentID();
protected:
	DECLARE_EVENT_TABLE()
protected:
	//�߼�
	void				UpdateParameterEditor();
	void				InitContents();
	void				UpdateStatementsChoice( wxString tag );
	void				RecreateNewStatement();
	void				InitControls(wxWindow *parent);
	void				ResizeControls();

protected:
	//�ؼ�
	wxSearchCtrl*		m_searchEdit;
	wxChoice*			m_pStatementsChoice;
	wxButton*			m_btnOk;
	wxStaticBox*		m_pContextBox;

	//��ѡ �ı�
	vector<wxControl*>	m_ContextCtrls;
	bool				m_bRequireResize;

	//����Ϣ
	Space::SNode*		m_pOrgNode;
	Space::SNode*		m_pNewNode;
	DWORD				m_eOpState;
	Space::SPACE_NODE_TYPE m_eCreationType;
	TEE::NODE_TYPE		m_eTEEType;
	wxString			m_TypeName;
};
