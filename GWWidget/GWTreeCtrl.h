#pragma once
#include "wx/treectrl.h"

namespace TEE
{
	class NodeBase;
}

class GWTreeItemData : public wxTreeItemData
{
public:
	GWTreeItemData(const wxString& desc, TEE::NodeBase *pNode ) : m_desc(desc), m_pNode(pNode) { }
	~GWTreeItemData();
	const wxChar *GetDesc() const { return m_desc.c_str(); }

	TEE::NodeBase			*m_pNode;
	wxString				m_desc;
};

class GWTreeCtrl : public wxTreeCtrl
{
public:
	enum
	{
		TreeCtrlIcon_File,
		TreeCtrlIcon_FileSelected,
		TreeCtrlIcon_Folder,
		TreeCtrlIcon_FolderSelected,
		TreeCtrlIcon_FolderOpened
	};

	GWTreeCtrl() { }
	GWTreeCtrl(wxWindow *parent, const wxWindowID id,
		const wxPoint& pos, const wxSize& size,
		long style);
	virtual ~GWTreeCtrl(){};

	void OnBeginRDrag(wxTreeEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnDeleteItem(wxTreeEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnItemMenu(wxTreeEvent& event);
	void OnGetInfo(wxTreeEvent& event);
	void OnSetInfo(wxTreeEvent& event);
	void OnItemExpanded(wxTreeEvent& event);
	void OnItemExpanding(wxTreeEvent& event);
	void OnItemCollapsed(wxTreeEvent& event);
	void OnItemCollapsing(wxTreeEvent& event);

	void OnSelChanging(wxTreeEvent& event);
	void OnTreeKeyDown(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnItemRClick(wxTreeEvent& event);

	void OnRMouseDown(wxMouseEvent& event);
	void OnRMouseUp(wxMouseEvent& event);
	void OnRMouseDClick(wxMouseEvent& event);

	void CreateImageList(int size = 16);

	void DoSortChildren(const wxTreeItemId& item, bool reverse = false)
	{ m_reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
	void DoEnsureVisible() { if (m_lastItem.IsOk()) EnsureVisible(m_lastItem); }

	void DoToggleIcon(const wxTreeItemId& item);

	void ShowMenu(wxTreeItemId id, const wxPoint& pt);

	int ImageSize(void) const { return m_imageSize; }

	void SetLastItem(wxTreeItemId id) { m_lastItem = id; }

protected:
	virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

	// is this the test item which we use in several event handlers?
	bool IsTestItem(const wxTreeItemId& item)
	{
		// the test item is the first child folder
		return GetItemParent(item) == GetRootItem() && !GetPrevSibling(item);
	}

private:
	void LogEvent(const wxChar *name, const wxTreeEvent& event);

	int          m_imageSize;               // current size of images
	bool         m_reverseSort;             // flag for OnCompareItems
	wxTreeItemId m_lastItem,                // for OnEnsureVisible()
		m_draggedItem;             // item being dragged right now

	// NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
	//     if you want your overloaded OnCompareItems() to be called.
	//     OTOH, if you don't want it you may omit the next line - this will
	//     make default (alphabetical) sorting much faster under wxMSW.
	DECLARE_DYNAMIC_CLASS(GWTreeCtrl)
	DECLARE_EVENT_TABLE()
};

