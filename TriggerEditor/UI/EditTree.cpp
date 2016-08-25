#include "EditTree.h"
#include "GWTreeData.h"
#include "wx/image.h"
#include "wx/imaglist.h"
#include "TgrIDs.h"
#include "SNode.h"
#include "StatementDlg.h"
#include "ParamDlg.h"
#include "SNodeVerify.h"
#include "SpaceManager.h"
#include "CmdManager.h"
#include <algorithm>
#include "VarDlg.h"
#include "SelOptionDlg.h"


extern void KickOutChildNodes(vector<Space::SNode*>& Nodes);
extern wxImageList* CreateTriggerTreeImageList();

BEGIN_EVENT_TABLE(EditTree, wxTreeCtrl)

EVT_TREE_BEGIN_DRAG(TGR_TRIGGER_TREE, EditTree::OnBeginDrag)
EVT_TREE_END_DRAG(TGR_TRIGGER_TREE, EditTree::OnEndDrag)
EVT_TREE_BEGIN_LABEL_EDIT(TGR_TRIGGER_TREE, EditTree::OnBeginLabelEdit)
EVT_TREE_END_LABEL_EDIT(TGR_TRIGGER_TREE, EditTree::OnEndLabelEdit)
EVT_TREE_DELETE_ITEM(TGR_TRIGGER_TREE, EditTree::OnDeleteItem)

EVT_TREE_ITEM_ACTIVATED(TGR_TRIGGER_TREE, EditTree::OnItemActivated)

EVT_CONTEXT_MENU(EditTree::OnContextMenu)
EVT_TREE_ITEM_MENU(TGR_TRIGGER_TREE, EditTree::OnItemMenu)
EVT_TREE_ITEM_RIGHT_CLICK(TGR_TRIGGER_TREE, EditTree::OnItemRClick)

EVT_MENU(TGR_TRIGGER_TREE_NEW_EVENT,			EditTree::OnNewEvent)
EVT_MENU(TGR_TRIGGER_TREE_NEW_CONDITION,		EditTree::OnNewCondition)
EVT_MENU(TGR_TRIGGER_TREE_NEW_PARAM, EditTree::OnNewParam)
EVT_MENU(TGR_TRIGGER_TREE_NEW_ACTION, EditTree::OnNewAction)
EVT_MENU(TGR_TRIGGER_TREE_NEW_VARIABLE, EditTree::OnNewVariable)
EVT_MENU(wxID_ANY,								EditTree::OnProcessAny)

EVT_LEFT_DCLICK(EditTree::OnLeftDClick)
EVT_KEY_DOWN(EditTree::OnKeyDown)

END_EVENT_TABLE()



// MyTreeCtrl implementation
#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(EditTree, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(EditTree, wxTreeCtrl)
#endif

