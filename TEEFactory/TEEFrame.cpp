
#include "TEEFrame.h"
#include "GWTreeCtrl.h"
#include "GWSplitterWindows.h"
#include "TEEApp.h"

#include "GWIDs.h"
#include "TEEIDs.h"
#include "TEEManager.h"
#include "NodeFactory.h"
#include "Option.h"
#include "NodeVerify.h"

#include "NodeManager.h"
#include "PropertyDialogs.h"
#include "IncludeDialogs.h"

#if wxUSE_HELP
#include "wx/cshelp.h"
#endif

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(TEEFrame, wxFrame)
EVT_SIZE(TEEFrame::OnSize)
EVT_IDLE(TEEFrame::OnIdle)

EVT_MENU(MENU_TEE_NEW_EVENT, TEEFrame::OnNewEvent)
EVT_MENU(MENU_TEE_NEW_CONDITION, TEEFrame::OnNewCondition)
EVT_MENU(MENU_TEE_NEW_FUNCTION, TEEFrame::OnNewFunction)
EVT_MENU(MENU_TEE_NEW_OPTION, TEEFrame::OnNewOption)
EVT_MENU(MENU_TEE_NEW_SPACE, TEEFrame::OnNewSpace)
EVT_MENU(MENU_TEE_DELETE, TEEFrame::OnDelete)
EVT_MENU(MENU_TEE_PROPERTY, TEEFrame::OnProperty)
EVT_MENU(MENU_TEE_SAVE, TEEFrame::OnSave)
EVT_MENU(wxID_ANY, TEEFrame::OnProcessAny)

EVT_TREE_ITEM_RIGHT_CLICK(CTRL_ECA_TREE, TEEFrame::OnRightClickECATree)
EVT_TREE_ITEM_ACTIVATED(CTRL_ECA_TREE, TEEFrame::OnActivateECATree)
EVT_TREE_SEL_CHANGED(CTRL_ECA_TREE, TEEFrame::OnSelECATreeChanged)
EVT_TREE_SEL_CHANGING(CTRL_ECA_TREE, TEEFrame::OnSelECATreeChanging)
EVT_TREE_ITEM_MENU(CTRL_ECA_TREE, TEEFrame::OnItemMenu)
EVT_RICHTEXT_CONTENT_INSERTED(CTRL_ECA_EDIT, TEEFrame::OnEditNotesChanged)
EVT_RICHTEXT_CONTENT_DELETED(CTRL_ECA_EDIT, TEEFrame::OnEditNotesChanged)
EVT_TREE_BEGIN_DRAG(CTRL_ECA_TREE, TEEFrame::OnBeginDrag)
EVT_TREE_BEGIN_RDRAG(CTRL_ECA_TREE, TEEFrame::OnBeginRDrag)
EVT_TREE_END_DRAG(CTRL_ECA_TREE, TEEFrame::OnEndDrag)
EVT_CHOICE(CTRL_TEE_OPTION_LIST, TEEFrame::OnOptionListChanged)
EVT_BUTTON(CTRL_TEE_ADD_PARAM, TEEFrame::OnAddParam)
END_EVENT_TABLE()

extern std::wstring MakeValidPathName(const wchar_t* pPathName);

