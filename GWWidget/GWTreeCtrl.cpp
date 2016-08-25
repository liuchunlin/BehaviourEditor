#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/log.h"
#endif

#include "wx/colordlg.h"
#include "wx/numdlg.h"

#include "wx/treebase.h"
#include "wx/image.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"
#include "wx/math.h"

#ifdef __WIN32__
// this is not supported by native control
#define NO_VARIABLE_HEIGHT
#endif

#include "GWIDs.h"
#include "GWTreeCtrl.h"
#include "NodeBase.h"

#include "bitmaps/icon1.xpm"
#include "bitmaps/icon2.xpm"
#include "bitmaps/icon3.xpm"
#include "bitmaps/icon4.xpm"
#include "bitmaps/icon5.xpm"

BEGIN_EVENT_TABLE(GWTreeCtrl, wxTreeCtrl)
EVT_TREE_BEGIN_LABEL_EDIT(CTRL_ECA_TREE, GWTreeCtrl::OnBeginLabelEdit)
EVT_TREE_END_LABEL_EDIT(CTRL_ECA_TREE, GWTreeCtrl::OnEndLabelEdit)
EVT_TREE_DELETE_ITEM(CTRL_ECA_TREE, GWTreeCtrl::OnDeleteItem)
EVT_TREE_SET_INFO(CTRL_ECA_TREE, GWTreeCtrl::OnSetInfo)
EVT_TREE_ITEM_EXPANDED(CTRL_ECA_TREE, GWTreeCtrl::OnItemExpanded)
EVT_TREE_ITEM_EXPANDING(CTRL_ECA_TREE, GWTreeCtrl::OnItemExpanding)
EVT_TREE_ITEM_COLLAPSED(CTRL_ECA_TREE, GWTreeCtrl::OnItemCollapsed)
EVT_TREE_ITEM_COLLAPSING(CTRL_ECA_TREE, GWTreeCtrl::OnItemCollapsing)
EVT_TREE_SEL_CHANGING(CTRL_ECA_TREE, GWTreeCtrl::OnSelChanging)
EVT_TREE_KEY_DOWN(CTRL_ECA_TREE, GWTreeCtrl::OnTreeKeyDown)
EVT_TREE_ITEM_ACTIVATED(CTRL_ECA_TREE, GWTreeCtrl::OnItemActivated)
EVT_CONTEXT_MENU(GWTreeCtrl::OnContextMenu)
EVT_TREE_ITEM_MENU(CTRL_ECA_TREE, GWTreeCtrl::OnItemMenu)
EVT_TREE_ITEM_RIGHT_CLICK(CTRL_ECA_TREE, GWTreeCtrl::OnItemRClick)
EVT_RIGHT_DOWN(GWTreeCtrl::OnRMouseDown)
EVT_RIGHT_UP(GWTreeCtrl::OnRMouseUp)
EVT_RIGHT_DCLICK(GWTreeCtrl::OnRMouseDClick)
END_EVENT_TABLE()

GWTreeItemData::~GWTreeItemData()
{
}

// MyTreeCtrl implementation
IMPLEMENT_DYNAMIC_CLASS(GWTreeCtrl, wxTreeCtrl)

GWTreeCtrl::GWTreeCtrl(wxWindow *parent, const wxWindowID id,
					   const wxPoint& pos, const wxSize& size,
					   long style)
					   : wxTreeCtrl(parent, id, pos, size, style)
{
	m_reverseSort = false;

	CreateImageList();

	// Add some items to the tree
	//AddTestItemsToTree(5, 2);
}

void GWTreeCtrl::CreateImageList(int size)
{
	if ( size == -1 )
	{
		SetImageList(NULL);
		return;
	}
	if ( size == 0 )
		size = m_imageSize;
	else
		m_imageSize = size;

	// Make an image list containing small icons
	wxImageList *images = new wxImageList(size, size, true);

	// should correspond to TreeCtrlIcon_xxx enum
	wxBusyCursor wait;
	wxIcon icons[5];
	icons[0] = wxIcon(icon1_xpm);
	icons[1] = wxIcon(icon2_xpm);
	icons[2] = wxIcon(icon3_xpm);
	icons[3] = wxIcon(icon4_xpm);
	icons[4] = wxIcon(icon5_xpm);

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
		{
			images->Add(icons[i]);
		}
		else
		{
			images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
		}
	}

	AssignImageList(images);
}

int GWTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
							   const wxTreeItemId& item2)
{
	if ( m_reverseSort )
	{
		// just exchange 1st and 2nd items
		return wxTreeCtrl::OnCompareItems(item2, item1);
	}
	else
	{
		return wxTreeCtrl::OnCompareItems(item1, item2);
	}
}