EditTree::EditTree(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	:wxTreeCtrl(parent, id, pos, size, style)
	, m_editingNode(NULL)
	, m_pSelNode(NULL)
	, m_selItemId(0)
{
	CreateImageList();
}

void EditTree::CreateImageList(int size)
{
	if ( size == -1 )
	{
		SetImageList(NULL);
		return;
	}
	wxImageList *images = CreateTriggerTreeImageList();
	AssignImageList(images);
}

void EditTree::OnDeleteItem(wxTreeEvent& event)
{
	event.Skip();
}

void EditTree::OnBeginLabelEdit(wxTreeEvent& event)
{
	event.Veto();
}

void EditTree::OnEndLabelEdit(wxTreeEvent& event)
{
	// don't allow anything except letters in the labels
	if ( !event.GetLabel().IsWord() )
	{
		wxMessageBox(wxT("非法字符"));

		event.Veto();
	}
}

void EditTree::OnItemActivated(wxTreeEvent& event)
{
	m_selItemId = event.GetItem();
	Space::SNode* pOpNode = GetSpaceNode(m_selItemId);
	if (pOpNode == NULL) return;

	bool bUpdateWidgets = false;
	TEE::NodeBase* pTEETmp = pOpNode->TEETmp();
	if (pOpNode->ClassType() == Space::SNT_VARIABLE)
	{
		VarEditDlg dlg(this, pOpNode, pOpNode->Parent());
		if (dlg.EditShowModal())
		{
			bUpdateWidgets = true;
		}
	}
	else if (pOpNode->IsA(Space::SNT_RETURNTYPE))
	{
		SelOptionDlg dlg(this, pOpNode);
		if (dlg.EditShowModal())
		{
			bUpdateWidgets = true;
			if (Space::SNode* pFun = pOpNode->Ancestor(Space::SNT_FUNCTION))
			{
				Space::SNode::CheckErrorTree(pFun);
				Space::UpdateNodeWidgets(pFun, true);

				UpdateReferenceNodes(pFun);
			}
		}
	}
	else if (pTEETmp)
	{
		Space::SPACE_NODE_TYPE eCreationType = pOpNode->ClassType();
		TEE::NODE_TYPE eTEEType = pTEETmp->ClassType();
		if (eCreationType == Space::SNT_SFUNCCALL)
			eTEEType = TEE::NT_FUNCTION;
		StatementDlg dlg(this, pOpNode, eCreationType, eTEEType);
		dlg.DoShowModal();
		bUpdateWidgets = true;
	}
	else
	{
		if (IsExpanded(m_selItemId))
			Collapse(m_selItemId);
		else
			Expand(m_selItemId);
	}

	if (bUpdateWidgets)
	{
		Space::SNode* pOpNode = GetSpaceNode(m_selItemId);
		Space::UpdateNodeWidgets(pOpNode, true);
	}
}

void EditTree::OnItemMenu(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeData *item = itemId.IsOk() ? (GWTreeData *)GetItemData(itemId)
		: NULL;
	wxPoint clientpt = event.GetPoint();
	wxPoint screenpt = ClientToScreen(clientpt);

	//wxLogMessage(wxT("OnItemMenu for item \"%s\" at screen coords (%i, %i)"), item ? item->GetDesc() : _T(""), screenpt.x, screenpt.y);

	ShowMenu(itemId, clientpt);
	event.Skip();
}

void EditTree::OnContextMenu(wxContextMenuEvent& event)
{
	wxPoint pt = event.GetPosition();

	//wxLogMessage(wxT("OnContextMenu at screen coords (%i, %i)"), pt.x, pt.y);
}

void EditTree::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	Space::SNode* pNode = GetSpaceNode( id );
	if( pNode == NULL )
		return;
	bool bRoot = (id == GetRootItem());

	wxMenu menu;
	menu.Append( TGR_TRIGGER_TREE_CUT,				wxT("剪切(&C)") );
	menu.Enable( TGR_TRIGGER_TREE_CUT,				!bRoot && pNode->Cutable() );
	menu.Append( TGR_TRIGGER_TREE_COPY,				wxT("复制(&C)") );
	menu.Enable( TGR_TRIGGER_TREE_COPY,				!bRoot && pNode->Copyable() );
	menu.Append( TGR_TRIGGER_TREE_PASTE,			wxT("粘贴(&P)") );
	menu.Enable( TGR_TRIGGER_TREE_PASTE,			true );
	menu.Append( TGR_TRIGGER_TREE_DELETE,			wxT("删除(&D)") );
	menu.Enable( TGR_TRIGGER_TREE_DELETE,			!bRoot && pNode->Deleteable() );

	menu.AppendSeparator();
	if (pNode->NewActionable())
		menu.Append(TGR_TRIGGER_TREE_NEW_ACTION, wxT("新动作(&A)"));
	if (pNode->NewActionable())
		menu.Append(TGR_TRIGGER_TREE_NEW_VARIABLE, wxT("新变量(&R)"));
	if (pNode->IsA(Space::SNT_TRIGGER) || pNode->IsA(Space::SNT_EVENTROOT) || pNode->IsA(Space::SNT_EVENT))
		menu.Append(TGR_TRIGGER_TREE_NEW_EVENT, wxT("新事件(&V)"));
	if (pNode->IsA(Space::SNT_TRIGGER) || pNode->IsA(Space::SNT_CONDITIONROOT) || pNode->IsA(Space::SNT_CONDITION) || pNode->IsA(Space::SNT_IF))
		menu.Append(TGR_TRIGGER_TREE_NEW_CONDITION, wxT("新条件(&N)"));
	if (pNode->IsA(Space::SNT_FUNCTION) || pNode->IsA(Space::SNT_PARAMROOT) || pNode->IsA(Space::SNT_VARIABLE) && pNode->Parent()->IsA(Space::SNT_PARAMROOT))
		menu.Append(TGR_TRIGGER_TREE_NEW_PARAM, wxT("新参数(&P)"));

	menu.AppendSeparator();
	if (pNode->Disableable())
		menu.Append(TGR_TRIGGER_TREE_PERMIT, pNode->Working() ? wxT("禁用(&S)") : wxT("启用(&G)"));

	PopupMenu(&menu, pt);

	return;
}
void EditTree::OnItemRClick(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeData *item = itemId.IsOk() ? (GWTreeData *)GetItemData(itemId) : NULL;
	SelectItem( itemId );

	event.Skip();
}

