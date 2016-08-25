#include "VarDlg.h"

#include "ParamDlg.h"

#include "EnumType.h"
#include "TgrIDs.h"
#include "GWTextCtrl.h"
#include "GWHyperlink.h"
#include "EditTree.h"

#include "CmdManager.h"
#include "SpaceManager.h"

#include "TEEManager.h"
#include "NodeVerify.h"

#include "wx/sizer.h"
#include "wx/imaglist.h"
#include "wx/listctrl.h"
#include "wx/spinctrl.h"
#include <regex>

BEGIN_EVENT_TABLE(VarListDlg, wxDialog)
EVT_LIST_ITEM_SELECTED				(wxID_ANY,	VarListDlg::OnSelectItem)
EVT_LIST_ITEM_ACTIVATED				(wxID_ANY,	VarListDlg::OnListItemActivated)
EVT_CHOICE							(wxID_ANY,	VarListDlg::OnChooseClient)
EVT_CONTEXT_MENU					(VarListDlg::OnContextMenu)
EVT_MENU							(wxID_ANY,	VarListDlg::OnProcessAny)

END_EVENT_TABLE()

VarListDlg::VarListDlg(wxWindow *parent)
:wxDialog( parent, wxID_ANY, _T("变量"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	InitControls();
	InitContents();

	//////////////////////////////////////////////////////////////////////////
}
VarListDlg::~VarListDlg()
{
	Space::SNode *pVarSet = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );
	if( pVarSet != NULL )
	{
		pVarSet->UserData( "list", NULL );
		pVarSet->UserData( "varlistdlg", NULL );
	}
}

void	VarListDlg::InitContents()
{
	m_TypeChoice->Append( _T("- 全部"), (void*)NULL );
	vector<TEE::NodeBase*> Ops;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTIONROOT, Ops);
	vector<TEE::NodeBase*>::iterator iterType( Ops.begin() );
	for( ; iterType != Ops.end(); ++iterType )
	{
		TEE::NodeBase* pNode = (*iterType);
		m_TypeChoice->Append( pNode->DisplayName(), reinterpret_cast<void*>(pNode) );
	}
	m_TypeChoice->Select(0);
	m_TypeChoice->Enable(false);

	vector<Space::SNode*> VarItems;
	Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );
	if (pVarSet) pVarSet->FindChildren(Space::SVerifyAlwaysOk(), VarItems);
	vector<Space::SNode*>::iterator iter(VarItems.begin());
	for( int iIndex = 0; iter != VarItems.end(); ++iter, ++iIndex )
	{
		Space::SNode* pVarItem = (*iter);

		long tmp = m_pList->InsertItem( iIndex, pVarItem->Name().c_str(), 0 );
		m_pList->SetItemData( tmp, long(pVarItem) );

		RefreshLayout( pVarItem );
	}

	if( pVarSet != NULL )
	{
		pVarSet->UserData( "list", reinterpret_cast<void*>(m_pList) );
		pVarSet->UserData( "varlistdlg", reinterpret_cast<void*>(this) );
	}
}
void	VarListDlg::InitControls()
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *subsizer;
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("变量类型:")), 0, wxALIGN_CENTER_VERTICAL);
	m_TypeChoice = new wxChoice(this, wxID_ANY);
	subsizer->Add(m_TypeChoice, 1, wxLEFT, 5);
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 10);

	m_pList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(450, 200), wxLC_REPORT | wxSUNKEN_BORDER);
	m_pList->InsertColumn(0, _T("变量名"), wxLIST_FORMAT_LEFT, 150 );
	m_pList->InsertColumn(1, _T("变量类型"), wxLIST_FORMAT_LEFT, 150 );
	m_pList->InsertColumn(2, _T("初始值"), wxLIST_FORMAT_LEFT, 150 );
	sizer->Add(m_pList, 1, wxEXPAND | wxRIGHT, 10);

	subsizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *ok = new wxButton(this, wxID_OK, _T("确定"));
	ok->SetDefault();
	subsizer->Add(ok, 0, wxALIGN_RIGHT | wxALL, 3);

	wxButton *cancel = new wxButton(this, wxID_CANCEL, _T("取消"));
	subsizer->Add(cancel, 0, wxALIGN_RIGHT | wxALL, 3);

	sizer->Add(subsizer, 0, wxALL | wxALIGN_RIGHT, 3);

	SetSizer(sizer);
	sizer->Fit(this);

	long flags = m_pList->GetWindowStyleFlag();
	flags |= wxLC_SINGLE_SEL;
	m_pList->SetWindowStyle(flags);
}
void VarListDlg::OnSelectItem(wxListEvent &event)
{
}
void	VarListDlg::OnListItemActivated(wxListEvent& event)
{
	Space::SNode* pNode = reinterpret_cast<Space::SNode*>(m_pList->GetItemData(event.GetIndex()));
	assert( pNode != NULL );
	Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );
	VarEditDlg dlg(this, pNode, pVarSet);
	dlg.EditShowModal();
	RefreshLayout( pNode );
}

