#include "TgrFrame.h"
#include "TgrApp.h"
#include "wx/richtext/richtextctrl.h"
#include <wx/srchctrl.h>
#include "GalleryTree.h"
#include "EditTree.h"
#include "VarDlg.h"
#include "TgrImages.h"
#include "TgrIDs.h"
#include "XmlUtility.h"
#include "CmdManager.h"
#include "SpaceManager.h"
#include "NodeVerify.h"
#include "LuaCode.h"
#include "TEEManager.h"
#include "Process_LuaProtocol.h"
#include <Vek/Base/Service.h>
#include "wx/filename.h"
#include "CancelableWindow.h"
#include "Utility/Utils.h"


void Log(const wchar_t* pszFormat, ...)
{
	va_list argptr;
	va_start(argptr, pszFormat);
	wxString s;
	s.PrintfV(pszFormat, argptr);
	va_end(argptr);

	wxGetApp().GetTgrFrame()->AddLog(s);
}

CancelableWindow* GetParentCancelableWindow(wxWindow* window)
{
	while (window)
	{
		CancelableWindow* cancelable = dynamic_cast<CancelableWindow*>(window);
		if (cancelable)
			return cancelable;
		window = window->GetParent();
	}
	return nullptr;
}
CancelableWindow* GetCurrentCancelableWindow()
{
	return GetParentCancelableWindow(wxWindow::FindFocus());
}

Space::SNode* GetEditingNode()
{
	return wxGetApp().GetTgrFrame()->GetEditingNode();
}

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(TgrEditFrame, wxFrame)
EVT_SIZE(TgrEditFrame::OnSize)
EVT_IDLE(TgrEditFrame::OnIdle)
EVT_CHILD_FOCUS(TgrEditFrame::OnChildFocus)
EVT_MENU(wxID_ANY, TgrEditFrame::OnProcessAny)
EVT_TREE_SEL_CHANGED(TGR_CLASSIFY_TREE, TgrEditFrame::OnSelClassifyItemChanged)
EVT_TREE_SEL_CHANGED(TGR_TRIGGER_TREE, TgrEditFrame::OnSelTriggerItemChanged)
EVT_SEARCHCTRL_SEARCH_BTN(TGR_SEARCH_EDIT, TgrEditFrame::OnSearchTrigger)
EVT_TEXT_ENTER(TGR_SEARCH_EDIT, TgrEditFrame::OnSearchTrigger)
EVT_CHOICE(TGR_SPACES_CHOICE, TgrEditFrame::OnSpaceChoice)
END_EVENT_TABLE()

// frame constructor
TgrEditFrame::TgrEditFrame(const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, id, title, pos, size, style)
	, m_DefaultTitleName(title)
	, m_iLastSpaceSelect(-1)
	, m_currentTime(timeGetTime())
{
	// set the frame icon
	SetIcon(wxICON(sample));

	InitMenu();
	InitBar();

	InitWindow();

	wstring content;
	if( Space::XmlUtility::LoadAuiPerspective( _T("perspective.p"), content ) )
	{
		m_mgr.LoadPerspective( content.c_str() );
	}

	InitFiles();

	UpdateCommandControls();
	UpdateCopyControls();

	SetFileChanged(false);
	SetConnectState(false);
	UpdateTitle();

	//Space::INode::SetDeleteCallback( NodeDeleting );
}
TgrEditFrame::~TgrEditFrame()
{
	if (m_bFileChanged)
	{
		wxMessageDialog dialog( this, TEXT("是否保存更改？"), TEXT(""), wxYES_NO|wxICON_WARNING);
		if (dialog.ShowModal() == wxID_YES)
		{
			SaveTrigger();
		}
	}

	m_mgr.UnInit();

	wxString AUIPerspective = m_mgr.SavePerspective();
	Space::XmlUtility::SaveAuiPerspective( _T("perspective.p"), AUIPerspective.wc_str() );

	TEE::GTEEMgr->Flush();
	Space::GSpaceMgr->Flush();
	GVekService->Flush();
}

extern std::wstring MakeValidPathName(const wchar_t* pPathName);

