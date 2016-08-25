#include "PropertyDialogs.h"

#if wxUSE_SPINCTRL
#include "wx/spinctrl.h"
#endif
#include "GWIDs.h"
#include "TEEManager.h"
#include "NodeVerify.h"
#include "TagDialogs.h"
// ----------------------------------------------------------------------------
// SettingsDialog
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS(TEEPropertyDialogBase, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(TEEPropertyDialogBase, wxPropertySheetDialog)
EVT_BUTTON(wxID_ANY, TEEPropertyDialogBase::OnButton)
END_EVENT_TABLE()
TEEPropertyDialogBase::TEEPropertyDialogBase( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create( parent, wxID_ANY, _("TEE"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
	CreateButtons( wxOK | wxCANCEL );

	m_Title = title;
	m_pNode = pNode;
}

TEEPropertyDialogBase::~TEEPropertyDialogBase()
{
}

void TEEPropertyDialogBase::OnButton(wxCommandEvent& event)
{
	if ( event.GetId() == wxID_OK )
	{
		wxString errCode;
		if( !DoCheck( errCode ) )
		{
			wxMessageDialog dialog( NULL, errCode, _T("提示"), wxOK|wxICON_INFORMATION);
			dialog.ShowModal();
			return;
		}
	}
	else if( event.GetId() == CTRL_TEE_SET_TAG )
	{
		TagDialog dialog;
		dialog.Init( m_pTags->GetValue() );
		if( dialog.ShowModal() == CTRL_TEE_TAG_OK )
		{
			m_pTags->SetValue( dialog.GetTagResults() );
		}
	}
	event.Skip();
}

wxPanel*	TEEPropertyDialogBase::CreatePage( )
{
	wxBookCtrlBase* parent = GetBookCtrl();

	wxPanel* pPanel = new wxPanel( parent, wxID_ANY );

	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *pUserEdit = new wxBoxSizer( wxVERTICAL );

	DoPage( pTopSizer, pPanel );

	pTopSizer->Add( pUserEdit, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pPanel->SetSizer( pTopSizer );
	pTopSizer->Fit( pPanel );

	parent->AddPage( pPanel, m_Title, true, -1 );
	LayoutDialog();

	return pPanel;
}

void	TEEPropertyDialogBase::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	wxBoxSizer *pDisplayNameBox = new wxBoxSizer( wxHORIZONTAL );
	pDisplayNameBox->Add( new wxStaticText( pPanel, wxID_STATIC, _("名称:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;
	m_pDisplayName = new wxTextCtrl( pPanel, CTRL_TEE_DIALOG_DISPLAY_NAME, _(""), wxDefaultPosition, wxSize( 330, 20 ) ) ;
	pDisplayNameBox->Add( m_pDisplayName, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pDisplayNameBox, 0, wxGROW|wxALL, 0 );

	wxBoxSizer *pCodeNameBox = new wxBoxSizer( wxHORIZONTAL );
	pCodeNameBox->Add( new wxStaticText( pPanel, wxID_STATIC, _("编码:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;
	m_pCodeName = new wxTextCtrl( pPanel, CTRL_TEE_DIALOG_CODE_NAME, _(""), wxDefaultPosition, wxSize( 330, 20 ) ) ;
	pCodeNameBox->Add( m_pCodeName, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pCodeNameBox, 0, wxGROW|wxALL, 0 );

	wxBoxSizer *pTagBox = new wxBoxSizer( wxHORIZONTAL );
	pTagBox->Add( new wxStaticText( pPanel, wxID_STATIC, _("标签:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;
	m_pTags = new wxTextCtrl( pPanel, wxID_ANY, _(""), wxDefaultPosition, wxSize( 230, 20 ) ) ;
	m_pTags->SetEditable(false);
	pTagBox->Add( m_pTags, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	m_pAddTag = new wxButton( pPanel, CTRL_TEE_SET_TAG, _("设置"), wxDefaultPosition, wxDefaultSize ) ;
	pTagBox->Add( m_pAddTag, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pTagBox, 0, wxGROW|wxALL, 0 );

}
bool		TEEPropertyDialogBase::DoModalBase( wxString& displayName, wxString& codeName, wxString& tags )
{
	m_pDisplayName->SetValue( displayName );
	m_pCodeName->SetValue( codeName );
	m_pTags->SetValue( tags );

	if( this->ShowModal() != wxID_OK )
		return false;

	if ( GetDisplayName() == wxEmptyString )
		return false;
	if ( GetCodeName() == wxEmptyString )
		return false;

	displayName = GetDisplayName();
	codeName = GetCodeName();
	tags = GetTagsName();

	return true;
}
bool	TEEPropertyDialogBase::DoCheck( wxString& errCode )
{
	wxString displayName = GetDisplayName();
	wxString codeName = GetCodeName();

	if( displayName == wxEmptyString || codeName == wxEmptyString )
	{
		errCode = wxString::Format( wxT("Name or Code cannot be NULL"));
		return false;
	}

	vector<TEE::NodeBase*> nodes;
	TEE::GTEEMgr->FindNodesOfType(m_eNodeType, TEE::VerifyDisplayName(displayName.wc_str()), nodes);
	if( nodes.size() > 1 || nodes.size() == 1 && nodes.front() != m_pNode )
	{
		errCode = wxString::Format( wxT("Repeatiton Name of %s"), displayName );
		return false;
	}

	nodes.clear();
	TEE::GTEEMgr->FindNodesOfType(m_eNodeType, TEE::VerifyCodeName(codeName.wc_str()), nodes);
	if(nodes.size() > 1 || nodes.size() == 1 && nodes.front() != m_pNode)
	{
		errCode = wxString::Format( wxT("Repeatiton Code of %s"), codeName );
		return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Event
EventPropertyDialog::EventPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_EVENT;
	m_TagRoot = _T("EventTag");
}
void	EventPropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );
}

bool	EventPropertyDialog::DoModal( wxString& displayName, wxString& codeName, wxString& tags )
{
	return DoModalBase( displayName, codeName, tags );
}
bool	EventPropertyDialog::DoCheck( wxString& errCode )
{
	bool bValid = TEEPropertyDialogBase::DoCheck( errCode );
	return bValid;
}

//////////////////////////////////////////////////////////////////////////
//Condition
ConditionPropertyDialog::ConditionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_CONDITION;
	m_TagRoot = _T("ConditionTag");
}
void	ConditionPropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );
}
bool	ConditionPropertyDialog::DoModal( wxString& displayName, wxString& codeName, wxString& tags )
{
	return DoModalBase( displayName, codeName, tags );
}
bool	ConditionPropertyDialog::DoCheck( wxString& errCode )
{
	bool bValid = TEEPropertyDialogBase::DoCheck( errCode );
	return bValid;
}
//////////////////////////////////////////////////////////////////////////
//Function
FunctionPropertyDialog::FunctionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_FUNCTION;
	m_TagRoot = _T("FunctionTag");
}
void	FunctionPropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );

	wxBoxSizer *pReturnInfo = new wxBoxSizer( wxHORIZONTAL );

	pReturnInfo->Add( new wxStaticText( pPanel, wxID_STATIC, _("返回类型:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;

	m_pReturnTypeChoice = new wxChoice( pPanel, CTRL_FUNCTION_DIALOG_RETURN_TYPE, wxDefaultPosition, wxSize( 100, 20 ), 0, NULL, wxCB_SORT );
	{
		vector<TEE::NodeBase*> options;
		TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTION, options);
		vector<TEE::NodeBase*>::iterator iter(options.begin());
		for( ; iter != options.end(); ++iter )
		{
			TEE::NodeBase* pNode = (*iter);
			const wstring& DisplayName = pNode->DisplayName();
			m_pReturnTypeChoice->Append( DisplayName.c_str(), reinterpret_cast<void*>(pNode) );
		}
	}
	pReturnInfo->Add( m_pReturnTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pReturnInfo, 0, wxGROW|wxALL, 0 );

	wxBoxSizer *pReturenDetail = new wxBoxSizer( wxHORIZONTAL );
	pReturenDetail->Add( new wxStaticText( pPanel, wxID_STATIC, _("返回详细:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;
	m_pReturnDetailChoice = new wxChoice( pPanel, CTRL_FUNCTION_DIALOG_RETURN_DETAIL, wxDefaultPosition, wxSize( 100, 20 ) );
	{
		map<TEE::RETURN_DETAIL, wstring>	detailMaps = TEE::GetReturnDetailNames();
		map<TEE::RETURN_DETAIL, wstring>::iterator iter(detailMaps.begin());
		for( ; iter != detailMaps.end(); ++iter )
		{
			TEE::RETURN_DETAIL eType = (*iter).first;
			const wstring& valueName = (*iter).second;
			m_pReturnDetailChoice->Append( valueName.c_str(), reinterpret_cast<void*>(eType) );
		}
	}
	pReturenDetail->Add( m_pReturnDetailChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pReturenDetail, 0, wxGROW|wxALL, 0 );

	m_canBeActionCheck = new wxCheckBox(pPanel, wxID_ANY, TEXT("显示在动作列表中"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	pUserEdit->AddSpacer(5);
	pUserEdit->Add(m_canBeActionCheck, 0, wxLEFT, 0);
}
bool	FunctionPropertyDialog::DoModal( wxString& displayName, wxString& codeName, wxString& returnType, int& eReturnDetail, wxString& tags, bool& canBeAction)
{
	TEE::NodeBase* pReturnTypeOption = TEE::TEEManager::GetInstance()->FindOptionFromCode(returnType.wc_str());
	wxString returnTypeDisplayName = pReturnTypeOption ? pReturnTypeOption->DisplayName() : TEXT("");
	m_pReturnTypeChoice->SetStringSelection(returnTypeDisplayName);
	m_pReturnDetailChoice->Select( eReturnDetail );
	m_canBeActionCheck->SetValue(canBeAction);

	bool bSuccess = DoModalBase( displayName, codeName, tags );
	if( bSuccess )
	{
		TEE::NodeBase* pFunc = reinterpret_cast<TEE::NodeBase*>(m_pReturnTypeChoice->GetClientData(m_pReturnTypeChoice->GetSelection()));
		if (pFunc)
		{
			returnType = pFunc->CodeName();
		}
		eReturnDetail = (int)m_pReturnDetailChoice->GetClientData( m_pReturnDetailChoice->GetSelection() );
		canBeAction = m_canBeActionCheck->GetValue();
	}
	return bSuccess;
}
bool	FunctionPropertyDialog::DoCheck( wxString& errCode )
{
	bool bValid = TEEPropertyDialogBase::DoCheck( errCode );
	if( m_pReturnTypeChoice->GetSelection() == -1 )
	{
		errCode += ( wxT("\n Select Return Type") );
		bValid &= false;
	}
	return bValid;
}

//////////////////////////////////////////////////////////////////////////
//option
OptionPropertyDialog::OptionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_OPTION;
	m_TagRoot = _T("OptionTag");
}
void	OptionPropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );

	wxBoxSizer *pValueInfo = new wxBoxSizer( wxHORIZONTAL );
	pValueInfo->Add( new wxStaticText( pPanel, wxID_STATIC, _("值类型:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 ) ;

	m_pValueTypeChoice = new wxChoice( pPanel, CTRL_FUNCTION_DIALOG_RETURN_TYPE, wxDefaultPosition, wxSize( 100, 20 ) );
	map<TEE::VALUE_TYPE, pair<wstring,wstring> >	valueMaps = TEE::GetValueTypeNames();
	map<TEE::VALUE_TYPE, pair<wstring,wstring> >::iterator iter(valueMaps.begin());
	for( ; iter != valueMaps.end(); ++iter )
	{
		TEE::VALUE_TYPE eType = (*iter).first;
		const wstring& valueName = (*iter).second.first;
		m_pValueTypeChoice->Append( valueName.c_str(), reinterpret_cast<void*>(eType) );
	}

	pValueInfo->Add( m_pValueTypeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pUserEdit->Add( pValueInfo, 0, wxGROW|wxALL, 0 );

	m_pAddTag->Enable(false);
}

bool	OptionPropertyDialog::DoModal( wxString& displayName, wxString& codeName, int& eValueType, wxString& tags )
{
	m_pValueTypeChoice->SetStringSelection( TEE::GetValueTypeName( (TEE::VALUE_TYPE)eValueType ).first );
	bool bSuccess = DoModalBase( displayName, codeName, tags );

	if( bSuccess )
		eValueType = (int)m_pValueTypeChoice->GetClientData( m_pValueTypeChoice->GetSelection() );

	return bSuccess;
}

bool	OptionPropertyDialog::DoCheck( wxString& errCode )
{
	bool bValid = TEEPropertyDialogBase::DoCheck( errCode );
	return bValid;
}

//////////////////////////////////////////////////////////////////////////
//Root
RootPropertyDialog::RootPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_ROOT;
	m_TagRoot = _T("");
}
void	RootPropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );
	m_pAddTag->Enable(false);
	m_pCodeName->Enable(false);
	m_pTags->Enable(false);
}
bool	RootPropertyDialog::DoModal( wxString& displayName, wxString& codeName, wxString& tags )
{
	m_pDisplayName->SetValue( displayName );

	if( this->ShowModal() != wxID_OK )
		return false;

	if ( GetDisplayName() == wxEmptyString )
		return false;

	displayName = GetDisplayName();
	return true;
}
bool	RootPropertyDialog::DoCheck( wxString& errCode )
{
	//bool bValid = TEEPropertyDialogBase::DoCheck( errCode );

	wxString displayName = GetDisplayName();
	return !displayName.empty();
}

//////////////////////////////////////////////////////////////////////////
//Space
SpacePropertyDialog::SpacePropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode )
:TEEPropertyDialogBase( parent, title, pNode )
{
	CreatePage( );
	m_eNodeType = TEE::NT_SPACEROOT;
	m_TagRoot = _T("");
}
void	SpacePropertyDialog::DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel )
{
	TEEPropertyDialogBase::DoPage( pUserEdit, pPanel );
	m_pAddTag->Enable(false);
	//m_pCodeName->Enable(false);
	m_pTags->Enable(false);
}
bool	SpacePropertyDialog::DoModal( wxString& displayName, wxString& codeName )
{
	m_pDisplayName->SetValue( displayName );
	m_pCodeName->SetValue( codeName );

	if( this->ShowModal() != wxID_OK )
		return false;

	if ( GetDisplayName() == wxEmptyString )
		return false;
	if ( GetCodeName() == wxEmptyString )
		return false;

	displayName = GetDisplayName();
	codeName = GetCodeName();
	return true;
}
bool	SpacePropertyDialog::DoCheck( wxString& errCode )
{
	wxString displayName = GetDisplayName();
	return !displayName.empty();
}