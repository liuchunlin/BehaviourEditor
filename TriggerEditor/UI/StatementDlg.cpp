#include "StatementDlg.h"
#include <wx/srchctrl.h>
#include "Vek/Base/ObjLink.h"
#include "Utils.h"
#include "TgrIDs.h"
#include "ParamDlg.h"
#include "ParameterEditor.h"
#include "EditTree.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
#include "CmdManager.h"
#include "SFunction.h"

BEGIN_EVENT_TABLE(StatementDlg, wxDialog)
EVT_SIZE			(StatementDlg::OnSize)
EVT_IDLE			(StatementDlg::OnIdle)
EVT_BUTTON			(wxID_ANY,						StatementDlg::OnButton)
EVT_TEXT			(STATEMENTDLG_SEARCH_EDIT,		StatementDlg::OnSearchEditChanged)
EVT_CHOICE			(STATEMENTDLG_CHOICE_STATEMENT,	StatementDlg::OnStatementChoiceChanged)
EVT_HYPERLINK		(TGR_MAIN_HYPERLINK,			StatementDlg::OnHyperLinkEvent)
END_EVENT_TABLE()

StatementDlg::StatementDlg(wxWindow *parent, Space::SNode *pStatement, Space::SPACE_NODE_TYPE eCreationType, TEE::NODE_TYPE eTEEType)
	: m_pOrgNode(pStatement)
	, m_pNewNode(NULL)
	, m_eOpState(ONS_UNCHANGED)
	, m_eCreationType(eCreationType)
	, m_eTEEType(eTEEType)
	, m_bRequireResize(false)
{
	//控件
	InitControls(parent);

	//内容
	InitContents();
}

StatementDlg::~StatementDlg()
{
}