void	EditTree::OnBeginDrag( wxTreeEvent& event )
{
	m_draggingItem = nullptr;
	Space::SNode* pNode = GetSpaceNode( event.GetItem() );
	if (pNode == nullptr) return;
	if ( IsActionType(pNode->ClassType()) ||
		pNode->IsA(Space::SNT_VARIABLE) ||
		pNode->IsA(Space::SNT_CONDITION) ||
		pNode->IsA(Space::SNT_EVENT))
	{
		m_draggingItem = event.GetItem();
		event.Allow();
	}
}

void	EditTree::OnEndDrag(wxTreeEvent& event)
{
	wxTreeItemId itemSrc = m_draggingItem;
	wxTreeItemId itemDst = event.GetItem();
	Space::SNode* pSrcNode = GetSpaceNode(itemSrc);
	Space::SNode* pDestNode = GetSpaceNode(itemDst);

	m_draggingItem = nullptr;
	if (pSrcNode == nullptr || pDestNode == nullptr)
		return;

	vector<Space::SNode*> draggingNodes;
	draggingNodes.push_back(pSrcNode);
	PasteNodes(draggingNodes, pDestNode, PST_DRAG);
}

void	EditTree::UpdateWidgets()
{
	this->DeleteAllItems();
	if(m_editingNode == NULL )
		return;

	wxTreeItemId rootId = AddRoot(m_editingNode->GetReadText(), TriggerIcon_Folder, TriggerIcon_Folder, new GWTreeData(m_editingNode) );
	SetItemImage(rootId, TriggerIcon_FolderOpen, wxTreeItemIcon_Expanded);

	for(Space::SNode* pRoot = m_editingNode->FirstChild(); pRoot; pRoot = pRoot->Next() )
	{
		wxTreeItemId parentId = CreateNodeWidgets(pRoot, rootId, 0, this, false);
		for(Space::SNode* pChild = pRoot->FirstChild(); pChild ; pChild = pChild->Next() )
		{
			CreateNodeWidgets(pChild, parentId, 0, this);
		}
	}
}

void	EditTree::OnSelectGalleryNode( Space::SNode* pNode )
{
	m_editingNode = NULL;
	m_pSelNode = NULL;
	if( pNode && (pNode->IsA(Space::SNT_TRIGGER) || pNode->IsA(Space::SNT_FUNCTION)) )
	{
		m_editingNode = pNode;
	}

	UpdateWidgets();
	ExpandAll();
}

void	EditTree::OnSelectNode( wxTreeItemId itemId, Space::SNode* pNode )
{
	m_selItemId = itemId;
	m_pSelNode = pNode;
}

Space::SNode*	EditTree::GetSpaceNode( const wxTreeItemId &id )
{
	if( !id.IsOk() )
		return NULL;

	GWTreeData *pData = (GWTreeData*)GetItemData( id );
	if( pData == NULL )
		return NULL;

	return pData->UserData();
}

