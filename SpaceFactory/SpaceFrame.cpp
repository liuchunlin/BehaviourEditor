
#include "SpaceFrame.h"
#include "wx/event.h"
#include "GWTreeCtrl.h"
#include "GWSplitterWindows.h"
#include "PropertyDialogs.h"
#include "IncludeDialogs.h"
#include "NodeManager.h"
#include "NodeVerify.h"

#include "TEEManager.h"
#include "NodeFactory.h"
#include "Option.h"
#include "SpaceIDs.h"
#include "GenerateCode.h"

#include "GWIDs.h"
#include "SpaceIDs.h"

extern std::wstring MakeValidPathName(const wchar_t* pPathName);

BEGIN_EVENT_TABLE(SpaceFrame, wxFrame)
EVT_SIZE(SpaceFrame::OnSize)
EVT_IDLE(SpaceFrame::OnIdle)
EVT_ACTIVATE(SpaceFrame::OnActive)

EVT_MENU(MENU_TEE_NEW_SPACE, SpaceFrame::OnNewSpace)
EVT_MENU(MENU_TEE_DELETE, SpaceFrame::OnDelete)
EVT_MENU(MENU_TEE_PROPERTY, SpaceFrame::OnProperty)
EVT_MENU(MENU_TEE_INCLUDE, SpaceFrame::OnInclude)
EVT_MENU(wxID_ANY, SpaceFrame::OnProcessAny)

EVT_TREE_ITEM_RIGHT_CLICK(CTRL_ECA_TREE, SpaceFrame::OnRightClickECATree)
EVT_TREE_ITEM_ACTIVATED(CTRL_ECA_TREE, SpaceFrame::OnActivateECATree)
EVT_TREE_SEL_CHANGED(CTRL_ECA_TREE, SpaceFrame::OnSelECATreeChanged)
EVT_TREE_ITEM_MENU(CTRL_ECA_TREE, SpaceFrame::OnItemMenu)
EVT_LISTBOX_DCLICK(CTRL_SPACE_LIB_LIST, SpaceFrame::OnLibListDClick)

END_EVENT_TABLE()

