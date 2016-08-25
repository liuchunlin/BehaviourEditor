#pragma once

#define USE_GENERIC_TREECTRL 0
#include "UICommon.h"
#include "wx/treectrl.h"
#include <map>
#include "CancelableWindow.h"

using namespace std;
namespace Space
{
	class SNode;
	enum SPACE_NODE_TYPE;
}
namespace TEE
{
	enum NODE_TYPE;
}

class EditTree : public wxTreeCtrl, public CancelableWindow
{
public:
	EditTree() { }
	EditTree( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style );
	virtual ~EditTree(){};

	virtual intptr_t GetDocumentID();

	//ÊÂ¼þ
	void OnBeginDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnDeleteItem(wxTreeEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnItemMenu(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnItemRClick(wxTreeEvent& event);
	void OnProcessAny(wxCommandEvent& event);
	void OnLeftDClick(wxMouseEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	void CreateImageList(int size = 16);
	void ShowMenu(wxTreeItemId id, const wxPoint& pt);

	void				OnNewEvent(wxCommandEvent& event);
	void				OnNewCondition(wxCommandEvent& event);
	void				OnNewAction(wxCommandEvent& event);
	void				OnNewVariable(wxCommandEvent& event);
	void				OnNewParam(wxCommandEvent& event);

	void				OnSelectGalleryNode( Space::SNode* pNode );
	void				OnSelectNode( wxTreeItemId itemId, Space::SNode* pNode );
	Space::SNode*		GetSpaceNode( const wxTreeItemId &id );
	void				OnCut();
	void				OnCopy();
	void				OnOperation();
	bool				OnPaste();
	bool				OnDelete();
	Space::SNode*		GetEditingNode(){ return m_editingNode; }
	void				UpdateWidgets();
	Space::SNode*		GetSelectedNode() { return m_pSelNode; }
	void				SelectNode(Space::SNode* pNode);
protected:
	//Âß¼­
	void				OnNewCreation(Space::SPACE_NODE_TYPE eCreationType, TEE::NODE_TYPE eTEEType);
	bool				GetCreationLocality( Space::SNode* pNewNode, Space::SNode*& pParent, Space::SNode*& pPrev );
	bool				PasteNodes(vector<Space::SNode*>& pastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType);
	bool				DoPasteNodes(const vector<Space::SNode*>& pastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType);
private:
	DECLARE_DYNAMIC_CLASS(EditTree)
	DECLARE_EVENT_TABLE()
private:
	Space::SNode*				m_editingNode;

	//Select
	Space::SNode				*m_pSelNode;
	wxTreeItemId				m_selItemId;
	wxTreeItemId				m_draggingItem;
};