void EditTree::SelectNode(Space::SNode* pNode)
{
	if (pNode == nullptr)
	{
		UnselectAll();
		return;
	}
	while (pNode)
	{
		wxTreeItemId srcId = pNode->UserData("itemid");
		wxTreeCtrl* pTree = reinterpret_cast<wxTreeCtrl*>(pNode->UserData("tree"));
		if (!srcId.IsOk() || pTree != this)
		{
			pNode = pNode->Parent();
			continue;
		}
		UnselectAll();
		SelectItem(srcId);
		break;
	}
}

void	EditTree::OnNewEvent(wxCommandEvent& event)
{
	OnNewCreation( Space::SNT_EVENT, TEE::NT_EVENT );
}

void	EditTree::OnNewCondition(wxCommandEvent& event)
{
	OnNewCreation( Space::SNT_CONDITION, TEE::NT_CONDITION );
}

void	EditTree::OnNewAction(wxCommandEvent& event)
{
	OnNewCreation(Space::SNT_TFUNCCALL, TEE::NT_FUNCTION);
}

void	EditTree::OnNewVariable(wxCommandEvent& event)
{
	Space::SNode* pActionRoot = m_editingNode->FindFirstChild(Space::SVerifyNodeType(Space::SNT_ACTIONROOT));
	VarEditDlg dlg(this, NULL, pActionRoot);
	Space::SNode* pNewVar = dlg.EditShowModal();
	if (pNewVar)
	{
		assert(pActionRoot);
		Space::SNode* lastVar = pActionRoot->FindLastChild(Space::SVerifyNodeType(Space::SNT_VARIABLE));
		GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd(pNewVar, pActionRoot, lastVar));
	}
}

void	EditTree::OnNewParam(wxCommandEvent& event)
{
	Space::SNode* pParamRoot = m_editingNode->FindFirstChild(Space::SVerifyNodeType(Space::SNT_PARAMROOT));
	VarEditDlg dlg(this, NULL, pParamRoot);
	Space::SNode* pNewVar = dlg.EditShowModal();
	if (pNewVar)
	{
		assert(pParamRoot);
		Space::SNode* lastVar = pParamRoot->FindLastChild(Space::SVerifyNodeType(Space::SNT_VARIABLE));
		GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd(pNewVar, pParamRoot, lastVar));
	}
}

bool	EditTree::GetCreationLocality( Space::SNode* pNewNode, Space::SNode*& pParent, Space::SNode*& pPrev )
{
	if( pNewNode == NULL )
	{
		return false;
	}
	
	Space::SNode* selNode = m_pSelNode;
	Space::SNode* selParent = selNode ? selNode->Parent() : nullptr;
	if ( IsActionType(pNewNode->ClassType()) )
	{
		if(selNode->IsA(Space::SNT_LOOP) ||
			selNode->IsA(Space::SNT_THEN) ||
			selNode->IsA(Space::SNT_ELSE))
		{
			pParent = selNode;
			pPrev = pParent ? pParent->LastChild() : NULL;
		}
		else if (selParent->IsA(Space::SNT_LOOP) ||
			selParent->IsA(Space::SNT_THEN) ||
			selParent->IsA(Space::SNT_ELSE))
		{
			pParent = selParent;
			pPrev = selNode;
		}
	}
	else if (pNewNode->IsA(Space::SNT_CONDITION))
	{
		if (selNode->IsA(Space::SNT_IF))
		{
			pParent = selNode;
			pPrev = pParent ? pParent->LastChild() : NULL;
		}
		else if (selParent->IsA(Space::SNT_IF))
		{
			pParent = selNode->Parent();
			pPrev = selNode;
		}
	}

	if( pParent == NULL )
	{
		Space::SVerifyNodeType vNodeType(pNewNode->CommonParentType());
		pParent = m_editingNode ? m_editingNode->FindFirstChild(vNodeType) : NULL;
		assert( pParent != NULL );
		if( pParent == NULL )
		{
			return false;
		}

		if (selNode != NULL &&
			(IsActionType(selNode->ClassType()) && IsActionType(pNewNode->ClassType()) || 
				selNode->ClassType() == pNewNode->ClassType()))
			pPrev = selNode;
		else
			pPrev = pParent->LastChild();
	}
	
	return true;
}
void	EditTree::OnNewCreation( Space::SPACE_NODE_TYPE eCreationType, TEE::NODE_TYPE eTEEType )
{
	StatementDlg dlg(this, NULL, eCreationType, eTEEType );
	Space::SNode* pNewNode = dlg.DoShowModal();
	if( pNewNode != NULL )
	{
		Space::SNode* pParent = NULL;
		Space::SNode* pPrev = NULL;
		if( GetCreationLocality( pNewNode, pParent, pPrev ) )
		{
			GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd( pNewNode, pParent, pPrev ));
		}
		else
		{
			SAFE_DELETE(pNewNode);
			assert(false);
		}
	}
}
void	EditTree::OnCut()
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) > 0)
	{
		vector<Space::SNode*> Nodes;
		for (UINT Index = 0; Index < Selections.size(); ++Index)
		{
			Space::SNode* pNode = GetSpaceNode( Selections[Index] );
			if( pNode && pNode->Cutable() )
			{
				Nodes.push_back(pNode);
			}
		}
		GCmdMgr->Cut( Nodes );
	}
}
void	EditTree::OnCopy()
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) > 0)
	{
		vector<Space::SNode*> Nodes;
		for (UINT Index = 0; Index < Selections.size(); ++Index)
		{
			Space::SNode* pNode = GetSpaceNode( Selections[Index] );
			if( pNode && pNode->Copyable() )
			{
				Nodes.push_back(pNode);
			}
		}
		GCmdMgr->Copy( Nodes );
	}
}

