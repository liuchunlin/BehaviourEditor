// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
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
#include "GWFrame.h"

#include "wx/richtext/richtextxml.h"
#include "wx/richtext/richtexthtml.h"

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

class SpaceFrame : public GWFrame
{
public:
	SpaceFrame(const wxString& title, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
	~SpaceFrame();
	virtual void	Resize();
	// event handlers (these functions should _not_ be virtual)

	virtual void	OnSize(wxSizeEvent& event);
	virtual void	OnIdle(wxIdleEvent& event);
	void OnActive(wxActivateEvent& event);
	virtual void	OnSelECATreeChanged(wxTreeEvent& event);
	virtual void	OnRightClickECATree(wxTreeEvent& event);
	virtual void	OnActivateECATree(wxTreeEvent& event);
	void OnLibListDClick(wxCommandEvent& event);

	virtual void	OnNewSpace(wxCommandEvent& event);
	virtual void	OnDelete(wxCommandEvent& event);
	virtual void	OnProperty(wxCommandEvent& event);
	virtual void	OnInclude(wxCommandEvent& event);
	virtual bool	DoNewNode( wxTreeItemId parentId, TEE::NODE_TYPE eType );
	virtual bool	DoProperty( wxTreeItemId itemId );

	void	OnItemMenu(wxTreeEvent& event);
	void	OnProcessAny(wxCommandEvent& event);
	bool	LoadSpaces(const wstring& filePath, const wstring& fileName, bool bHost);
	bool	ReloadSpace();

protected:
	virtual bool	InitWindow();
	virtual bool	NewSpaces();
	virtual TEE::NodeBase*	GetTreeItemNode( wxTreeItemId itemId );
	virtual bool	RefreshTEEValidate( wxTreeItemId itemId );
	virtual bool	RefreshSubRoot( wxTreeItemId subRootItemId, bool bExpand);
	void InitLibsList();
	void SetChanged( bool bChanged );

protected:
	DECLARE_EVENT_TABLE()

	GWSplitterWindow*	m_pSplitterV;

	GWTreeCtrl*			m_pECATree;
	wxListBox*			m_pLibsList;

	wxTreeItemId		m_ECARoot;
	TEE::NodeBase*		m_Root;		

	wxString			m_DefaultPath;
	wxString			m_CurrentPath;
	wxString			m_CurrentName;
	wxString			m_FileSuffix;
	map<wstring, FILETIME> m_fileTimes;

	bool				m_bChanged;

};

extern wchar_t GConfigFileName[MAX_PATH];