bool	TgrEditFrame::InitFiles()
{
	//根据配置 载入文件:
	bool bSuccess = true;
	TCHAR strGameSpace[256];
	DWORD nBuffer = GetPrivateProfileString( _T("GameInfo"), _T("DefaultSpace"), _T(""), strGameSpace, sizeof(strGameSpace)/sizeof(TCHAR), GConfigFileName );
	bSuccess &= nBuffer != 0;

	TCHAR strTriggerFilePath[256];
	nBuffer = GetPrivateProfileString( _T("TriggersPath"), _T("path"), _T(""), strTriggerFilePath, sizeof(strTriggerFilePath)/sizeof(TCHAR), GConfigFileName );
	bSuccess &= nBuffer != 0;

	TCHAR strTEELibPath[256];
	nBuffer = GetPrivateProfileString( _T("TEELibPath"), _T("path"), _T(""), strTEELibPath, sizeof(strTEELibPath)/sizeof(TCHAR), GConfigFileName );
	bSuccess &= nBuffer != 0;

	TCHAR strLuaGeneratePath[256];
	nBuffer = GetPrivateProfileString( _T("LuaGeneratePath"), _T("path"), _T(""), strLuaGeneratePath, sizeof(strLuaGeneratePath)/sizeof(TCHAR), GConfigFileName );
	bSuccess &= nBuffer != 0;

	if( !bSuccess )
	{
		wxMessageDialog dialog( NULL, _T("请先检查配置文件:Profile.ini"), _T("错误"), wxOK|wxICON_ERROR);
		dialog.ShowModal();
		PostQuitMessage(0);
	}
	GGenCode->SetGenerateLuaPath( MakeValidPathName(strLuaGeneratePath).c_str() );

	LoadSpaceInfos( MakeValidPathName(strTriggerFilePath)+wxString(strGameSpace) );

	LoadCurrentSpace();

	return true;
}

bool	TgrEditFrame::Show(bool show /*= true*/)
{
	GVekService->Init();
	return wxFrame::Show( show );
}
bool	TgrEditFrame::LoadCurrentSpace()
{
	int iSelect = m_SpacesChoice->GetSelection();
	if( iSelect == -1  )
		return false;

	TEE::NodeBase* pSpaceNode = static_cast<TEE::NodeBase*>( m_SpacesChoice->GetClientData( iSelect ) );
	if( pSpaceNode == NULL )
		return false;

	wxBusyCursor wait;

	TEE::GTEEMgr->FlushIncludes();
	TEE::GTEEMgr->LoadTEEInclude( pSpaceNode );
	wxString strFileName = wxString::Format( _T("%s.TGR"), pSpaceNode->CodeName().c_str() );
	strFileName = wxFileName(m_SpaceFileName).GetPath(true) + strFileName;
	Space::GSpaceMgr->Flush();
	Space::SNode* pRoot = Space::GSpaceMgr->LoadNode( strFileName.wc_str() );
	if( pRoot != NULL )
	{
		SetFileChanged( false );
	}
	else
	{
		pRoot = Space::GSpaceMgr->BuildDefault<Space::SNT_ROOT>();
		pRoot->Name( pSpaceNode->CodeName() );
		SetFileChanged( true );
	}
	m_pGalleryTree->UpdateNewRoot( pRoot );
	m_pEditTree->OnSelectGalleryNode( NULL );

	GCmdMgr->Flush();
	UpdateCommandControls();
	UpdateCopyControls();

	UpdateTitle();

	AddLog( wxString::Format( _T("打开空间:%s"), pSpaceNode->DisplayName().c_str() ) );
	return true;
}
bool	TgrEditFrame::LoadSpaceInfos( const wxString& fileName )
{
	m_SpaceFileName = fileName;

	m_SpacesChoice->Clear();

	TEE::NodeBase* pSpaceRoot = TEE::GTEEMgr->LoadSpaceNode( fileName.wc_str() );

	vector<TEE::NodeBase*> spaces;
	pSpaceRoot->FindChildren(TEE::VerifyNodeType(TEE::NT_SPACEROOT), spaces);
	vector<TEE::NodeBase*>::iterator iter( spaces.begin() );
	for( ; iter != spaces.end(); ++iter )
	{
		TEE::NodeBase* pSpace = (*iter);
		m_SpacesChoice->Append( pSpace->DisplayName(), reinterpret_cast<void*>(pSpace) );
	}
	m_iLastSpaceSelect = -1;

	m_pGalleryTree->UpdateNewRoot( NULL );
	m_pEditTree->OnSelectGalleryNode( NULL );

	SetFileChanged( false );
	UpdateTitle();

	GCmdMgr->Flush();
	UpdateCommandControls();
	UpdateCopyControls();

	AddLog( wxString::Format( _T("载入了游戏:%s"), pSpaceRoot->DisplayName().c_str() ) );

	return true;
}