intptr_t GetDocumentIDFromNode(const Space::SNode* pNode)
{
	return pNode ? (intptr_t)pNode : (intptr_t)-1;
}

intptr_t EditTree::GetDocumentID()
{
	Space::SNode* editingNode = GetEditingNode();
	return GetDocumentIDFromNode(editingNode);
}
void	EditTree::OnOperation()
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) > 0)
	{
		vector<Space::SNode*> Nodes;
		for (UINT Index = 0; Index < Selections.size(); ++Index)
		{
			Space::SNode* pNode = GetSpaceNode( Selections[Index] );
			if( pNode && pNode->Copyable() )
			{
				Nodes.push_back(pNode);
			}
		}
		KickOutChildNodes(Nodes);

		vector<Space::ICommand*> Commands;
		for (vector<Space::SNode*>::const_reverse_iterator ritr = Nodes.rbegin(); ritr != Nodes.rend(); ++ritr)
		{
			Space::SNode* pNode = *ritr;
			Commands.push_back( new Space::ExChangeOperateNodeCmd( pNode ) );
		}
		GCmdMgr->AddCommand(GetDocumentID(), new Space::MacroCmd(Commands));
	}
}

bool	EditTree::OnPaste()
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) == 0)
		return false;

	Space::SNode* pSelNode = GetSpaceNode( Selections.Last() );
	if (pSelNode == NULL)
		return false;

	bool isCutting = true;
	vector<Space::SNode*> CuttingNodes = GCmdMgr->GetCuttingNodes();
	vector<Space::SNode*> CopyingNodes;
	if (CuttingNodes.empty())
	{
		CopyingNodes = GCmdMgr->CreateNewPastingNodes();
		isCutting = false;
	}

	//单独粘贴变量
	vector<Space::SNode*>& pastingNodes = isCutting ? CuttingNodes : CopyingNodes;
	return PasteNodes(pastingNodes, pSelNode, isCutting ? PST_CUT : PST_COPY);
}