void	StatementDlg::InitControls(wxWindow *parent)
{
	m_TypeName = Space::SpaceManager::GetTypeBaseName( (Space::SPACE_NODE_TYPE)m_eCreationType ).c_str();

	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create( parent, wxID_ANY, TEXT("设置 ") + m_TypeName, wxDefaultPosition, wxSize(500, 330), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );
	SetAffirmativeId(TGR_SET_EVENT_CANCEL);
	SetEscapeId(wxID_ANY);

	const int iSpace = 4;

	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	m_searchEdit = new wxSearchCtrl(this, STATEMENTDLG_SEARCH_EDIT, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	m_searchEdit->ShowCancelButton(true);
	m_pStatementsChoice = new wxChoice( this, STATEMENTDLG_CHOICE_STATEMENT, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT );
	m_pContextBox = new wxStaticBox( this, wxID_ANY, m_TypeName + TEXT(" 文本"), wxDefaultPosition, wxSize(450, 160) );
	main_sizer->AddSpacer(5);
	main_sizer->Add(m_searchEdit, 0, wxALL | wxEXPAND, iSpace);
	main_sizer->Add(m_pStatementsChoice, 0, wxALL | wxEXPAND, iSpace);
	main_sizer->Add(m_pContextBox, 0, wxALL | wxEXPAND, iSpace);

	wxButton* btnCancel = new wxButton( this, TGR_SET_EVENT_CANCEL, TEXT("取消") );
	m_btnOk = new wxButton( this, TGR_SET_EVENT_OK, TEXT("确定") );
	m_btnOk->SetFocus();
	m_btnOk->SetDefault();
	wxBoxSizer* dialogButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	dialogButtonSizer->Add(10, 0, 1);
	dialogButtonSizer->Add(m_btnOk, 0, wxRight, iSpace);
	dialogButtonSizer->Add(10, 0); // spacer
	dialogButtonSizer->Add(btnCancel, 0, wxRight, iSpace);
	main_sizer->Add(dialogButtonSizer, 0, wxALL | wxEXPAND, 2);

	SetSizer(main_sizer);
	GetSizer()->SetSizeHints(this);
	GetSizer()->Fit(this);
}

int FindChoiceItemByData(wxChoice* pChoice, void* data)
{
	if (pChoice == nullptr) return -1;
	unsigned int count = pChoice->GetCount();
	for (unsigned int index = 0; index < count; ++index)
	{
		if (pChoice->GetClientData(index) == data)
			return index;
	}
	return -1;
}

void	StatementDlg::InitContents()
{
	UpdateStatementsChoice(wxString());

	if( m_pOrgNode != NULL )
	{
		if (m_pOrgNode->GetErrorMask() != Space::VS_OK)
		{
			Space::GSpaceMgr->VerifyParameters(m_pOrgNode);
		}

		int iSel = -1;
		if (m_pOrgNode->IsA(Space::SNT_SFUNCCALL))
		{
			iSel = FindChoiceItemByData(m_pStatementsChoice, m_pOrgNode->Definition());
		}
		else
		{
			iSel = FindChoiceItemByData(m_pStatementsChoice, m_pOrgNode->TEETmp());
		}
		m_pStatementsChoice->Select( iSel );
		UpdateParameterEditor();
	}
	else					//new
	{
		m_eOpState |= ONS_NEW_NODE;
		RecreateNewStatement();
		m_searchEdit->SetFocus();
	}
}

void StatementDlg::OnSize(wxSizeEvent& event)
{
	m_bRequireResize = true;
	event.Skip();
}

void StatementDlg::OnIdle(wxIdleEvent& event)
{
	if (m_bRequireResize)
	{
		ResizeControls();
		m_bRequireResize = false;
		Refresh();
	}
	event.Skip();
}

void StatementDlg::ResizeControls()
{
	if (m_pContextBox != NULL)
	{
		wxPoint beginPos = m_pContextBox->GetPosition();
		beginPos += wxPoint(8, 16);
		wxSize allSize = m_pContextBox->GetSize();

		ParamEditor::Resize(beginPos, allSize, m_ContextCtrls);
	}
}

void	StatementDlg::UpdateStatementsChoice( wxString filterTag )
{
	m_pStatementsChoice->Clear();
	wstring strFilter = filterTag.Trim().wc_str();

	//TEE
	vector<TEE::NodeBase*> nodes;
	TEE::GTEEMgr->GetNodesByType((TEE::NODE_TYPE)m_eTEEType, nodes);
	for( vector<TEE::NodeBase*>::iterator iter(nodes.begin()); iter != nodes.end(); ++iter )
	{
		TEE::NodeBase* pNode = (*iter);
		if( m_pOrgNode != NULL && m_pOrgNode->Parent() != NULL && !m_pOrgNode->Parent()->IsStatementRoot() )
		{
			if( pNode->CodeName() == _T("MultiIF") ||
				pNode->CodeName() == _T("ForEachActions") ||
				pNode->CodeName() == _T("ForEachAActions") ||
				pNode->CodeName() == _T("ForEachBActions") )
			continue;
		}

		if (pNode->IsA(TEE::NT_FUNCTION) && !pNode->IsVoidFunction() && !static_cast<TEE::Function*>(pNode)->m_canBeAction)
			continue;

		wstring nodeName = pNode->DisplayName(true);
		if( nodeName.empty() ) continue;
		if (pNode->IsA(TEE::NT_FUNCTION) && !pNode->IsVoidFunction())
		{
			TEE::Function* pFunction = static_cast<TEE::Function*>(pNode);
			TEE::NodeBase* pReturnTypeOption = TEE::TEEManager::GetInstance()->FindOptionFromCode(pFunction->m_ReturnCode);
			if (pReturnTypeOption)
			{
				nodeName += TEXT(" -> ") + pReturnTypeOption->DisplayName();
			}
		}

		if (!strFilter.empty() && ci_find_substr(nodeName, strFilter) == -1) continue;

		m_pStatementsChoice->Append(nodeName.c_str(), reinterpret_cast<void*>(pNode));
	}

	//SFunction
	if (IsActionType(m_eCreationType))
	{
		vector<Space::SFunction*> functions;
		Vek::ObjLink<Space::SFunction>::VisitAll([&functions](Space::SFunction* pFunction)->void {
			if (Space::IsNodeInSpace(pFunction))
				functions.push_back(pFunction);
		});
		for (size_t index = 0; index < functions.size(); ++index)
		{
			Space::SFunction* func = functions[index];
			if (!func->IsVoidFunction()) continue;

			wstring nodeName = SFuncTag + TEXT(" - ") + func->GetReadText();
			if (!strFilter.empty() && ci_find_substr(nodeName, strFilter) == -1) continue;

			m_pStatementsChoice->Append(nodeName.c_str(), reinterpret_cast<void*>(func));
		}
	}

	m_pStatementsChoice->Select(0);
}

void	StatementDlg::OnButton(wxCommandEvent& event)
{
	this->EndModal( event.GetId() );
	event.Skip();
}
void	StatementDlg::OnSearchEditChanged( wxCommandEvent &event )
{
	UpdateStatementsChoice( event.GetString().wc_str() );
	RecreateNewStatement();
}

void	StatementDlg::OnStatementChoiceChanged( wxCommandEvent &event )
{
	RecreateNewStatement();
}

void	StatementDlg::RecreateNewStatement()
{
	m_eOpState |= ONS_ORG_NODE_OVERRIDE;

	SAFE_DELETE(m_pNewNode);

	int iSelect = m_pStatementsChoice->GetSelection();
	if( iSelect != -1 )
	{
		Vek::TreeNodeBase* base = static_cast<Vek::TreeNodeBase*>(m_pStatementsChoice->GetClientData(iSelect));
		m_pNewNode = Space::BuildStatementSOrT(base);
		UpdateParameterEditor();
	}
}

void	StatementDlg::OnHyperLinkEvent( GWHyperlinkEvent& event )
{
	Space::SNode* pOrgNode = static_cast<Space::SNode*>( event.GetClientData() );
	if( pOrgNode == NULL )
		return;

	if( pOrgNode->ClassType() == Space::SNT_CONDITION )
	{
		StatementDlg dlg(GetParent(), pOrgNode, pOrgNode->ClassType(), TEE::NT_CONDITION );
		dlg.DoShowModal();
	}
	else if( pOrgNode->IsVoidFunctionCall() || pOrgNode->IsA(Space::SNT_SET_VARIABLE) )
	{
		StatementDlg dlg(GetParent(), pOrgNode, pOrgNode->ClassType(), TEE::NT_FUNCTION );
		dlg.DoShowModal();
	}
	else if( pOrgNode->ClassType() == Space::SNT_EVENT )
	{
		StatementDlg dlg(GetParent(), pOrgNode, pOrgNode->ClassType(), TEE::NT_EVENT );
		dlg.DoShowModal();
	}
	else
	{
		DWORD ePrefType = PDT_NORMAL_PARAM;
		wstring selfType;
		if( pOrgNode->TEETmp() != NULL )
		{
			pOrgNode->TEETmp()->GetOptionCode( selfType );
		}

		if( pOrgNode->Parent() != NULL && pOrgNode->Parent()->Name() == _T("SetVariable") )
		{
			if( pOrgNode->Next() != NULL )
				ePrefType = PDT_VARIABLE_CHOICE;
			else
			{
				if( _T("TRIGGER") == selfType )
				{
					ePrefType = PDT_TRIGGER_CHOICE;
				}
				else
				{
					ePrefType = PDT_VARIABLE_VALUE;
				}
			}
		}
		else if( _T("TRIGGER") == selfType )
		{
			ePrefType = PDT_TRIGGER_CHOICE;
		}

		ParamDlg dlg( pOrgNode, ePrefType );
		dlg.PrefShowModal();
	}
	UpdateParameterEditor();
	m_eOpState |= ONS_ORG_NODE_CHANGED;
}

intptr_t StatementDlg::GetDocumentID()
{
	EditTree* tgrTree = dynamic_cast<EditTree*>(GetParent());
	return tgrTree ? tgrTree->GetDocumentID() : (intptr_t)-1;
}

Space::SNode*	StatementDlg::DoShowModal()
{
	Space::SNode* pOrgBaseNode = Space::SNode::Clone( m_pOrgNode );
	if( ShowModal() == TGR_SET_EVENT_OK )
	{
		if( m_pNewNode != NULL && m_pOrgNode != NULL && m_pOrgNode->Parent() != NULL && m_pOrgNode->Parent()->IsStatementRoot() )
		{
			GCmdMgr->AddCommand(GetDocumentID(), new Space::ExChangeNodeCmd( m_pOrgNode, m_pNewNode ));
		}
		else if( m_pNewNode != NULL && m_pOrgNode != NULL )
		{
			Space::SNode::Swap( m_pOrgNode, m_pNewNode );
			m_pOrgNode->Remove();
			m_pNewNode->DominoOffect();
		}
		else if( m_eOpState == ONS_ORG_NODE_CHANGED )
		{
			GCmdMgr->AddCommand(GetDocumentID(), new Space::ExChangeNodeChildCmd( m_pOrgNode, pOrgBaseNode ), false);
			pOrgBaseNode = NULL;
		}
	}
	else
	{
		SAFE_DELETE( m_pNewNode );
		if( m_eOpState == ONS_ORG_NODE_CHANGED )
		{
			Space::ICommand *pCmd = new Space::ExChangeNodeChildCmd( m_pOrgNode, pOrgBaseNode );
			pCmd->Execute();
			pOrgBaseNode = NULL;
			SAFE_DELETE( pCmd );
		}
		m_eOpState = ONS_UNCHANGED;
	}
	SAFE_DELETE( pOrgBaseNode );
	return m_pNewNode;
}

void	StatementDlg::UpdateParameterEditor()
{
	wxPoint beginPos = m_pContextBox->GetPosition();
	beginPos += wxPoint(8,16);
	wxSize allSize = m_pContextBox->GetSize();

	Space::SNode* pWorkingNode = m_pNewNode != NULL ?  m_pNewNode: m_pOrgNode;

	Space::ErrorMask nValid = Space::SNode::CheckErrorTree( pWorkingNode );
	m_btnOk->Enable( nValid == Space::VS_OK );

	ParamEditor::Build( this, beginPos, allSize, pWorkingNode, m_ContextCtrls );
}
