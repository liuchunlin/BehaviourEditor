#pragma once

#define USE_GENERIC_TREECTRL 0
#include "UICommon.h"
#include "wx/treectrl.h"
#include "CancelableWindow.h"

class GalleryTree : public wxTreeCtrl, public CancelableWindow
{
public:
	GalleryTree() { }
	GalleryTree( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style );
	virtual ~GalleryTree(){};

	virtual intptr_t GetDocumentID() { return 0; }

	void OnBeginDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnDeleteItem(wxTreeEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnItemMenu(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnItemRClick(wxTreeEvent& event);

	void CreateImageList(int size = 16);
	void ShowMenu(wxTreeItemId id, const wxPoint& pt);

	void				OnProcessAny(wxCommandEvent& event);
	void				OnDelete();
	void				OnNewGroup();
	void				OnNewTrigger();
	void				OnNewFunction();
	void				OnCut();
	void				OnCopy();
	void				OnOperation();
	bool				OnPaste();

	void				UpdateNewRoot( Space::SNode* pRoot );
	Space::SNode*		GetSpaceNode( const wxTreeItemId &id );
	Space::SNode*		GetSelectedNode(bool bLastOne = false);
	void				SelectNode(Space::SNode* pNode);
protected:
	bool PasteNodes(vector<Space::SNode*>& PastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType);

private:
	DECLARE_DYNAMIC_CLASS(GalleryTree)
	DECLARE_EVENT_TABLE()
	wxTreeItemId		m_draggingItem;
};