bool	EditTree::PasteNodes(vector<Space::SNode*>& pastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType)
{
	bool isVarPasteSuccess = true;
	bool isOtherPasteSuccess = true;
	vector<Space::SNode*> varItems;
	for (int index = 0; index < (int)pastingNodes.size(); ++index)
	{
		Space::SNode* pNode = pastingNodes[index];
		if (pNode && pNode->IsA(Space::SNT_VARIABLE))
		{
			varItems.push_back(pNode);
			pastingNodes.erase(pastingNodes.begin() + index--);
		}
	}
	bool shouldDeleteOnFail = srcType == PST_COPY;
	if (varItems.size() > 0)
	{
		Space::SNode* targetNode = (pSelNode->IsA(Space::SNT_VARIABLE) || pSelNode->IsA(Space::SNT_ACTIONROOT)) ? pSelNode : pSelNode->Ancestor(Space::SNT_ACTIONROOT);
		isVarPasteSuccess = DoPasteNodes(varItems, targetNode, srcType);
		if (!isVarPasteSuccess && shouldDeleteOnFail)
		{
			for (UINT i = 0; i < varItems.size(); ++i)
			{
				SAFE_DELETE(varItems[i]);
			}
			varItems.clear();
		}
		Space::SNode::CheckErrorTree(m_editingNode);
	}

	isOtherPasteSuccess = DoPasteNodes(pastingNodes, pSelNode, srcType);
	if (!isOtherPasteSuccess && shouldDeleteOnFail)
	{
		for (UINT i = 0; i < pastingNodes.size(); ++i)
		{
			SAFE_DELETE(pastingNodes[i]);
		}
		pastingNodes.clear();
	}
	return isVarPasteSuccess || isOtherPasteSuccess;
}

bool	EditTree::DoPasteNodes(const vector<Space::SNode*>& pastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType)
{
	if (pastingNodes.size() == 0 || pSelNode == nullptr)
		return false;
	for (UINT Index = 0; Index < pastingNodes.size(); ++Index)
	{
		if (!pSelNode->Pasteable(pastingNodes[Index]))
		{
			return false;
		}
	}

	//查询应该粘贴在哪里
	Space::SNode* pParent = NULL;
	Space::SNode* pPrev = NULL;
	if (IsActionType(pastingNodes.front()->ClassType()))
	{
		if (pSelNode->ClassType() == Space::SNT_LOOP ||
			pSelNode->ClassType() == Space::SNT_THEN ||
			pSelNode->ClassType() == Space::SNT_ELSE)
		{
			pParent = pSelNode;
		}
		else if (IsActionType(pSelNode->ClassType()))
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
		else if (pSelNode->Parent() &&
			(pSelNode->Parent()->ClassType() == Space::SNT_LOOP ||
				pSelNode->Parent()->ClassType() == Space::SNT_THEN ||
				pSelNode->Parent()->ClassType() == Space::SNT_ELSE))
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
	}
	else if (pastingNodes.front()->ClassType() == Space::SNT_CONDITION)
	{
		if (pSelNode->ClassType() == Space::SNT_IF)
		{
			pParent = pSelNode;
		}
		else if (pSelNode->Parent() != NULL && (pSelNode->Parent()->ClassType() == Space::SNT_IF))
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
	}
	else if (pastingNodes.front()->ClassType() == Space::SNT_VARIABLE)
	{
		if (pSelNode->IsA(Space::SNT_VARIABLE))
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
		else
		{
			pParent = pSelNode;
			pPrev = pSelNode->FindLastChild(Space::SVerifyNodeType(Space::SNT_VARIABLE));
		}
	}

	if (pParent == NULL)
	{
		Space::SVerifyNodeType typeVerify(pastingNodes.front()->CommonParentType());
		pParent = m_editingNode ? m_editingNode->FindFirstChild(typeVerify) : NULL;
		if (pParent == NULL)
		{
			pParent = m_editingNode->Ancestor(typeVerify.eType);
		}
		pPrev = pSelNode->Parent() == pParent && pSelNode->ClassType() == pastingNodes.front()->ClassType() ? pSelNode : pParent->LastChild();
	}

	if (srcType == PST_CUT || srcType == PST_DRAG)
	{
		bool bIsTargetBeenCutting = std::find(pastingNodes.begin(), pastingNodes.end(), pPrev) != pastingNodes.end();
		for (Space::SNode* pNode = pParent; pNode != NULL && !bIsTargetBeenCutting; pNode = pNode->Parent())
		{
			bIsTargetBeenCutting |= std::find(pastingNodes.begin(), pastingNodes.end(), pNode) != pastingNodes.end();
		}
		if (!bIsTargetBeenCutting)
		{
			vector<Space::ICommand*> Commands;
			for (vector<Space::SNode*>::const_reverse_iterator ritr = pastingNodes.rbegin(); ritr != pastingNodes.rend(); ++ritr)
			{
				Space::SNode* pCuttingNode = *ritr;
				Commands.push_back(new Space::PasteNodeCmd(pCuttingNode, pParent, pPrev, true, pCuttingNode->Parent(), pCuttingNode->Prev()));
			}
			GCmdMgr->AddCommand(GetDocumentID(), new Space::MacroCmd(Commands));
			GCmdMgr->OnPaste();
		}
	}
	else
	{
		for (vector<Space::SNode*>::const_iterator itr = pastingNodes.begin(); itr != pastingNodes.end(); ++itr)
		{
			Space::SNode* pCopyingNode = *itr;
			wstring name = pCopyingNode->Name();
			Space::GSpaceMgr->UniqueName(pCopyingNode);
		}

		vector<Space::ICommand*> Commands;
		for (vector<Space::SNode*>::const_reverse_iterator ritr = pastingNodes.rbegin(); ritr != pastingNodes.rend(); ++ritr)
		{
			Space::SNode* pCopyingNode = *ritr;
			Commands.push_back(new Space::PasteNodeCmd(pCopyingNode, pParent, pPrev, false, NULL, NULL));
		}
		GCmdMgr->AddCommand(GetDocumentID(), new Space::MacroCmd(Commands));
		GCmdMgr->OnPaste();
	}

	return true;
}

