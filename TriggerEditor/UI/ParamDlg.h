#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/richtext/richtextctrl.h"
#include "wx/radiobox.h"
#include "wx/spinctrl.h"

#include "GWHyperlink.h"
#include "GWTextCtrl.h"

#include "EnumType.h"

#include <vector>
#include <map>
using namespace std;

namespace Space
{
	class SNode;
}
namespace TEE
{
	class NodeBase;
}
class ParamDlg: public wxDialog
{
	DECLARE_CLASS(ParamDlg)
public:
	ParamDlg( Space::SNode *pNode, DWORD ePrefType = PDT_NORMAL_PARAM );
	~ParamDlg();

	//事件
	void				OnSize( wxSizeEvent& event );
	void				OnIdle( wxIdleEvent& event );
	void				OnAdjustComboChanged( wxCommandEvent &event );
	void				OnAdjustComboTextChanged( wxCommandEvent &event );
	void				OnContextVariableChoice( wxCommandEvent &event );
	void				OnLocalVariableChoice(wxCommandEvent &event);
	void				OnParamVariableChoice(wxCommandEvent &event);
	void				OnGlobalVariableChoice( wxCommandEvent &event );
	void				OnFunctionChoice( wxCommandEvent &event );
	void				OnHyperLinkLeftUp( GWHyperlinkEvent& event );
	void				OnValueText( wxCommandEvent& event );
	void				OnBoolRadio( wxCommandEvent &event );
	void				OnButton( wxCommandEvent& event );
	void				OnRadioButton( wxCommandEvent &event );

	Space::SNode*		PrefShowModal();
protected:
	DECLARE_EVENT_TABLE()
protected:
	enum
	{
		MASK_OK_BUTTON		= 0x00000001,
		MASK_PREFERENCES	= 0x00000002,

		MASK_EVERYTHING		= 0xFFFFFFFF,
	};

	//逻辑
	void				InitControls();
	void				InitContents();
	void				ResizeControls();
	bool				CheckRadio( int eType );
	void				UpdateLayout( int iMask = MASK_EVERYTHING );
	void				UpdateAdjustList();
	void				UpdateGlobalVariableList();
	void				UpdateContextVariableList();
	void				UpdateLocalVariableList();
	void				UpdateParamVariableList();
	Space::SNode*		GetNewWorkNode();
protected:
	//树信息
	DWORD				m_ePrefType;
	Space::SNode		*m_pOrgNode;
	TEE::NodeBase		*m_pTMPOption;
	map<int, Space::SNode*>	m_NewNodes;

	//列表内容
	map<wstring,void*>	m_FunctionContents;

	//控件
	wxComboBox			*m_pAdjustCombo;
	wxChoice			*m_pContextVariableChoice;
	wxChoice			*m_pLocalVariableChoice;
	wxChoice			*m_pParamVariableChoice;
	wxChoice			*m_pGlobalVariableChoice;
	wxChoice			*m_pFunctionChoice;
	GWTextCtrl			*m_pEditValue;
	wxBoxSizer			*m_pAdjustSizer;
	wxBoxSizer			*m_pContextVariableSizer;
	wxBoxSizer			*m_pLocalVariableSizer;
	wxBoxSizer			*m_pParamVariableSizer;
	wxButton			*m_pOk;
	map<int, wxRadioButton*> m_Radios;
	wxStaticBox			*m_pContentBox;
	wxRadioBox			*m_pBoolRadio;
	vector<wxControl*>	m_ContextCtrls;
	bool				m_bRequireResize;
};