void VarListDlg::OnChooseClient(wxCommandEvent &event)
{
}

void VarListDlg::OnContextMenu(wxContextMenuEvent& event)
{
	wxPoint point = event.GetPosition();
	point = ScreenToClient(point);

	wxMenu menu;
	long item = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if( item != -1 )
	{
		menu.Append(TGR_VARLIST_NEW,		_T("新变量"));
		menu.Append(TGR_VARLIST_EDIT,		_T("编辑变量"));
		menu.Append(TGR_VARLIST_DELETE,		_T("删除变量"));
	}
	else
	{
		menu.Append(TGR_VARLIST_NEW,		_T("新变量"));
		menu.Append(TGR_VARLIST_EDIT,		_T("编辑变量"));
		menu.Enable(TGR_VARLIST_EDIT,		false );
		menu.Append(TGR_VARLIST_DELETE,		_T("删除变量"));
		menu.Enable(TGR_VARLIST_DELETE,		false );
	}
	PopupMenu(&menu, point.x, point.y);
}

intptr_t VarListDlg::GetDocumentID()
{
	Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot(Space::SNT_VARSET, NULL);
	return pVarSet ? (intptr_t)pVarSet : (intptr_t)-1;
}

void	VarListDlg::OnProcessAny(wxCommandEvent& event)
{
	switch( event.GetId() )
	{
	case TGR_VARLIST_NEW:
		{
			Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );
			VarEditDlg dlg(this, NULL, pVarSet);
			Space::SNode* pNewNode = dlg.EditShowModal();
			if( pNewNode != NULL )
			{
				Space::SNode* pParent = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );

				GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd( pNewNode, pParent, pParent ? pParent->LastChild() : NULL ));
				RefreshLayout( pNewNode );
			}
		}
		break;
	case TGR_VARLIST_EDIT:
		{
			long item = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if ( item != -1 )
			{
				Space::SNode* pNode = reinterpret_cast<Space::SNode*>(m_pList->GetItemData(item));
				assert( pNode != NULL );
				Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot( Space::SNT_VARSET, NULL );
				VarEditDlg dlg(this, pNode, pVarSet);
				dlg.EditShowModal();
				RefreshLayout( pNode );
			}
		}
		break;
	case TGR_VARLIST_DELETE:
		{
			long item = m_pList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if ( item != -1 )
			{
				Space::SNode* pNode = reinterpret_cast<Space::SNode*>(m_pList->GetItemData(item));
				set<Space::SNode*>	exports = pNode->GetReferences();
				if( !exports.empty() )
				{
					wxString info = wxString::Format( _T("变量:%s，仍被使用中，删除它会造成触发器失效.\n\n确定删除？"), pNode->Name().c_str() );
					if( wxMessageBox( info, _T("警告"), wxOK | wxCANCEL ) != wxOK )
					{
						return;
					}
				}
				GCmdMgr->AddCommand(GetDocumentID(), new Space::DeleteNodeCmd(pNode));
			}
		}
		break;
	default:
		assert( false );
		break;
	}
}
void	VarListDlg::RefreshLayout( Space::SNode* pNode )
{
	if( pNode == NULL || pNode->FirstChild() == NULL )
		return;

	long index = m_pList->FindItem( -1, reinterpret_cast<wxUIntPtr>(pNode) );
	if( index < 0 || index >= m_pList->GetItemCount() )
		return;

	m_pList->SetItem( index, 0, pNode->Name().c_str() );

	bool bAry = pNode->IsArray();
	int iSize = reinterpret_cast<int>(pNode->UserData("size"));
	wxString typeName = pNode->TEETmp() ? pNode->TEETmp()->DisplayName().c_str() : _T("???");
	if( bAry )
	{
		typeName += wxString::Format( _T(" [%s:%d]"), bAry?_T("数组"):_T(""), iSize<=0?1:iSize );
	}

	m_pList->SetItem( index, 1, typeName );

	if( pNode->FirstChild() != NULL )
	{
		bool bValid = pNode->FirstChild()->ClassType() != Space::SNT_PARAMETER;
		wstring defaultVal = !bValid ? _T("- 没有 -"): pNode->FirstChild()->GetReadText();
		m_pList->SetItem( index, 2, defaultVal.c_str() );
	}

	m_pList->SetItemData( index, long(pNode) );
}

