#pragma once

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/treectrl.h"
#include "wx/richtext/richtextctrl.h"
#include <map>
#include <string>
#include <vector>
using namespace std;
#include "wx/sstream.h"

#if wxUSE_FILESYSTEM
#include "wx/filesys.h"
#include "wx/fs_mem.h"
#endif

#if wxUSE_HELP
#include "wx/cshelp.h"
#endif

#include "wx/richtext/richtextxml.h"
#include "wx/richtext/richtexthtml.h"

#include "GWFrame.h"

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"

namespace TEE
{
	class NodeBase;
	enum NODE_TYPE;
}
class GWSplitterWindow;
class wxRichTextCtrl;
class GWTreeCtrl;
class wxTreeEvent;

// IDs for the controls and the menu commands
enum
{
	// menu items
	ID_Quit = wxID_EXIT,
	ID_About = wxID_ABOUT,
};

class TEEFrame : public GWFrame
{
public:
	TEEFrame(const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~TEEFrame();
	virtual void	Resize();
	// event handlers (these functions should _not_ be virtual)

	virtual void	OnSize(wxSizeEvent& event);
	virtual void	OnIdle(wxIdleEvent& event);
	virtual void	OnSelECATreeChanged(wxTreeEvent& event);
	virtual void	OnSelECATreeChanging(wxTreeEvent& event);
	virtual void	OnRightClickECATree(wxTreeEvent& event);
	virtual void	OnActivateECATree(wxTreeEvent& event);

	virtual void	OnEditNotesChanged(wxRichTextEvent& event);

	virtual void	OnNewEvent(wxCommandEvent& event);
	virtual void	OnNewCondition(wxCommandEvent& event);
	virtual void	OnNewFunction(wxCommandEvent& event);
	virtual void	OnNewOption(wxCommandEvent& event);
	virtual void	OnNewSpace(wxCommandEvent& event);
	virtual void	OnDelete(wxCommandEvent& event);
	virtual void	OnProperty(wxCommandEvent& event);
	virtual void	OnInclude(wxCommandEvent& event);
	virtual void	OnSave(wxCommandEvent& event);
	virtual bool	DoNewNode( wxTreeItemId parentId, TEE::NODE_TYPE eType );
	virtual bool	DoProperty( wxTreeItemId itemId );

	void	OnItemMenu(wxTreeEvent& event);

	void OnBeginDrag(wxTreeEvent& event);
	void OnBeginRDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnProcessAny(wxCommandEvent& event);
	bool LoadSpaces(const wstring& filePath, const wstring& fileName, bool bHost);
	void InitOptionList();
	void OnOptionListChanged(wxCommandEvent &event);
	void OnAddParam(wxCommandEvent &event);

protected:
	virtual bool	InitWindow();
	virtual bool	NewTEELib();
	virtual TEE::NodeBase*	GetTreeItemNode( wxTreeItemId itemId );
	virtual bool	RefreshSelectNode( wxTreeItemId itemId );
	virtual bool	RefreshNodeEdit( wxTreeItemId itemId );
	virtual bool	RefreshNodeView( wxTreeItemId itemId );
	virtual bool	RefreshTEEValidate( wxTreeItemId itemId );
	virtual bool	RefreshSubRoot( wxTreeItemId subRootItemId, bool bExpand);
	const vector<pair<TEE::NODE_TYPE,wstring> >& GetRootNodes();
	void			SetChanged( bool bChanged );

protected:
	DECLARE_EVENT_TABLE()

	wxChoice* m_optionsChoice;
	wxChoice* m_adjustsChoice;
	wxButton* m_addParamButton;

	GWSplitterWindow					*m_pSplitterV;
	GWSplitterWindow					*m_pSplitterH;

	GWTreeCtrl							*m_pECATree;
	wxRichTextCtrl						*m_pECAEdit;
	wxRichTextCtrl						*m_pECAView;

	wxTreeItemId						m_ECARoot;
	map<int,wxTreeItemId>				m_ECARoots;
	map<wxTreeItemId, TEE::NodeBase*>	m_ECANodes;
	TEE::NodeBase						*m_Root;		

	wxString							m_DefaultPath;
	wxString							m_CurrentPath;
	wxString							m_CurrentName;
	wxString							m_FileSuffix;

	int									m_eDragType;		//left=1, right=2
	wxTreeItemId						m_DraggedItem;

	bool								m_bChanged;

};

extern wchar_t GConfigFileName[MAX_PATH];