TEEFrame::TEEFrame(const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: GWFrame(nullptr, id, title, pos, size, style)
	, m_pECATree(NULL)
	, m_pECAEdit(NULL)
	, m_pECAView(NULL)
	, m_Root(NULL)
	, m_eDragType(0)
	, m_bChanged(false)
	, m_optionsChoice(nullptr)
	, m_adjustsChoice(nullptr)
	, m_addParamButton(nullptr)
{
	// set the frame icon
	SetIcon(wxICON(sample));

	// the "About" item should be in the help menu
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_About, _T("&About...\tF1"), _T("Show about dialog"));

	// create a menu bar
	wxMenu *fileMenu = new wxMenu;

	fileMenu->Append(wxID_NEW, _T("&New\tCtrl+N"), _T("New a file"));
	fileMenu->Append(wxID_OPEN, _T("&Open\tCtrl+O"), _T("Open a file"));
	fileMenu->Append(wxID_SAVE, _T("&Save\tCtrl+S"), _T("Save a file"));
	fileMenu->Append(wxID_SAVEAS, _T("&Save As...\tF12"), _T("Save to a new file"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_Quit, _T("E&xit\tAlt+X"), _T("Quit this program"));

	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, _T("&File"));
	//menuBar->Append(editMenu, _T("&Edit"));
	menuBar->Append(helpMenu, _T("&Help"));

	// ... and attach this menu bar to the frame
	SetMenuBar(menuBar);

	// create a status bar just for fun (by default with 1 pane only)
	// but don't create it on limited screen space (WinCE)
	bool is_pda = wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA;

#if wxUSE_STATUSBAR
	if (!is_pda)
	{
		CreateStatusBar(2);
		SetStatusText(_T("Welcome to wxRichTextCtrl!"));
	}
#endif

	wxToolBar* toolBar = CreateToolBar();
	toolBar->AddTool(wxID_NEW, TEXT("New"), wxBitmap(new_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("New"));
	toolBar->AddTool(wxID_OPEN, TEXT("Open"), wxBitmap(open_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Open"));
	toolBar->AddTool(wxID_SAVE, TEXT("Save"), wxBitmap(save_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Save"));
	toolBar->AddSeparator();
	toolBar->AddStretchableSpace();
	m_optionsChoice = new wxChoice(toolBar, CTRL_TEE_OPTION_LIST);
	toolBar->AddControl(m_optionsChoice, TEXT("Options"));
	m_adjustsChoice = new wxChoice(toolBar, CTRL_TEE_ADJUST_LIST);
	toolBar->AddControl(m_adjustsChoice, TEXT("Adjusts"));
	m_addParamButton = new wxButton(toolBar, CTRL_TEE_ADD_PARAM, TEXT("添加参数"));
	toolBar->AddControl(m_addParamButton);
	toolBar->Realize();

	m_FileSuffix = _T("TEE");
	InitWindow();

	//根据配置 载入文件:
	bool bSuccess = true;
	TCHAR strTriggerFilePath[256];
	TCHAR nBuffer = GetPrivateProfileString(_T("TriggersPath"), _T("path"), _T(""), strTriggerFilePath, sizeof(strTriggerFilePath) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	TCHAR strTEELibPath[256];
	nBuffer = GetPrivateProfileString(_T("TEELibPath"), _T("path"), _T(""), strTEELibPath, sizeof(strTEELibPath) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	if (!bSuccess)
	{
		wxMessageDialog dialog(NULL, _T("请先检查配置文件:Profile.ini"), _T("错误"), wxOK | wxICON_ERROR);
		dialog.ShowModal();
		PostQuitMessage(0);
	}
	m_DefaultPath = MakeValidPathName(strTEELibPath).c_str();

	bool bLoaded = false;
	auto app = &wxGetApp();
	for (int i = 0; i < app->argc; ++i)
	{
		wxString param = FANSIToTCHAR(app->argv[i]);
		wxString fileName;
		if (param.StartsWith(TEXT("-"), &fileName))
		{
			wstring fullPath = m_DefaultPath + fileName.wc_str();
			bLoaded = LoadSpaces(fullPath.c_str(), fileName.wc_str(), true);
			break;
		}
	}
	if (!bLoaded)
	{
		NewTEELib();
	}
}

TEEFrame::~TEEFrame()
{
	TEE::GTEEMgr->Flush();
}

void TEEFrame::Resize()
{
	wxSize size = GetClientSize();
	if( m_pECAEdit != NULL && m_pECAView != NULL && m_pECATree != NULL )
	{
		wxSize spaceSize = m_pECATree->GetSize();
		m_pSplitterH->SetSize( size.x-spaceSize.x, size.y );
	}
}

bool TEEFrame::InitWindow()
{
	m_pSplitterV = new GWSplitterWindow( this, this );
	m_pSplitterV->SetWindowStyle( wxSP_3DSASH );
	m_pSplitterV->SetMinimumPaneSize(1);

	m_pECATree = new GWTreeCtrl( m_pSplitterV, CTRL_ECA_TREE, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER | wxTR_EDIT_LABELS );
	m_pSplitterH = new GWSplitterWindow( m_pSplitterV, this );
	m_pSplitterH->SetWindowStyle( wxSP_3DSASH );
	m_pSplitterH->SetMinimumPaneSize(1);
	m_pSplitterV->SplitVertically( m_pECATree, m_pSplitterH, 200 );
	m_pSplitterV->UpdateSize();

	m_pECAEdit = new wxRichTextCtrl(m_pSplitterH, CTRL_ECA_EDIT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER|wxWANTS_CHARS);
	m_pECAView = new wxRichTextCtrl(m_pSplitterH, CTRL_ECA_VIEW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER|wxWANTS_CHARS);
	m_pECAView->SetEditable( false );
	m_pSplitterH->SplitHorizontally(m_pECAEdit, m_pECAView, 300);
	m_pSplitterH->UpdateSize();

	m_pECAEdit->Clear();
	m_pECAView->Clear();
	return true;
}
bool	TEEFrame::NewTEELib()
{
	m_CurrentPath = _T("New File");
	m_CurrentName = wxEmptyString;
	m_pECAEdit->Clear();
	m_pECAView->Clear();
	SetChanged( false );
	this->SetTitle( m_CurrentPath );
	TEE::NodeBase* pRootNode = TEE::GTEEMgr->NewSpace();
	pRootNode->m_DisplayName = _T("新建TEE库");
	pRootNode->m_CodeName = _T("RootNode");

	m_pECATree->DeleteAllItems();

	GWTreeItemData * pData = new GWTreeItemData( pRootNode->m_CodeName, pRootNode );
	m_ECARoot = m_pECATree->AddRoot( pRootNode->m_DisplayName, 0, 0, pData );

	const vector<pair<TEE::NODE_TYPE,wstring> >& RootNodes = GetRootNodes();
	for(auto cIter = RootNodes.begin(); cIter != RootNodes.end(); ++cIter )
	{
		TEE::NODE_TYPE eType = cIter->first;
		const wstring& strDisplayName = cIter->second;
		TEE::NodeBase* pNewNode = TEE::GNodeFactory.CreateNode( eType );
		pNewNode->m_DisplayName = strDisplayName;
		pRootNode->AddChild(pNewNode);

		m_ECARoots[eType] = m_pECATree->AppendItem( m_ECARoot, strDisplayName.c_str(), 1, 1, new GWTreeItemData( wxEmptyString, pNewNode) );
	}

	return true;
}

TEE::NodeBase*	TEEFrame::GetTreeItemNode( wxTreeItemId itemId )
{
	GWTreeItemData *pData = itemId.IsOk() ? static_cast<GWTreeItemData *>(m_pECATree->GetItemData(itemId)) : NULL;
	if( pData != NULL )
		return pData->m_pNode;

	return NULL;
}
bool	TEEFrame::RefreshSelectNode( wxTreeItemId itemId )
{
	TEE::NodeBase* pSelNode = GetTreeItemNode( itemId );

	if ( pSelNode == NULL  )
		return false;
	 
	m_pECAEdit->Clear();
	m_pECAView->Clear();

	if( pSelNode->Editable() )
	{
		m_pECAEdit->SetFocus();
		m_pECAEdit->SetEditable( false );

		{
			RefreshNodeEdit( itemId );
			m_pECAEdit->SetEditable( true );
		}
	}
	else
	{
		m_pECAEdit->SetEditable( false );
	}	

	if( pSelNode->Viewable() )
	{
		RefreshNodeView( itemId );
	}
	return true;
}
bool	TEEFrame::RefreshNodeEdit( wxTreeItemId itemId )
{
	TEE::NodeBase* pSelNode = GetTreeItemNode( itemId );
	if( pSelNode == NULL )
		return false;

	list<TEE::RichText> records;
	pSelNode->ToRichString( records );

	wxRichTextAttr urlStyle;
	for( list<TEE::RichText>::iterator iter = records.begin(); iter != records.end(); ++iter )
	{
		const TEE::RichText& record = (*iter);
		wxColour color = *wxBLACK;
		switch(record.eColor)
		{
		case TEE::RC_BLACK:
			color = *wxBLACK;
			break;
		case TEE::RC_BLUE:
			color = *wxBLUE;
			break;
		case TEE::RC_RED:
			color = *wxRED;
			break;
		case TEE::RC_GREY:
			color = *wxLIGHT_GREY;
			break;
		case TEE::RC_GREEN:
			color = *wxGREEN;
			break;
		default:
			assert( false );
			break;
		}
		urlStyle.SetTextColour(color);
		urlStyle.SetFontUnderlined(record.bUnderLine);

		m_pECAEdit->BeginStyle(urlStyle);
		m_pECAEdit->WriteText( record.content );
		m_pECAEdit->EndStyle();
	}
	m_pECAEdit->SetModified(false);
	return true;
}
bool	TEEFrame::RefreshNodeView( wxTreeItemId itemId )
{
	TEE::NodeBase* pSelNode = GetTreeItemNode( itemId );
	if( pSelNode == NULL )
		return false;

	m_pECAView->Clear();

	list<TEE::RichText> records;
	pSelNode->GetNotesViewString( records );

	for( list<TEE::RichText>::iterator iter = records.begin(); iter != records.end(); ++iter )
	{
		const TEE::RichText& record = (*iter);
		if( record.content.empty() )
			continue;
		wxColour color = *wxBLACK;
		wxRichTextAttr urlStyle;
		switch(record.eColor)
		{
		case TEE::RC_BLACK:
			color = *wxBLACK;
			break;
		case TEE::RC_BLUE:
			color = *wxBLUE;
			break;
		case TEE::RC_RED:
			color = *wxRED;
			break;
		case TEE::RC_GREY:
			color = *wxLIGHT_GREY;
			break;
		case TEE::RC_GREEN:
			color = *wxCYAN;
			break;
		default:
			assert( false );
			break;
		}
		urlStyle.SetTextColour(color);
		urlStyle.SetFontUnderlined(record.bUnderLine);

		m_pECAView->BeginStyle(urlStyle);
		m_pECAView->WriteText( record.content );
		m_pECAView->EndStyle();
	}
	m_pECAView->Newline();
	m_pECAView->Newline();
	list<TEE::RichText> codes;
	pSelNode->GetNotesCodeString( codes );
	for( list<TEE::RichText>::iterator iter = codes.begin(); iter != codes.end(); ++iter )
	{
		const TEE::RichText& code = (*iter);
		wxColour color = *wxBLACK;
		wxRichTextAttr urlStyle;
		switch(code.eColor)
		{
		case TEE::RC_BLACK:
			color = *wxBLACK;
			break;
		case TEE::RC_BLUE:
			color = *wxBLUE;
			break;
		case TEE::RC_RED:
			color = *wxRED;
			break;
		case TEE::RC_GREY:
			color = *wxLIGHT_GREY;
			break;
		case TEE::RC_GREEN:
			color = *wxCYAN;
			break;
		default:
			assert( false );
			break;
		}
		urlStyle.SetTextColour(color);
		urlStyle.SetFontUnderlined(code.bUnderLine);

		m_pECAView->BeginStyle(urlStyle);
		m_pECAView->WriteText( code.content );
		m_pECAView->EndStyle();
	}
	m_pECAView->SetModified(false);
	return true;
}
bool	TEEFrame::RefreshTEEValidate( wxTreeItemId itemId )
{
	TEE::NodeBase* pNode = GetTreeItemNode( itemId );
	if( pNode == NULL )
		return false;

	TEE::ErrorMask flag = pNode->GetErrorMask();
	if( (flag != TEE::ERR_OK) )
		m_pECATree->SetItemBackgroundColour( itemId, *wxRED );
	else
		m_pECATree->SetItemBackgroundColour( itemId, *wxWHITE );
	{
		wxTreeItemIdValue cookie;
		wxTreeItemId childItemId = m_pECATree->GetFirstChild( itemId, cookie );
		RefreshTEEValidate( childItemId );
		wxTreeItemId SiblingItemId = m_pECATree->GetNextSibling( itemId );
		RefreshTEEValidate( SiblingItemId );
	}
	return true;
}
bool TEEFrame::RefreshSubRoot( wxTreeItemId subRootItemId, bool bExpand )
{
	TEE::NodeBase* pNode = GetTreeItemNode( subRootItemId );
	bool isSubRoot = pNode && pNode->Parent() && pNode->Parent()->IsA(TEE::NT_ROOT);
	if(!isSubRoot) return false;
	if (pNode->IsA(TEE::NT_TAGROOT)) return true;

	m_pECATree->DeleteChildren( subRootItemId );

	TEE::NodeBase* pChild = pNode->FirstChild();
	while ( pChild )
	{
		m_pECATree->AppendItem( subRootItemId, pChild->DisplayName(true), 0, 0, new GWTreeItemData(pChild->m_CodeName.c_str(), pChild) );
		pChild = pChild->Next();
	}
	if (bExpand) m_pECATree->Expand( subRootItemId );
	m_pECATree->Update();
	return true;
}

void TEEFrame::OnProcessAny(wxCommandEvent& event)
{
	if (event.GetId() == wxID_SAVE)
	{
		TEE::NodeBase *pNode = GetTreeItemNode( m_pECATree->GetRootItem() );
		if( pNode != NULL )
		{
			TEE::ErrorMask flag = pNode->GetErrorMask();
			if( flag != TEE::ERR_OK )
			{
				wxMessageDialog dialog( NULL, _T("库文件中包含错误格式，请根据红色提示修改"), _T("提示"), wxOK|wxICON_INFORMATION);

				dialog.ShowModal();

				return;
			}

			if( m_CurrentName == wxEmptyString )
			{
				wxString path=m_DefaultPath;
				wxString filename;

				wxFileDialog dialog( this, wxString::Format(_T("保存%s库文件"), m_FileSuffix), path, filename, wxString::Format(_T("TEE files (*.%s)|*.%s"), m_FileSuffix, m_FileSuffix), wxFD_SAVE );

				if (dialog.ShowModal() == wxID_OK)
				{
					wxString path = dialog.GetPath();

					if (!path.empty())
					{
						TEE::NodeBase *pNode = GetTreeItemNode( m_pECATree->GetRootItem() );
						if( pNode != NULL )
						{
							if( TEE::TEEManager::GetInstance()->SaveNode( path.wc_str(), pNode ) )
							{
								m_CurrentPath = path;
								m_CurrentName = dialog.GetFilename();
								this->SetTitle( m_CurrentName );
							}
						}
					}
					SetChanged( false );
				}
			}
			else
			{
				TEE::TEEManager::GetInstance()->SaveNode( m_CurrentPath.wc_str(), pNode );
				SetChanged( false );
			}
		}
	}
	else if (event.GetId() == wxID_OPEN)
	{
		wxString path=m_DefaultPath;
		wxString filename;
		wxArrayInt fileTypes;

		wxFileDialog dialog( this, wxString::Format(_("载入%s库文件"), m_FileSuffix), path, filename, wxString::Format(_T("TEE files (*.%s)|*.%s"), m_FileSuffix, m_FileSuffix), wxFD_OPEN );

		if (dialog.ShowModal() == wxID_OK)
		{
			wxString path = dialog.GetPath();
			wxString fileName = dialog.GetFilename();

			if (!path.empty())
			{
				bool bSuccess = LoadSpaces( path.wc_str(), fileName.wc_str(), true );
				assert(bSuccess);
			}
		}
	}
	else if (event.GetId() == wxID_SAVEAS)
	{
		wxString path;
		wxString filename;

		wxFileDialog dialog( this, wxString::Format(_("另存为%s库文件"), m_FileSuffix), path, filename, wxString::Format(_T("TEE files (*.%s)|*.%s"), m_FileSuffix, m_FileSuffix), wxFD_SAVE );

		if (dialog.ShowModal() == wxID_OK)
		{
			wxString path = dialog.GetPath();

			if (!path.empty())
			{
				TEE::NodeBase *pNode = GetTreeItemNode( m_pECATree->GetRootItem() );
				if( pNode != NULL )
				{
					TEE::ErrorMask flag = pNode->GetErrorMask();
					if( flag != TEE::ERR_OK )
					{
						wxMessageDialog dialog( NULL, _T("库文件中包含错误格式，请根据红色提示修改"), _T("提示"), wxOK|wxICON_INFORMATION);

						dialog.ShowModal();

						return;
					}

					if( TEE::TEEManager::GetInstance()->SaveNode( path.wc_str(), pNode ) )
					{
						m_CurrentPath = path;
						m_CurrentName = dialog.GetFilename();
						this->SetTitle( m_CurrentPath );
					}
					SetChanged( false );
				}
			}
		}
	}
	else if( event.GetId() == wxID_NEW )
	{
		NewTEELib();
	}
}
void	TEEFrame::OnSize(wxSizeEvent& event)
{
	Resize();
	event.Skip();
}
void	TEEFrame::OnIdle(wxIdleEvent& event)
{
	event.Skip();
}

void	TEEFrame::OnNewEvent(wxCommandEvent& event)
{
	DoNewNode( m_ECARoots[TEE::NT_EVENTROOT], TEE::NT_EVENT );
}
void	TEEFrame::OnNewCondition(wxCommandEvent& event)
{
	DoNewNode( m_ECARoots[TEE::NT_CONDITIONROOT], TEE::NT_CONDITION );
}
void	TEEFrame::OnNewFunction(wxCommandEvent& event)
{
	DoNewNode( m_ECARoots[TEE::NT_FUNCTIONROOT], TEE::NT_FUNCTION );
}
void	TEEFrame::OnNewOption(wxCommandEvent& event)
{
	DoNewNode( m_ECARoots[TEE::NT_OPTIONROOT], TEE::NT_OPTION );
}
void	TEEFrame::OnNewSpace(wxCommandEvent& event)
{
	DoNewNode( m_ECARoot, TEE::NT_SPACEROOT );
}

void	TEEFrame::OnDelete(wxCommandEvent& event)
{
	wxTreeItemId deleteItemId = m_pECATree->GetSelection();
	TEE::NodeBase* pNode = GetTreeItemNode(deleteItemId);
	m_pECATree->Delete( deleteItemId );

	if (pNode)
		pNode->Remove(false);

	TEE::NodeBase* pIncludeRoot = TEE::GTEEMgr->TEERoot()->FindFirstChild(TEE::VerifyNodeType(TEE::NT_INCLUDEROOT));
	TEE::GTEEMgr->RefreshIncludes( pIncludeRoot );

	TEE::NodeBase* pRoot = GetTreeItemNode(m_pECATree->GetRootItem());
	TEE::NodeBase::CheckErrorTree(pRoot);
	RefreshTEEValidate( m_pECATree->GetRootItem() );

	SetChanged( true );
}
bool	TEEFrame::DoProperty( wxTreeItemId itemId )
{
	if( !itemId.IsOk() )
	{
		assert( false );
		return false;
	}
	TEE::NodeBase* pSelNode = GetTreeItemNode( itemId );
	if( pSelNode != NULL )
	{
		if( GNodeMgr->OnNodeProperty( NULL, pSelNode->m_classType, pSelNode ) )
		{
			m_pECATree->SetItemText( itemId, pSelNode->DisplayName( true ) );
			RefreshSelectNode( itemId );

			if( pSelNode->ClassType() == TEE::NT_INCLUDEROOT )
			{
				RefreshSubRoot( itemId, true );
			}

			TEE::NodeBase* pIncludeRoot = TEE::GTEEMgr->TEERoot()->FindFirstChild(TEE::VerifyNodeType(TEE::NT_INCLUDEROOT));
			TEE::GTEEMgr->RefreshIncludes( pIncludeRoot );

			TEE::NodeBase* pRoot = GetTreeItemNode( m_pECATree->GetRootItem() );
			TEE::NodeBase::CheckErrorTree(pRoot);
			RefreshTEEValidate( m_pECATree->GetRootItem() );
			SetChanged( true );
		}
	}
	return true;
}

void	TEEFrame::OnProperty(wxCommandEvent& event)
{
	DoProperty( m_pECATree->GetSelection() );
	return;
}
void	TEEFrame::OnInclude(wxCommandEvent& event)
{
	TEE::NodeBase* pSelNode = GetTreeItemNode( m_pECATree->GetSelection() );
	if( pSelNode == NULL )
		return;

	set<wstring> includes =	TEE::GTEEMgr->FindIncludes( pSelNode );
	IncludeDialog dialog( includes );
	if( dialog.ShowModal() == CTRL_TEE_INCLUDE_OK )
	{
		TEE::GNodeFactory.BuildIncludesChild( dialog.GetIncludes(), pSelNode );

		RefreshSubRoot( m_pECATree->GetSelection(), true );

		TEE::GTEEMgr->RefreshIncludes( NULL );
		SetChanged(true);
	}
}

void	TEEFrame::OnSave(wxCommandEvent& event)
{
	event.Skip();
}
bool	TEEFrame::DoNewNode( wxTreeItemId parentId, TEE::NODE_TYPE eType )
{
	TEE::NodeBase* pRoot = GetTreeItemNode( parentId );
	if( pRoot == NULL )
		return false;

	TEE::NodeBase* pNewNode = GNodeMgr->OnNodeProperty( pRoot, eType, NULL );
	if( pNewNode != NULL )
	{
		wxTreeItemId newItemId = m_pECATree->AppendItem( parentId, pNewNode->DisplayName( true ), 0, 0, new GWTreeItemData( pNewNode->m_CodeName, pNewNode ));
		m_pECATree->Expand( parentId );
		m_pECATree->SelectItem( newItemId );

		TEE::NodeBase* pRoot = GetTreeItemNode(m_pECATree->GetRootItem());
		TEE::NodeBase::CheckErrorTree(pRoot);
		RefreshTEEValidate( m_pECATree->GetRootItem() );
		SetChanged( true );
	}

	return true;
}

void TEEFrame::OnSelECATreeChanged(wxTreeEvent& event)
{
	wxTreeItemId selItemId = event.GetItem();

	RefreshSelectNode( selItemId );
	event.Skip();   
}
void	TEEFrame::OnSelECATreeChanging(wxTreeEvent& event)
{
	event.Skip();
}


void	TEEFrame::OnEditNotesChanged(wxRichTextEvent& event)
{
	//解析
	if( !m_pECAEdit->IsEditable() )
		return;

	wxTreeItemId selItemId = m_pECATree->GetSelection();

	TEE::NodeBase* pNode = GetTreeItemNode( selItemId );
	if( pNode != NULL )
	{
		pNode->FromString( m_pECAEdit->GetValue().wc_str() );
		RefreshTEEValidate( m_pECATree->GetRootItem() );
		if(1)
		{
			RefreshNodeView( selItemId );
		}
	}

	SetChanged( true );
	return;

}
void	TEEFrame::OnRightClickECATree(wxTreeEvent& event)
{
	event.Skip(false);
}
void	TEEFrame::OnActivateECATree(wxTreeEvent& event)
{
	DoProperty( event.GetItem() );

	return;
}
void	TEEFrame::SetChanged( bool bChanged )
{
	if( bChanged == m_bChanged )
		return;

	m_bChanged = bChanged;

	SetTitle( wxString::Format( _T("%s%s"), m_CurrentPath, bChanged ? _T("*") : _T("") ) );
}

void	TEEFrame::OnItemMenu(wxTreeEvent& event)
{
	wxTreeItemId eventItemId = event.GetItem();

	TEE::NodeBase* pNode = GetTreeItemNode(eventItemId);
	if (pNode == NULL)
		return;

	wxMenu menu;
	menu.Append(MENU_TEE_NEW_EVENT, wxT("新事件(&E)"));
	menu.Enable(MENU_TEE_NEW_EVENT, pNode->NewEventable());
	menu.Append(MENU_TEE_NEW_CONDITION, wxT("新条件(&C)"));
	menu.Enable(MENU_TEE_NEW_CONDITION, pNode->NewConditionable());
	menu.Append(MENU_TEE_NEW_FUNCTION, wxT("新功能(&F)"));
	menu.Enable(MENU_TEE_NEW_FUNCTION, pNode->NewFunctionable());
	menu.Append(MENU_TEE_NEW_OPTION, wxT("新选项(&O)"));
	menu.Enable(MENU_TEE_NEW_OPTION, pNode->NewOptionable());
	menu.Append(MENU_TEE_NEW_SPACE, wxT("新空间(&S)"));
	menu.Enable(MENU_TEE_NEW_SPACE, pNode->NewSpaceable());
	menu.AppendSeparator();
	menu.Append(MENU_TEE_DELETE, wxT("删除(&D)"));
	menu.Enable(MENU_TEE_DELETE, pNode->Deleteable());
	menu.Append(MENU_TEE_PROPERTY, wxT("属性(&P)"));
	menu.Enable(MENU_TEE_PROPERTY, pNode->Propertyable());

	wxPoint pt = event.GetPoint();
	pt.y += this->GetToolBar()->GetSize().y;
	pt += wxPoint(5, 5);
	PopupMenu(&menu, pt);
	event.Skip();
}
const vector<pair<TEE::NODE_TYPE, wstring> >& TEEFrame::GetRootNodes()
{
	return TEE::GetTEERootNodes();
}
void TEEFrame::OnBeginDrag(wxTreeEvent& event)
{
	TEE::NodeBase* pNode = GetTreeItemNode(event.GetItem());
	if (pNode == NULL || !pNode->Dragable())
		return;
	m_DraggedItem = event.GetItem();
	m_eDragType = 1;
	event.Allow();
}
void TEEFrame::OnBeginRDrag(wxTreeEvent& event)
{
	TEE::NodeBase* pNode = GetTreeItemNode(event.GetItem());
	if (pNode == NULL || !pNode->Dragable())
		return;
	m_DraggedItem = event.GetItem();
	m_eDragType = 2;
	event.Allow();
}

void TEEFrame::OnEndDrag(wxTreeEvent& event)
{
	wxTreeItemId DstItem = event.GetItem();
	TEE::NodeBase* pSrcNode = GetTreeItemNode(m_DraggedItem);
	TEE::NodeBase* pDstNode = GetTreeItemNode(DstItem);
	if (pSrcNode == pDstNode)
		return;
	if (pSrcNode == NULL || pDstNode == NULL)
		return;
	if (pSrcNode->Parent() != pDstNode->Parent())
		return;

	switch (m_eDragType)
	{
	case 1:	//左键拖动，插入
	{
		pSrcNode->Unlink();
		m_pECATree->Delete(m_DraggedItem);
		if (pDstNode->Parent())
			pDstNode->Parent()->InsertOrAppend(pSrcNode, pDstNode);
		m_pECATree->InsertItem(m_pECATree->GetItemParent(DstItem), DstItem, pSrcNode->DisplayName(true), 0, 0, new GWTreeItemData(wxEmptyString, pSrcNode));
	}
	break;
	case 2:	//右键拖动，交换
	{
		TEE::NodeBase::Swap(pSrcNode, pDstNode);
		GWTreeItemData *pSrcData = m_DraggedItem.IsOk() ? static_cast<GWTreeItemData *>(m_pECATree->GetItemData(m_DraggedItem)) : NULL;
		GWTreeItemData *pDstData = DstItem.IsOk() ? static_cast<GWTreeItemData *>(m_pECATree->GetItemData(DstItem)) : NULL;
		m_pECATree->SetItemData(m_DraggedItem, pDstData);
		m_pECATree->SetItemData(DstItem, pSrcData);
		m_pECATree->SetItemText(m_DraggedItem, pDstNode->DisplayName(true));
		m_pECATree->SetItemText(DstItem, pSrcNode->DisplayName(true));
	}
	break;
	default:
		assert(false);
		break;
	}

	m_eDragType = 0;
	m_DraggedItem = (wxTreeItemId)0l;
	SetChanged(true);
}

bool	TEEFrame::LoadSpaces(const wstring& filePath, const wstring& fileName, bool bHost)
{
	SetStatusText(filePath);

	TEE::GTEEMgr->Flush();
	m_Root = TEE::GTEEMgr->LoadTEENode(fileName, bHost);
	if (m_Root == NULL)
		return false;

	m_pECATree->DeleteAllItems();

	GWTreeItemData * pData = new GWTreeItemData(m_Root->m_CodeName.c_str(), m_Root);
	m_ECARoot = m_pECATree->AddRoot(m_Root->m_DisplayName.c_str(), 0, 0, pData);

	for (TEE::NodeBase* pChild = m_Root->FirstChild(); pChild; pChild = pChild->Next())
	{
		wxTreeItemId subRootItemId = m_pECATree->AppendItem(m_ECARoot, pChild->m_DisplayName.c_str(), 1, 1, new GWTreeItemData(pChild->m_CodeName.c_str(), pChild));
		m_ECARoots[pChild->ClassType()] = subRootItemId;

		RefreshSubRoot(subRootItemId, false);
	}
	m_pECATree->Expand(m_ECARoot);
	m_pECATree->Update();

	m_CurrentPath = filePath.c_str();
	m_CurrentName = fileName.c_str();

	SetChanged(false);
	SetTitle(m_CurrentPath);
	m_pECAEdit->Clear();
	m_pECAView->Clear();
	TEE::NodeBase::CheckErrorTree(m_Root);
	RefreshTEEValidate(m_pECATree->GetRootItem());

	InitOptionList();
	return true;
}

void TEEFrame::InitOptionList()
{
	m_optionsChoice->Clear();
	m_adjustsChoice->Clear();

	vector<TEE::NodeBase*> Options;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTION, Options);
	vector<TEE::NodeBase*>::iterator iter(Options.begin());
	for (; iter != Options.end(); ++iter)
	{
		TEE::NodeBase* pNode = (*iter);
		m_optionsChoice->Append(pNode->DisplayName(), reinterpret_cast<void*>(pNode));
	}
}

void TEEFrame::OnOptionListChanged(wxCommandEvent &event)
{
	int selection = m_optionsChoice->GetSelection();
	if (selection == -1) return;
	TEE::NodeBase* pOption = reinterpret_cast<TEE::NodeBase*>(m_optionsChoice->GetClientData(selection));
	if (pOption == nullptr) return;

	m_adjustsChoice->Clear();
	m_adjustsChoice->Append(TEXT(""), (void*)(nullptr));

	vector<TEE::NodeBase*> adjusts;
	pOption->FindChildren(TEE::VerifyAlwaysOk(), adjusts);
	for (auto iter = adjusts.begin(); iter != adjusts.end(); ++iter)
	{
		TEE::NodeBase* pNode = (*iter);
		assert(pNode->ClassType() == TEE::NT_ADJUST);
		const wstring& KeyName = pNode->DisplayName();
		m_adjustsChoice->Append(KeyName, reinterpret_cast<void*>(pNode));
	}
}

void TEEFrame::OnAddParam(wxCommandEvent &event)
{
	int optionSelection = m_optionsChoice->GetSelection();
	if (optionSelection == -1) return;
	TEE::NodeBase* pOption = reinterpret_cast<TEE::NodeBase*>(m_optionsChoice->GetClientData(optionSelection));
	if (pOption == nullptr) return;
	int adjustSelection = m_adjustsChoice->GetSelection();
	TEE::NodeBase* pAdjust = adjustSelection == -1 ? nullptr : reinterpret_cast<TEE::NodeBase*>(m_adjustsChoice->GetClientData(adjustSelection));

	wxString paramName = pOption->CodeName().c_str();
	paramName.MakeLower();
	wxString strParam;
	if (pAdjust)
		strParam.Printf(TEXT("<%s %s = %s>"), pOption->CodeName().c_str(), paramName.wc_str(), pAdjust->DisplayName().c_str());
	else
		strParam.Printf(TEXT("<%s %s>"), pOption->CodeName().c_str(), paramName.wc_str());

	m_pECAEdit->WriteText(strParam);
	m_pECAEdit->SetFocus();
}