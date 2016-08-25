#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/aui/aui.h"
#include "wx/treectrl.h"


class wxRichTextCtrl;
class GalleryTree;
class EditTree;
class wxSearchCtrl;
namespace Space
{
	class SNode;
}
extern void Log(const wchar_t* pszFormat, ...);


// Define a new frame type: this is going to be our main frame
class TgrEditFrame : public wxFrame
{
public:
	TgrEditFrame( const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style );
	~TgrEditFrame();

	virtual bool		Show(bool show = true);
	// event handlers (these functions should _not_ be virtual)
	void				OnSize(wxSizeEvent& event);
	void				OnIdle(wxIdleEvent& event);
	void				OnProcessAny(wxCommandEvent& event);
	void				OnSelClassifyItemChanged(wxTreeEvent& event);
	void				OnSelTriggerItemChanged(wxTreeEvent& event);
	void				OnSearchTrigger(wxCommandEvent& event);
	void				OnKeyDown(wxKeyEvent& event);
	void				OnSpaceChoice( wxCommandEvent &event );
	void				OnChildFocus(wxChildFocusEvent& event);

	void				UpdateCommandControls();
	void				UpdateCopyControls();
	void				CmdNodeDeleting( Space::SNode* pNode );
	Space::SNode*		GetEditingNode();
	EditTree*			GetEditTree(){ return m_pEditTree; }
	void				SetFileChanged( bool bChanged );
	void				SetConnectState( bool bConnected );
	bool				IsConnected() { return m_bConnected; }
	void				UpdateTitle();

	//Log
	void				AddLog( const wxString& log );
	void				ClearLog();
private:
	wxRichTextCtrl*		CreateTextCtrl( const wxString& ctrl_text );
	Space::SNode*		GetSearchStartNode();
	Space::SNode*		SearchNode(Space::SNode* searchStartNode, wxString searchText);
	void				SearchTrigger();

	bool				InitMenu();
	bool				InitBar();
	bool				InitWindow();

	bool				InitFiles();
	bool				LoadSpaceInfos( const wxString& fileName );
	bool				LoadCurrentSpace();
	bool				NewSpaceRoot();
	bool				SaveTrigger(bool bCompileLua = false);
	bool				OpenSpaces();
	bool				OnCut();
	bool				OnCopy();
	bool				OnPaste();
	bool				OnDelete();

	// any class wishing to process wxWidgets events must use this macro
	DECLARE_EVENT_TABLE()

	//wxCtrl
	wxMenu*							m_OperatorMenu;
	GalleryTree*					m_pGalleryTree;
	EditTree*						m_pEditTree;
	wxRichTextCtrl*					m_pEdit;
	wxSearchCtrl*					m_searchEdit;
	wxAuiManager					m_mgr;
	wxChoice						*m_SpacesChoice;
	int								m_iLastSpaceSelect;
	
	//trigger info
	wxString						m_DefaultTitleName;
	wxString						m_SpaceFileName;

	bool							m_bFileChanged;	
	bool							m_bConnected;	
	DWORD							m_currentTime;
};