void GWTreeCtrl::DoToggleIcon(const wxTreeItemId& item)
{
	int image = (GetItemImage(item) == TreeCtrlIcon_Folder)
		? TreeCtrlIcon_File
		: TreeCtrlIcon_Folder;
	SetItemImage(item, image, wxTreeItemIcon_Normal);

	image = (GetItemImage(item) == TreeCtrlIcon_FolderSelected)
		? TreeCtrlIcon_FileSelected
		: TreeCtrlIcon_FolderSelected;
	SetItemImage(item, image, wxTreeItemIcon_Selected);
}

void GWTreeCtrl::LogEvent(const wxChar *name, const wxTreeEvent& event)
{
	wxTreeItemId item = event.GetItem();
	wxString text;
	if ( item.IsOk() )
		text << _T('"') << GetItemText(item).c_str() << _T('"');
	else
		text = _T("invalid item");
	//wxLogMessage(wxT("%s(%s)"), name, text.c_str());
}

// avoid repetition
#define TREE_EVENT_HANDLER(name)                                 \
	void GWTreeCtrl::name(wxTreeEvent& event)                        \
{                                                                \
	LogEvent(_T(#name), event);                                  \
	SetLastItem(wxTreeItemId());                                 \
	event.Skip();                                                \
}


TREE_EVENT_HANDLER(OnBeginRDrag)
TREE_EVENT_HANDLER(OnDeleteItem)
TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)
//TREE_EVENT_HANDLER(OnSelChanged)
TREE_EVENT_HANDLER(OnSelChanging)

#undef TREE_EVENT_HANDLER

void GWTreeCtrl::OnTreeKeyDown(wxTreeEvent& event)
{
	event.Skip();
}

void GWTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
	event.Veto();
}

void GWTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
	// don't allow anything except letters in the labels
	if ( !event.GetLabel().IsWord() )
	{
		wxMessageBox(wxT("·Ç·¨×Ö·û"));

		event.Veto();
	}
}

void GWTreeCtrl::OnItemCollapsing(wxTreeEvent& event)
{
	////wxLogMessage(wxT("OnItemCollapsing"));

	// for testing, prevent the user from collapsing the first child folder
	//wxTreeItemId itemId = event.GetItem();
	//if ( IsTestItem(itemId) )
	//{
	//	wxMessageBox(wxT("You can't collapse this item."));

	//	event.Veto();
	//}
}

void GWTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
	// show some info about this item
	wxTreeItemId itemId = event.GetItem();
	GWTreeItemData *item = (GWTreeItemData *)GetItemData(itemId);
	if ( IsExpanded( itemId ) )
		Collapse( itemId );
	else 
		Expand( itemId );
	event.Skip();
	//wxLogMessage(wxT("OnItemActivated"));
}

void GWTreeCtrl::OnItemMenu(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeItemData *item = itemId.IsOk() ? (GWTreeItemData *)GetItemData(itemId)
		: NULL;
	wxPoint clientpt = event.GetPoint();
	wxPoint screenpt = ClientToScreen(clientpt);

	//wxLogMessage(wxT("OnItemMenu for item \"%s\" at screen coords (%i, %i)"), item ? item->GetDesc() : _T(""), screenpt.x, screenpt.y);

	ShowMenu(itemId, clientpt);
	event.Skip();
}

void GWTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
{
	wxPoint pt = event.GetPosition();

	//wxLogMessage(wxT("OnContextMenu at screen coords (%i, %i)"), pt.x, pt.y);
}

void GWTreeCtrl::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	return;
}

void GWTreeCtrl::OnItemRClick(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeItemData *item = itemId.IsOk() ? (GWTreeItemData *)GetItemData(itemId) : NULL;
	SelectItem( itemId );

	//wxLogMessage(wxT("Item \"%s\" right clicked"), item ? item->GetDesc(): _T(""));

	event.Skip();
}

void GWTreeCtrl::OnRMouseDown(wxMouseEvent& event)
{
	//wxLogMessage(wxT("Right mouse button down"));

	event.Skip();
}

void GWTreeCtrl::OnRMouseUp(wxMouseEvent& event)
{
	//wxLogMessage(wxT("Right mouse button up"));

	event.Skip();
}

void GWTreeCtrl::OnRMouseDClick(wxMouseEvent& event)
{
	wxTreeItemId id = HitTest(event.GetPosition());

	event.Skip();
}

static inline const wxChar *Bool2String(bool b)
{
	return b ? wxT("") : wxT("not ");
}