bool	EditTree::OnDelete()
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) > 0)
	{
		vector<Space::SNode*> Nodes;
		for (UINT Index = 0; Index < Selections.size(); ++Index)
		{
			Space::SNode* pNode = GetSpaceNode( Selections[Index] );
			if( pNode && pNode->Deleteable() )
			{
				Nodes.push_back(pNode);
			}
		}
		KickOutChildNodes(Nodes);

		vector<Space::ICommand*> Commands;
		for (vector<Space::SNode*>::const_reverse_iterator ritr = Nodes.rbegin(); ritr != Nodes.rend(); ++ritr)
		{
			Space::SNode* pDeletingNode = *ritr;
			Commands.push_back( new Space::DeleteNodeCmd( pDeletingNode ) );
		}
		GCmdMgr->AddCommand(GetDocumentID(), new Space::MacroCmd(Commands));
	}
	return true;
}
void	EditTree::OnProcessAny(wxCommandEvent& event)
{
	switch( event.GetId() )
	{
	case TGR_TRIGGER_TREE_CUT:
		OnCut();
		break;
	case TGR_TRIGGER_TREE_COPY:
		OnCopy();
		break;
	case TGR_TRIGGER_TREE_PASTE:
		OnPaste();
		break;
	case TGR_TRIGGER_TREE_DELETE:
		OnDelete();
		break;
	case TGR_TRIGGER_TREE_PERMIT:
		OnOperation();
	default:
		break;
	}
}

void EditTree::OnLeftDClick(wxMouseEvent& event)
{
	wxTreeItemId hitItem = HitTest(event.GetPosition());
	if (hitItem.IsOk())
	{
		event.Skip();
		return;
	}
	if (m_editingNode == nullptr)
		return;
	SelectNode(nullptr);
	OnNewAction(wxCommandEvent());
}

void EditTree::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
	if (m_editingNode == nullptr)
		return;

	long keycode = event.GetKeyCode();
	if (keycode == '=' && event.ControlDown())
	{
		if (m_pSelNode->IsA(Space::SNT_EVENTROOT) ||
			m_pSelNode->IsA(Space::SNT_EVENT))
			OnNewEvent(wxCommandEvent());
		else if (m_pSelNode->IsA(Space::SNT_CONDITIONROOT) ||
			m_pSelNode->IsA(Space::SNT_CONDITION) ||
			m_pSelNode->IsA(Space::SNT_IF))
			OnNewCondition(wxCommandEvent());
		else
			OnNewCreation(Space::SNT_TFUNCCALL, TEE::NT_FUNCTION);
	}
}