SpaceFrame::SpaceFrame(const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: GWFrame(NULL, id, title, pos, size, style)
	, m_pECATree(NULL)
	, m_pSplitterV(nullptr)
	, m_pLibsList(nullptr)
	, m_Root(NULL)
	, m_bChanged(false)
{
	// set the frame icon
	SetIcon(wxICON(sample));

	// help menu
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_About, _T("&About...\tF1"), _T("Show about dialog"));

	// file menu
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
	wxBitmap bmp = wxArtProvider::GetBitmap(_T("wxART_HELP_BOOK"), _T("wxART_OTHER_C"), wxSize(16, 16));
	toolBar->AddTool(CTRL_GEN_CODE, TEXT("生成代码"), bmp, wxNullBitmap, wxITEM_NORMAL, TEXT("生成代码"));

	toolBar->Realize();

	m_FileSuffix = _T("SPACE");
	InitWindow();

	//根据配置 载入文件:
	bool bSuccess = true;
	TCHAR strGameSpace[256];
	DWORD nBuffer = GetPrivateProfileString(_T("GameInfo"), _T("DefaultSpace"), _T(""), strGameSpace, sizeof(strGameSpace) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	TCHAR strTriggerFilePath[256];
	nBuffer = GetPrivateProfileString(_T("TriggersPath"), _T("path"), _T(""), strTriggerFilePath, sizeof(strTriggerFilePath) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	TCHAR strTEELibPath[256];
	nBuffer = GetPrivateProfileString(_T("TEELibPath"), _T("path"), _T(""), strTEELibPath, sizeof(strTEELibPath) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	TCHAR strCPPGeneratePath[256];
	nBuffer = GetPrivateProfileString(_T("CPPGeneratePath"), _T("path"), _T(""), strCPPGeneratePath, sizeof(strCPPGeneratePath) / sizeof(TCHAR), GConfigFileName);
	bSuccess &= nBuffer != 0;

	if (!bSuccess)
	{
		wxMessageDialog dialog(NULL, _T("请先检查配置文件:Profile.ini"), _T("错误"), wxOK | wxICON_ERROR);
		dialog.ShowModal();
		PostQuitMessage(0);
	}

	m_DefaultPath = MakeValidPathName(strTriggerFilePath);
	std::wstring ValidPath = MakeValidPathName(strCPPGeneratePath);
	GGenCode->SetGenerateCPPPath(ValidPath.c_str());

	InitLibsList();
	if (!LoadSpaces(MakeValidPathName(strTEELibPath) + wstring(strGameSpace), strGameSpace, true))
	{
		NewSpaces();
	}
}

SpaceFrame::~SpaceFrame()
{
	TEE::GTEEMgr->Flush();
}

void SpaceFrame::Resize()
{
	wxSize winSize = GetClientSize();
	if( m_pECATree && m_pLibsList )
	{
		wxSize libListSize = m_pLibsList->GetSize();
		m_pECATree->SetSize( winSize.x - libListSize.x, winSize.y );
	}
}

bool SpaceFrame::InitWindow()
{
	m_pSplitterV = new GWSplitterWindow( this, this );
	m_pSplitterV->SetWindowStyle( wxSP_3DSASH );
	m_pSplitterV->SetMinimumPaneSize(1);

	m_pECATree = new GWTreeCtrl( m_pSplitterV, CTRL_ECA_TREE, wxDefaultPosition, wxSize(500, 0), wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER | wxTR_EDIT_LABELS );
	m_pLibsList = new wxListBox(m_pSplitterV, CTRL_SPACE_LIB_LIST, wxDefaultPosition, wxSize(130, 0));
	m_pSplitterV->SplitVertically( m_pECATree, m_pLibsList );
	m_pSplitterV->UpdateSize();
	return true;
}

bool	SpaceFrame::NewSpaces()
{
	m_CurrentPath = _T("New File");
	m_CurrentName = wxEmptyString;
	SetChanged( false );
	this->SetTitle( m_CurrentPath );
	TEE::NodeBase* pRootNode = TEE::GTEEMgr->NewSpace();
	pRootNode->m_DisplayName = _T("新建空间库");
	pRootNode->m_CodeName = _T("RootNode");

	m_pECATree->DeleteAllItems();

	GWTreeItemData * pData = new GWTreeItemData( pRootNode->m_CodeName, pRootNode );
	m_ECARoot = m_pECATree->AddRoot( pRootNode->m_DisplayName, 0, 0, pData );

	return true;
}

TEE::NodeBase*	SpaceFrame::GetTreeItemNode( wxTreeItemId itemId )
{
	GWTreeItemData *pData = itemId.IsOk() ? static_cast<GWTreeItemData *>(m_pECATree->GetItemData(itemId)) : NULL;
	if( pData != NULL )
		return pData->m_pNode;

	return NULL;
}

bool	SpaceFrame::RefreshTEEValidate( wxTreeItemId itemId )
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

bool SpaceFrame::RefreshSubRoot( wxTreeItemId subRootItemId, bool bExpand )
{
	TEE::NodeBase* pNode = GetTreeItemNode( subRootItemId );
	bool isSubRoot = pNode && pNode->Parent() && pNode->Parent()->IsA(TEE::NT_ROOT);
	if(!isSubRoot) return false;

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

void SpaceFrame::OnProcessAny(wxCommandEvent& event)
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
				this->SetTitle( path );
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
		NewSpaces();
	}
	else if (event.GetId() == CTRL_GEN_CODE)
	{
		GGenCode->Gen();
	}

}

void	SpaceFrame::OnSize(wxSizeEvent& event)
{
	Resize();
	event.Skip();
}

void	SpaceFrame::OnIdle(wxIdleEvent& event)
{
	event.Skip();
}

void	SpaceFrame::OnNewSpace(wxCommandEvent& event)
{
	DoNewNode( m_ECARoot, TEE::NT_SPACEROOT );
}

void	SpaceFrame::OnDelete(wxCommandEvent& event)
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

bool	SpaceFrame::DoProperty( wxTreeItemId itemId )
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

void	SpaceFrame::OnProperty(wxCommandEvent& event)
{
	DoProperty( m_pECATree->GetSelection() );
	return;
}

void	SpaceFrame::OnInclude(wxCommandEvent& event)
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

bool	SpaceFrame::DoNewNode( wxTreeItemId parentId, TEE::NODE_TYPE eType )
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

void SpaceFrame::OnSelECATreeChanged(wxTreeEvent& event)
{
	wxTreeItemId selItemId = event.GetItem();
	event.Skip();   
}

void	SpaceFrame::OnRightClickECATree(wxTreeEvent& event)
{
	event.Skip(false);
}

void	SpaceFrame::OnActivateECATree(wxTreeEvent& event)
{
	DoProperty( event.GetItem() );
}

void	SpaceFrame::SetChanged( bool bChanged )
{
	if( bChanged == m_bChanged )
		return;

	m_bChanged = bChanged;

	SetTitle( wxString::Format( _T("%s%s"), m_CurrentPath, bChanged ? _T("*") : _T("") ) );
}

void	SpaceFrame::OnItemMenu(wxTreeEvent& event)
{
	wxTreeItemId eventItemId = event.GetItem();

	TEE::NodeBase* pNode = GetTreeItemNode(eventItemId);
	if (pNode == NULL)
		return;

	wxMenu menu;
	menu.Append(MENU_TEE_NEW_SPACE, wxT("新空间(&S)"));
	menu.Enable(MENU_TEE_NEW_SPACE, pNode->NewSpaceable());
	menu.AppendSeparator();
	menu.Append(MENU_TEE_DELETE, wxT("删除(&D)"));
	menu.Enable(MENU_TEE_DELETE, pNode->Deleteable());
	menu.Append(MENU_TEE_PROPERTY, wxT("属性(&P)"));
	menu.Enable(MENU_TEE_PROPERTY, pNode->Propertyable());
	menu.Append(MENU_TEE_INCLUDE, wxT("包含(&I)"));
	menu.Enable(MENU_TEE_INCLUDE, pNode->Includeable());

	wxPoint pt = event.GetPoint();
	pt.y += this->GetToolBar()->GetSize().y;
	pt += wxPoint(5, 5);
	PopupMenu(&menu, pt);
	event.Skip();
}

bool	SpaceFrame::LoadSpaces(const wstring& filePath, const wstring& fileName, bool bHost)
{
	TEE::GTEEMgr->Flush();
	m_Root = TEE::GTEEMgr->LoadSpaceNode(filePath);
	if (m_Root == NULL)
		return false;

	m_pECATree->DeleteAllItems();

	GWTreeItemData * pData = new GWTreeItemData(m_Root->m_CodeName.c_str(), m_Root);
	m_ECARoot = m_pECATree->AddRoot(m_Root->m_DisplayName.c_str(), 0, 0, pData);

	for (TEE::NodeBase* pChild = m_Root->FirstChild(); pChild; pChild = pChild->Next())
	{
		wxTreeItemId subRootItemId = m_pECATree->AppendItem(m_ECARoot, pChild->m_DisplayName.c_str(), 1, 1, new GWTreeItemData(pChild->m_CodeName.c_str(), pChild));
		RefreshSubRoot(subRootItemId, false);
	}
	m_pECATree->Expand(m_ECARoot);
	m_pECATree->Update();

	m_CurrentPath = filePath.c_str();
	m_CurrentName = fileName.c_str();

	SetChanged(false);
	SetTitle(m_CurrentPath);
	TEE::NodeBase::CheckErrorTree(m_Root);
	RefreshTEEValidate(m_pECATree->GetRootItem());

	TEE::GTEEMgr->LoadSpaceIncludes();
	return true;
}

bool SpaceFrame::ReloadSpace()
{
	return LoadSpaces(wstring(m_CurrentPath.wc_str()), wstring(m_CurrentName.wc_str()), true);
}

extern bool FindTEELibs(vector<wstring>& TEEFiles, vector<WIN32_FIND_DATA>* pFileAttrs);

void SpaceFrame::InitLibsList()
{
	m_pLibsList->Clear();

	vector<wstring> teeFiles;
	vector<WIN32_FIND_DATA> fileAttrs;
	FindTEELibs(teeFiles, &fileAttrs);

	for (auto it = teeFiles.begin(); it != teeFiles.end(); ++it)
	{
		m_pLibsList->AppendString(it->c_str());
	}
	m_fileTimes.clear();
	for (size_t i = 0; i < fileAttrs.size(); ++i)
	{
		m_fileTimes[fileAttrs[i].cFileName] = fileAttrs[i].ftLastWriteTime;
	}
}

#ifdef DEBUG
const wchar_t TEEEditorExe[] = TEXT("TEEEditorD.exe");
#else
const wchar_t TEEEditorExe[] = TEXT("TEEEditor.exe");
#endif

void SpaceFrame::OnLibListDClick(wxCommandEvent& event)
{
	int nSel = m_pLibsList->GetSelection();
	if (nSel == -1) return;

	wxString command = TEXT("-") + m_pLibsList->GetString(nSel);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(TEEEditorExe, const_cast<LPWSTR>(command.wc_str()), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
}

bool operator !=(FILETIME ft1, FILETIME ft2)
{
	return ft1.dwHighDateTime != ft2.dwHighDateTime ||
		ft1.dwLowDateTime != ft2.dwLowDateTime;
}

void SpaceFrame::OnActive(wxActivateEvent& event)
{
	// check tee files is changed
	vector<wstring> teeFiles;
	vector<WIN32_FIND_DATA> fileAttrs;
	FindTEELibs(teeFiles, &fileAttrs);

	bool bFileChanged = false;
	for (size_t i = 0; i < fileAttrs.size(); ++i)
	{
		WIN32_FIND_DATA attr = fileAttrs[i];
		auto itFileTime = m_fileTimes.find(attr.cFileName);
		if (itFileTime != m_fileTimes.end())
		{
			bFileChanged |= itFileTime->second != attr.ftLastWriteTime;
		}
		m_fileTimes[attr.cFileName] = attr.ftLastWriteTime;
	}
	// reload space when change
	if (bFileChanged)
	{
		ReloadSpace();
	}
}