bool	TgrEditFrame::InitMenu()
{
	// create a menu bar
	m_OperatorMenu = new wxMenu;

	m_OperatorMenu->Append(wxID_OPEN,		_T("打开"), _T(""));
	m_OperatorMenu->Append(wxID_SAVE,		_T("保存并上传"), _T(""));
	m_OperatorMenu->Append(TGR_PUBLISH,		_T("发布"), _T(""));
	m_OperatorMenu->AppendSeparator();
	m_OperatorMenu->Append(TGR_CONNECT,		_T("连接"), _T(""));
	m_OperatorMenu->Append(TGR_DISCONNECT,	_T("断开"), _T(""));

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append( m_OperatorMenu, _T("操作") );

	SetMenuBar(menuBar);

	return true;
}
bool	TgrEditFrame::InitBar()
{
	wxToolBar* toolBar = CreateToolBar();
	toolBar->AddTool(wxID_OPEN, TEXT("Open"), wxBitmap(open_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Open"));
	toolBar->AddTool(wxID_SAVE, TEXT("Save"), wxBitmap(save_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Save"));

	toolBar->AddSeparator();
	toolBar->AddTool(wxID_CUT, TEXT("Cut"), wxBitmap(cut_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Cut"));
	toolBar->AddTool(wxID_COPY, TEXT("Copy"), wxBitmap(copy_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Copy"));
	toolBar->AddTool(wxID_PASTE, TEXT("Paste"), wxBitmap(paste_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Paste"));
	toolBar->AddSeparator();
	toolBar->AddTool(wxID_UNDO, TEXT("Undo"), wxBitmap(undo_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Undo"));
	toolBar->AddTool(wxID_REDO, TEXT("Redo"), wxBitmap(redo_xpm), wxNullBitmap, wxITEM_NORMAL, TEXT("Redo"));
	toolBar->AddSeparator();

#if 1
	m_SpacesChoice = new wxChoice(toolBar, TGR_SPACES_CHOICE );
	toolBar->AddControl(m_SpacesChoice);
#endif
	toolBar->AddSeparator();

	wxBitmap bmp = wxArtProvider::GetBitmap( _T("wxART_HELP_BOOK"), _T("wxART_OTHER_C"), wxSize(16,16));
	toolBar->AddTool(TGR_FRAME_BAR_VARIABLE, TEXT("变量"), bmp, wxNullBitmap, wxITEM_NORMAL, TEXT("变量"));

	toolBar->Realize();

	return true;
}

void TgrEditFrame::OnSize(wxSizeEvent& event)
{
	event.Skip();
}
void	TgrEditFrame::OnIdle(wxIdleEvent& event)
{
	event.Skip();

	DWORD lastTime = m_currentTime;
	m_currentTime = timeGetTime();
	GVekService->Tick( m_currentTime * 0.001f );
}
void	TgrEditFrame::OnSpaceChoice( wxCommandEvent &event )
{
	if( m_iLastSpaceSelect == m_SpacesChoice->GetSelection() )
		return;

	if( m_bFileChanged )
	{
		wxMessageDialog dialog( NULL, _T("是否放弃修改？"), _T("警告"), wxOK|wxCANCEL|wxICON_WARNING);
		int msg = dialog.ShowModal();
		if( msg != wxID_OK )
		{
			if( m_iLastSpaceSelect != -1 )
			{
				m_SpacesChoice->Select( m_iLastSpaceSelect );
			}
			return ;
		}
	}
	m_iLastSpaceSelect = m_SpacesChoice->GetSelection();
	LoadCurrentSpace();
}

bool TgrEditFrame::InitWindow()
{
	m_mgr.SetManagedWindow(this);

	m_pGalleryTree = new GalleryTree( this, TGR_CLASSIFY_TREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_EDIT_LABELS | wxSUNKEN_BORDER | wxTR_MULTIPLE );
	m_mgr.AddPane( m_pGalleryTree, wxAuiPaneInfo().Name(wxT("space")).PaneBorder(true).CloseButton(false).CaptionVisible(false));

	m_searchEdit = new wxSearchCtrl(this, TGR_SEARCH_EDIT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	m_mgr.AddPane(m_searchEdit, wxAuiPaneInfo().Name(wxT("find")).PaneBorder(true).Fixed().CloseButton(false).CaptionVisible(false));

	m_pEdit = CreateTextCtrl( wxT("") );
	m_pEdit->SetEditable( false );
	m_mgr.AddPane( m_pEdit, wxAuiPaneInfo().Name(wxT("edit")).CenterPane());

	m_pEditTree = new EditTree( this, TGR_TRIGGER_TREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_EDIT_LABELS | wxSUNKEN_BORDER | wxTR_MULTIPLE );
	m_mgr.AddPane( m_pEditTree, wxAuiPaneInfo().Name(wxT("trigger")).CenterPane());

	m_mgr.SetFlags(m_mgr.GetFlags() | wxAUI_MGR_ALLOW_ACTIVE_PANE);
	m_mgr.GetPane(wxT("find")).Show().Left().Position(0);
	m_mgr.GetPane(wxT("space")).Show().Left().Position(1);
	m_mgr.GetPane(wxT("trigger")).Show().Position(0);
	m_mgr.GetPane(wxT("edit")).Show().Position(1);

	m_mgr.Update();

	wxTheApp->Connect(wxID_ANY, wxEVT_KEY_DOWN,	wxKeyEventHandler(TgrEditFrame::OnKeyDown),(wxObject*)0, this);

	return true;
}

wxRichTextCtrl* TgrEditFrame::CreateTextCtrl(const wxString& ctrl_text)
{
	return new wxRichTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER|wxWANTS_CHARS );
}

void	TgrEditFrame::OnChildFocus(wxChildFocusEvent& event)
{
	UpdateCommandControls();
}

void	TgrEditFrame::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetEventType() == wxEVT_KEY_DOWN)
	{
		long keycode = event.GetKeyCode();
		switch (keycode)
		{
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
			OnDelete();
			break;
		case WXK_F3:
			SearchTrigger();
			break;
		}
		if( event.ControlDown() )
		{
			if( keycode == 'C' )
			{
				//Copy
				OnCopy();
			}
			else if( keycode == 'V' )
			{
				//Paste
				OnPaste();
			}
			else if( keycode == 'X' )
			{
				//Cut
				OnCut();
			}
			else if( keycode == 'Z' )
			{
				if (CancelableWindow* canceable = GetCurrentCancelableWindow())
				{
					canceable->Undo();
				}
			}
			else if( keycode == 'Y' )
			{
				if (CancelableWindow* canceable = GetCurrentCancelableWindow())
				{
					canceable->Redo();
				}
			}
			else if( keycode == 'S' )
			{
				SaveTrigger();
			}
		}
	}
	event.Skip();
}

void	TgrEditFrame::OnProcessAny(wxCommandEvent& event)
{
	switch(event.GetId())
	{
	case wxID_SAVE:
		if( !event.IsChecked() )
			SaveTrigger();
		break;
	case TGR_PUBLISH:
		if( !event.IsChecked() )
			SaveTrigger(true);
		break;
	case wxID_NEW:
		//NewSpaceRoot();
		break;
	case wxID_REDO:
		if (CancelableWindow* canceable = GetCurrentCancelableWindow())
		{
			canceable->Redo();
		}
		break;
	case wxID_UNDO:
		if (CancelableWindow* canceable = GetCurrentCancelableWindow())
		{
			canceable->Undo();
		}
		break;
	case wxID_OPEN:
		if( !event.IsChecked() )
			OpenSpaces();
		break;
	case wxID_CUT:
		OnCut();
		break;
	case wxID_COPY:
		OnCopy();
		break;
	case wxID_PASTE:
		OnPaste();
		break;
	case TGR_FRAME_BAR_VARIABLE:
		{
			VarListDlg dlg(this);
			dlg.ShowModal();
		}
		break;
	case TGR_CONNECT:
		GLuaProto->Connect();
		break;
	case TGR_DISCONNECT:
		GLuaProto->DisConnect();
		break;
	default:
		break;
	}
	event.Skip();
}
void	TgrEditFrame::UpdateCommandControls()
{
	CancelableWindow* canceable = GetCurrentCancelableWindow();
	GetToolBar()->EnableTool( wxID_REDO, canceable && canceable->CanReDo());
	GetToolBar()->EnableTool( wxID_UNDO, canceable && canceable->CanUnDo());
}
void	TgrEditFrame::UpdateCopyControls()
{
	Space::SNode *pNode = GCmdMgr->Selected();
	GetToolBar()->EnableTool( wxID_CUT, pNode == NULL ? false : pNode->Cutable() );
	GetToolBar()->EnableTool( wxID_COPY, pNode == NULL ? false : pNode->Copyable() );
	GetToolBar()->EnableTool( wxID_PASTE, pNode == NULL ? false : true );
}

void	TgrEditFrame::OnSelClassifyItemChanged(wxTreeEvent& event)
{
	Space::SNode* pNode = m_pGalleryTree->GetSpaceNode( event.GetItem() );
	m_pEditTree->OnSelectGalleryNode( pNode );

	GCmdMgr->Selected( pNode );
	UpdateCopyControls();
	event.Skip();
}
void	TgrEditFrame::OnSelTriggerItemChanged(wxTreeEvent& event)
{
	Space::SNode* pNode = m_pEditTree->GetSpaceNode( event.GetItem() );
	m_pEditTree->OnSelectNode( event.GetItem(), pNode );

	GCmdMgr->Selected( pNode );
	UpdateCopyControls();
	event.Skip();
}

void	TgrEditFrame::OnSearchTrigger(wxCommandEvent& event)
{
	SearchTrigger();
}

void	TgrEditFrame::SearchTrigger()
{
	wxString searchText = m_searchEdit->GetValue();
	if (searchText.length() == 0) return;

	Space::SNode* searchStartNode = GetSearchStartNode();
	Space::SNode* targetNode = SearchNode(searchStartNode, searchText);
	if (targetNode == nullptr) return;

	Space::SNode* classifyNode = nullptr;
	if (targetNode->ClassType() == Space::SNT_GROUP ||
		targetNode->ClassType() == Space::SNT_TRIGGER)
		classifyNode = targetNode;
	else
		classifyNode = targetNode->Ancestor(Space::SNT_TRIGGER);
	if (classifyNode)
	{
		m_pGalleryTree->SelectNode(classifyNode);
	}

	if (targetNode != classifyNode)
	{
		m_pEditTree->SelectNode(targetNode);
		m_pEditTree->SetFocus();
	}
	else
	{
		m_pGalleryTree->SetFocus();
	}
}

Space::SNode* TgrEditFrame::GetSearchStartNode()
{
	Space::SNode* selTriggerNode = m_pEditTree->GetSelectedNode();
	Space::SNode* selClassifyNode = m_pGalleryTree->GetSelectedNode();
	return selTriggerNode != nullptr ? selTriggerNode : selClassifyNode;
}

Space::SNode* NavigationNext(Space::SNode* current, bool passCurrent = false);
Space::SNode* GetParentWithWidget(Space::SNode* pNode);
Space::SNode* TgrEditFrame::SearchNode(Space::SNode* searchStartNode, wxString searchText)
{
	if (searchStartNode == nullptr || searchStartNode->ClassType() == Space::SNT_ROOT)
	{
		searchStartNode = Space::GSpaceMgr->FindRoot(Space::SNT_GROUP, nullptr);
	}
	else
	{
		if (searchStartNode->ClassType() == Space::SNT_GROUP ||
			searchStartNode->ClassType() == Space::SNT_TRIGGER)
		{
			searchStartNode = NavigationNext(searchStartNode);
		}
		else
		{
			searchStartNode = GetParentWithWidget(searchStartNode);
			searchStartNode = NavigationNext(searchStartNode, true);
		}
	}

	Space::SNode* current = searchStartNode;
	while (current)
	{
		wstring text = current->GetReadText();
		if (ci_find_substr(text, wstring(searchText.wc_str())) != -1)
			return current;
		current = NavigationNext(current);
	}
	return nullptr;
}

Space::SNode* GetParentWithWidget(Space::SNode* pNode)
{
	if (pNode == nullptr) return nullptr;
	while (pNode)
	{
		wxTreeCtrl* pTree = reinterpret_cast<wxTreeCtrl*>(pNode->UserData("tree"));
		if (pTree != nullptr) return pNode;
		pNode = pNode->Parent();
	}
	return nullptr;
}

Space::SNode* NavigationNext(Space::SNode* current, bool passCurrent)
{
	if (current == nullptr) return nullptr;
	if (!passCurrent && current->FirstChild()) return current->FirstChild();
	if (current->Next()) return current->Next();

	Space::SNode* parent = current->Parent();
	while (parent)
	{
		if (parent->Next()) return parent->Next();
		parent = parent->Parent();
	}
	return nullptr;
}

bool	TgrEditFrame::NewSpaceRoot()
{
	assert( false );
	return false;

	m_SpaceFileName.clear();

	Space::SNode*	pRoot = Space::GSpaceMgr->BuildDefault<Space::SNT_ROOT>();
	m_pGalleryTree->UpdateNewRoot( pRoot );
	m_pEditTree->DeleteAllItems();

	GCmdMgr->Flush();
	UpdateCommandControls();

	SetFileChanged( false );
	UpdateTitle();
	return true;
}

bool	TgrEditFrame::SaveTrigger(bool bCompileLua)
{
	Space::SNode* pChiefRoot = Space::GSpaceMgr->ChiefRoot();
	if( pChiefRoot == NULL )
		return false;
	int iSelect = m_SpacesChoice->GetSelection();
	if( iSelect == -1  )
		return false;
	TEE::NodeBase* pSpaceNode = static_cast<TEE::NodeBase*>( m_SpacesChoice->GetClientData( iSelect ) );
	if( pSpaceNode == NULL )
		return false;

	Space::ErrorMask flag = pChiefRoot->GetErrorMask();
	if( flag != Space::VS_OK )
	{
		wxMessageDialog dialog( NULL, _T("文件中包含错误格式，请根据红色提示修改"), _T("提示"), wxOK|wxICON_INFORMATION);
		dialog.ShowModal();
		return false;
	}

	wxBusyCursor wait;

	bool bSuccess = true;
	if( Space::GSpaceMgr->GetTriggerFileName().length() == 0 )
	{
		wxFileDialog dialog( this, _("保存TGR脚本"), wxFileName(m_SpaceFileName).GetPath(), TEXT(""), _T("Tgr files (*.TGR)|*.TGR"), wxFD_SAVE );
		if (dialog.ShowModal() == wxID_OK)
		{
			wxString FilePath = dialog.GetPath();
			if ( !FilePath.empty() )
			{
				bSuccess &= Space::GSpaceMgr->SaveNode( FilePath.wc_str(), pChiefRoot );
			}
		}
	}
	else
	{
		bSuccess &= Space::GSpaceMgr->SaveNode( (wxFileName(m_SpaceFileName).GetPath(true)+pSpaceNode->CodeName().c_str()+_T(".TGR")).wc_str(), pChiefRoot );
	}
	SetFileChanged( false );

	if (!bSuccess)
	{
		AddLog(_T("保存失败！"));
	}

	// 生成代码并重启空间
	if ( !GGenCode->Gen() )
	{
		AddLog(_T("生成lua代码失败！"));
		bSuccess = false;
	}

	if (bSuccess && bCompileLua)
	{
		if ( !GGenCode->CompileLua() )
		{
			AddLog(_T("编译lua代码失败！"));
			bSuccess = false;
		}
	}

	if (bSuccess)
	{
		AddLog(_T("保存成功！"));
	}

	return bSuccess;
}
bool	TgrEditFrame::OpenSpaces()
{
	wxString FilePath = wxFileName(m_SpaceFileName).GetPath(true);
	wxFileDialog dialog( this, _("打开Space文件"), FilePath, TEXT(""), _T("Space files (*.SPACE)|*.SPACE"), wxFD_OPEN );
	bool bSuccess = true;
	if (dialog.ShowModal() == wxID_OK)
	{
		FilePath = dialog.GetPath();
		if ( !FilePath.empty() )
		{
			LoadSpaceInfos( FilePath );
		}
	}

	return bSuccess;
}
bool	TgrEditFrame::OnCut()
{
	wxWindow* pFocus = FindFocus();
	if( m_pGalleryTree == pFocus )
	{
		m_pGalleryTree->OnCut();
	}
	else if( m_pEdit == pFocus )
	{
		//m_pEdit->Cut();
	}
	else if( m_pEditTree == pFocus )
	{
		m_pEditTree->OnCut();
	}
	else
	{
		//assert( false );
		return false;
	}
	UpdateCopyControls();
	return true;
}
bool	TgrEditFrame::OnCopy()
{
	wxWindow* pFocus = FindFocus();
	if( m_pGalleryTree == pFocus )
	{
		m_pGalleryTree->OnCopy();
	}
	else if( m_pEdit == pFocus )
	{
		//m_pEdit->Cut();
	}
	else if( m_pEditTree == pFocus )
	{
		m_pEditTree->OnCopy();
	}
	else
	{
		//assert( false );
		return false;
	}
	UpdateCopyControls();
	return true;
}
bool	TgrEditFrame::OnPaste()
{
	wxWindow* pFocus = FindFocus();
	if( m_pGalleryTree == pFocus )
	{
		m_pGalleryTree->OnPaste();
	}
	else if( m_pEdit == pFocus )
	{
		//m_pEdit->Cut();
	}
	else if( m_pEditTree == pFocus )
	{
		m_pEditTree->OnPaste();
	}
	else
	{
		//assert( false );
		return false;
	}
	return true;
}

bool	TgrEditFrame::OnDelete()
{
	wxWindow* pFocus = FindFocus();
	if( m_pGalleryTree == pFocus )
	{
		m_pGalleryTree->OnDelete();
	}
	else if( m_pEdit == pFocus )
	{
		//m_pEdit->Cut();
	}
	else if( m_pEditTree == pFocus )
	{
		m_pEditTree->OnDelete();
	}
	else
	{
	//	assert( false );
		return false;
	}
	return true;
}
void	TgrEditFrame::CmdNodeDeleting( Space::SNode* pNode )
{
	UpdateCopyControls();
}
Space::SNode*	TgrEditFrame::GetEditingNode()
{
	return m_pEditTree->GetEditingNode();
}
void	TgrEditFrame::SetFileChanged( bool bChanged )
{
	if( bChanged == m_bFileChanged )
		return;

	m_bFileChanged = bChanged;
	UpdateTitle();
}
void	TgrEditFrame::SetConnectState( bool bConnected )
{
	m_OperatorMenu->Enable( TGR_CONNECT, !bConnected );
	m_OperatorMenu->Enable( TGR_DISCONNECT, bConnected );

	if( bConnected )
	{
		wstring locIp;
		int locPort;
		wstring serverIp;
		int serverPort;
		GLuaProto->GetIpPort_( locIp, locPort, serverIp, serverPort );
		AddLog( wxString::Format( _T("已连接,本机Ip:%s,Port:%d,服务器Ip:%s, Port:%d"), locIp.c_str(), locPort, serverIp.c_str(), serverPort ) );
	}
	else
	{
		AddLog( wxString::Format( _T("断开服务器") ) );
	}

	if( m_bConnected == bConnected )
		return;

	m_bConnected = bConnected;
	UpdateTitle();
}
void	TgrEditFrame::UpdateTitle()
{
	int iSelect = m_SpacesChoice->GetSelection();
	if( iSelect == -1  )
		return;

	TEE::NodeBase* pSpaceNode = static_cast<TEE::NodeBase*>( m_SpacesChoice->GetClientData( iSelect ) );
	wxString CurrentTriggerName;
	if( pSpaceNode != NULL )
	{
		CurrentTriggerName = pSpaceNode->DisplayName().c_str();
	}
	SetTitle( wxString::Format( _T("%s - %s %s(%s)"), m_DefaultTitleName, CurrentTriggerName, m_bFileChanged ? _T("*") : _T(""), m_bConnected ? _T("联机") : _T("脱机") ) );
}
void	TgrEditFrame::AddLog( const wxString& log )
{
	m_pEdit->SetInsertionPoint(m_pEdit->GetLastPosition());
	m_pEdit->WriteText( log+_T("\n") );
	m_pEdit->SetModified(false);
	m_pEdit->ShowPosition( m_pEdit->GetLastPosition() );
}
void	TgrEditFrame::ClearLog()
{
	m_pEdit->Clear();
}