//////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(VarEditDlg, wxDialog)
EVT_TEXT		(TGR_VAR_EDIT_NAME,				VarEditDlg::OnVarNameText)
EVT_CHOICE		(TGR_VAR_EDIT_TYPE,				VarEditDlg::OnVarTypeChoice)
EVT_BUTTON		(wxID_ANY,						VarEditDlg::OnButton)
EVT_HYPERLINK	(TGR_VAR_EDIT_DEFAULT_VALUE,	VarEditDlg::OnHyperLinkEvent)
EVT_CHECKBOX	(TGR_VAR_EDIT_CHECK_ARY,		VarEditDlg::OnAryCheck)

END_EVENT_TABLE()

VarEditDlg::VarEditDlg(wxWindow *parent, Space::SNode* pOrgNode, Space::SNode* pVarRoot)
:wxDialog( parent, wxID_ANY, _T("变量"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
,m_pOrgNode(pOrgNode)
,m_pWorkNode(NULL)
,m_pTEETmp(NULL)
,m_pVarRoot(pVarRoot)
{
	InitControls();
	InitContents();

	if( m_pOrgNode == NULL )
	{
		int iSelect = ( m_pVarType->GetSelection() );
		if( iSelect != -1 )
		{
			TEE::NodeBase *pTEETmp = reinterpret_cast<TEE::NodeBase*>(m_pVarType->GetClientData( iSelect ) );

			m_pWorkNode = Space::GSpaceMgr->BuildDefault<Space::SNT_VARIABLE>( pTEETmp );
			assert( m_pWorkNode != NULL );
			if( m_pWorkNode != NULL )
			{
				m_pWorkNode->Name( m_pVarName->GetValue().wc_str() );
				bool bAry = m_pAryCheck->GetValue();
				m_pWorkNode->IsArray(bAry);
				int iSize = m_pArySize->GetValue();
				m_pWorkNode->UserData( "size", reinterpret_cast<void*>(iSize) );
			}
		}
	}
	else
	{
		m_pWorkNode = Space::SNode::Clone( pOrgNode );
	}
	RefreshLayout();

	//////////////////////////////////////////////////////////////////////////
}

VarEditDlg::~VarEditDlg()
{
}
void	VarEditDlg::InitContents()
{
	vector<TEE::NodeBase*> Options;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTION, Options);
	vector<TEE::NodeBase*>::iterator iter( Options.begin() );
	for( ; iter != Options.end(); ++iter )
	{
		TEE::NodeBase* pNode = (*iter);
		if( Space::GSpaceMgr->IsInternalType(pNode->CodeName()) )
			continue;
		m_pVarType->Append( pNode->DisplayName(), reinterpret_cast<void*>(pNode) );
	}
	if( m_pOrgNode != NULL )
	{
		m_pVarName->SetValue( m_pOrgNode->Name().c_str() );
		m_pVarType->SetStringSelection( m_pOrgNode->TEETmp()->DisplayName() );

		bool bAry = m_pOrgNode->IsArray();
		m_pAryCheck->SetValue( bAry );

		if( !bAry )
		{
			m_pArySize->Enable(false);
		}
		else
		{
			int iSize = reinterpret_cast<int>(m_pOrgNode->UserData("size"));
			m_pArySize->SetValue( iSize );
		}
	}
	else
	{
		m_pVarName->SetValue( _T("") );
		m_pVarType->Select(0);

		m_pArySize->Enable(false);
	}
}
void	VarEditDlg::InitControls()
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *subsizer;

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("变量名(&N):"), wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);
	m_pVarName = new GWTextCtrl( this, TGR_VAR_EDIT_NAME, _T(""), wxDefaultPosition, wxDefaultSize );
	subsizer->Add( m_pVarName, 1, wxALL, 3 );
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("变量类型(&T):"), wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);
	m_pVarType = new wxChoice( this, TGR_VAR_EDIT_TYPE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
	subsizer->Add( m_pVarType, 1, wxALL, 3 );
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add( 110, 0 );
	m_pAryCheck = new wxCheckBox( this, TGR_VAR_EDIT_CHECK_ARY, _T("数组(&A)"), wxDefaultPosition, wxDefaultSize);
	subsizer->Add( m_pAryCheck, 0, wxALL, 3 );
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("尺寸(&S):")), 0, wxALL, 5);
	m_pArySize = new wxSpinCtrl( this, wxID_ANY, _T("1"), wxDefaultPosition, wxDefaultSize);
	m_pArySize->SetMin(1);
	subsizer->Add( m_pArySize, 1, wxALL, 3 );
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("初始值:"), wxDefaultPosition, wxDefaultSize), 0, wxALL, 5);
	m_pDefaultValue = new GWHyperlinkCtrl( this, TGR_VAR_EDIT_DEFAULT_VALUE, _T("没有"), wxDefaultPosition, wxDefaultSize);
	m_pDefaultValue->SetNormalColour( *wxRED );
	subsizer->Add( m_pDefaultValue, 0, wxALL, 3 );
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxButton(this, TGR_VAR_EDIT_CLEAR_DEFAULT, _T("清除值(&C)")), 0, wxALL, 5);
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	m_pOk = new wxButton(this, wxID_OK, _T("确定"));
	m_pOk->SetDefault();
	subsizer->Add(m_pOk, 0, wxALIGN_RIGHT | wxALL, 3);
	wxButton *cancel = new wxButton(this, wxID_CANCEL, _T("取消"));
	subsizer->Add(cancel, 0, wxALIGN_RIGHT | wxALL, 3);
	sizer->Add(subsizer, 0, wxALL | wxALIGN_RIGHT, 3);

	//
	SetSizer(sizer);
	sizer->Fit(this);
}
Space::SNode*	VarEditDlg::EditShowModal()
{
	if( m_pWorkNode == NULL )
	{
		wxMessageDialog dialog( NULL, _T("没有可定义的变量类型"), _T("提示"), wxOK|wxICON_INFORMATION);
		dialog.ShowModal();
		return NULL;
	}

	m_pDefaultValue->SetClientData( static_cast<void*>(m_pWorkNode->FirstChild()) );
	if (ShowModal() == wxID_OK)
	{
		TEE::NodeBase *pTEETmp = reinterpret_cast<TEE::NodeBase *>(m_pVarType->GetClientData( m_pVarType->GetSelection() ) );
		m_pWorkNode->TEETmp( pTEETmp );
		m_pWorkNode->Name( m_pVarName->GetValue().wc_str() );
		bool bAry = m_pAryCheck->GetValue();
		m_pWorkNode->IsArray(bAry);
		int iSize = m_pArySize->GetValue();
		m_pWorkNode->UserData( "size", reinterpret_cast<void*>(iSize) );

		if( m_pOrgNode != NULL )
		{
			intptr_t docID = (intptr_t)-1;
			wxWindow* parentWindow = GetParent();
			if (EditTree* triggerTree = dynamic_cast<EditTree*>(parentWindow))
			{
				docID = triggerTree->GetDocumentID();
			}
			else if (VarListDlg* varListDlg = dynamic_cast<VarListDlg*>(parentWindow))
			{
				docID = varListDlg->GetDocumentID();
			}
			GCmdMgr->AddCommand(docID, new Space::ExChangeNodeContentCmd( m_pOrgNode, m_pWorkNode ));
			m_pWorkNode = NULL;
		}
		else
		{
			return m_pWorkNode;
		}
	}
	else
	{
		if( m_pWorkNode != NULL )
		{
			m_pWorkNode->SNode::Name(_T(""));
			SAFE_DELETE(m_pWorkNode);
		}

		return NULL;
	}
	return m_pOrgNode;
}
void	VarEditDlg::OnVarNameText( wxCommandEvent& event )
{
	wstring value = event.GetString();
	m_pOk->Enable( IsValidSymbolName(value.c_str()) );
}
void	VarEditDlg::OnVarTypeChoice( wxCommandEvent &event )
{
	TEE::NodeBase *pTEETmp = reinterpret_cast<TEE::NodeBase *>(m_pVarType->GetClientData( m_pVarType->GetSelection() ) );
	m_pWorkNode->TEETmp( pTEETmp );

	Space::SNode* pNewDefault = Space::GSpaceMgr->ReBuildVariableDefault( m_pWorkNode );
	m_pDefaultValue->SetClientData( reinterpret_cast<void*>(pNewDefault) );

	RefreshLayout();
}
void	VarEditDlg::OnButton( wxCommandEvent &event )
{
	if( event.GetId() == wxID_OK )
	{
		wstring nameValue = m_pVarName->GetValue().wc_str();
		if( m_pOrgNode == NULL || m_pOrgNode->Name() != nameValue )
		{
			if( m_pVarRoot && m_pVarRoot->FindFirstChild(Space::SNodeTypeAndNamePredicate(Space::SVerifyNodeType(Space::SNT_VARIABLE),Space::SVerifyNameType(nameValue))) )
			{
				wxMessageBox( wxString::Format( wxT("%s,与已有变量同名"), nameValue.c_str() ) );
				return;
			}
		}
		if( m_pOrgNode != NULL )
		{
			set<Space::SNode*>	exports = m_pOrgNode->GetReferences();
			if( !exports.empty() )
			{
				TEE::NodeBase *pTEETmp = reinterpret_cast<TEE::NodeBase *>(m_pVarType->GetClientData( m_pVarType->GetSelection() ) );
				if( m_pOrgNode != NULL && m_pOrgNode->TEETmp() != pTEETmp )
				{
					wxString info = wxString::Format( _T("改变变量类型，会造成重设初值，并且会造成使用它的触发器失效.\n\n确定改变？") );
					if( wxMessageBox( info, _T("警告"), wxOK | wxCANCEL ) != wxOK )
					{
						return;
					}
				}
			}
		}
	}
	else if( event.GetId() == TGR_VAR_EDIT_CLEAR_DEFAULT )
	{
		Space::SNode* pNewDefault = Space::GSpaceMgr->ReBuildVariableDefault( m_pWorkNode );
		m_pDefaultValue->SetClientData( reinterpret_cast<void*>(pNewDefault) );
		RefreshLayout();
		return;
	}

	this->EndModal( event.GetId() );
	event.Skip();
}
void	VarEditDlg::OnHyperLinkEvent( GWHyperlinkEvent& event )
{
	Space::SNode *pNode = static_cast<Space::SNode*>( event.GetClientData() );
	if( pNode == NULL )
		return;
	ParamDlg dlg( pNode, PDT_VARIABLE_DEFAULT );
	Space::SNode *pNewNode = dlg.PrefShowModal( );
	if( pNewNode != NULL )
	{
		m_pDefaultValue->SetClientData( reinterpret_cast<void*>(pNewNode) );
		RefreshLayout();
	}
}
void	VarEditDlg::OnAryCheck( wxCommandEvent& event )
{
	bool bAry = event.IsChecked();
	m_pArySize->Enable(bAry);
}
void	VarEditDlg::RefreshLayout()
{
	if( m_pWorkNode != NULL && m_pWorkNode->FirstChild() != NULL )
	{
		bool bValid = m_pWorkNode->FirstChild()->ClassType() != Space::SNT_PARAMETER;
		m_pDefaultValue->SetLabel( !bValid ? _T("没有"): m_pWorkNode->FirstChild()->GetReadText() );
		m_pDefaultValue->SetNormalColour( bValid ? *wxBLUE : *wxRED );
	}
}
