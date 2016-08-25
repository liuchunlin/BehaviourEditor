#include "GalleryTree.h"
#include "CmdManager.h"
#include "GWTreeData.h"
#include "file.xpm"
#include "folder.xpm"
#include "folderopen.xpm"
#include "action.xpm"
#include "event.xpm"
#include "condition.xpm"
#include "variable.xpm"
#include "branch.xpm"
#include "loop.xpm"
#include "return.xpm"
#include "trigger.xpm"
#include "function.xpm"
#include "wx/image.h"
#include "wx/imaglist.h"
#include "TgrIDs.h"
#include "CmdManager.h"
#include "SpaceManager.h"
#include "SNode.h"
#include "SFunction.h"
#include "LuaCode.h"
#include <algorithm>

extern void KickOutChildNodes(vector<Space::SNode*>& Nodes);

wxImageList* CreateTriggerTreeImageList()
{
	const int kImageSize = 16;

	// should correspond to ETriggerIcon enum
	wxIcon icons[] =
	{
		wxIcon(file_xpm),
		wxIcon(folder_xpm),
		wxIcon(folderopen_xpm),
		wxIcon(action_xpm),
		wxIcon(event_xpm),
		wxIcon(condition_xpm),
		wxIcon(variable_xpm),
		wxIcon(branch_xpm),
		wxIcon(loop_xpm),
		wxIcon(return_xpm),
		wxIcon(trigger_xpm),
		wxIcon(function_xpm)
	};

	wxImageList* imageList = new wxImageList(kImageSize, kImageSize, true);
	for (size_t i = 0; i < WXSIZEOF(icons); i++)
	{
		if (icons[i].GetWidth() == kImageSize)
		{
			imageList->Add(icons[i]);
		}
		else
		{
			imageList->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(kImageSize, kImageSize)));
		}
	}
	return imageList;
}

//-------------------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(GalleryTree, wxTreeCtrl)

BEGIN_EVENT_TABLE(GalleryTree, wxTreeCtrl)
EVT_TREE_BEGIN_DRAG					(TGR_CLASSIFY_TREE, GalleryTree::OnBeginDrag)
EVT_TREE_END_DRAG					(TGR_CLASSIFY_TREE, GalleryTree::OnEndDrag)
EVT_TREE_BEGIN_LABEL_EDIT			(TGR_CLASSIFY_TREE, GalleryTree::OnBeginLabelEdit)
EVT_TREE_END_LABEL_EDIT				(TGR_CLASSIFY_TREE, GalleryTree::OnEndLabelEdit)
EVT_TREE_ITEM_ACTIVATED				(TGR_CLASSIFY_TREE, GalleryTree::OnItemActivated)
EVT_TREE_ITEM_MENU					(TGR_CLASSIFY_TREE, GalleryTree::OnItemMenu)
EVT_TREE_ITEM_RIGHT_CLICK			(TGR_CLASSIFY_TREE, GalleryTree::OnItemRClick)
EVT_MENU							(wxID_ANY,			GalleryTree::OnProcessAny)
EVT_CONTEXT_MENU(GalleryTree::OnContextMenu)
END_EVENT_TABLE()

GalleryTree::GalleryTree(wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:wxTreeCtrl(parent, id, pos, size, style)
{
	CreateImageList();
}

void GalleryTree::CreateImageList(int size)
{
	if ( size == -1 )
	{
		SetImageList(NULL);
		return;
	}
	wxImageList *images = CreateTriggerTreeImageList();
	AssignImageList(images);
}

void GalleryTree::OnBeginLabelEdit(wxTreeEvent& event)
{
	if( event.GetItem() == GetRootItem() )
	{
		event.Veto();
	}
}

void GalleryTree::OnEndLabelEdit(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	Space::SNode* pSrc = GetSpaceNode(itemId);
	if(pSrc == nullptr || event.GetLabel() == wxEmptyString )
	{
		event.Veto();
	}
	else
	{
		wxString strNewName = event.GetLabel();
		if (pSrc->IsA(Space::SNT_FUNCTION))
		{
			size_t sepPos = strNewName.find(FunctionNameSeperator.c_str());
			wxString funcName = strNewName.Mid(0, sepPos);
			wxString funcDesc = sepPos != wxString::npos ? strNewName.Mid(sepPos + FunctionNameSeperator.length()) : TEXT("");
			if (Space::GSpaceMgr->CheckNameValid(pSrc, funcName.wc_str()))
			{
				GCmdMgr->AddCommand(GetDocumentID(), new Space::RenameFunctionCmd(pSrc, funcName.wc_str(), funcDesc.wc_str()));
			}
			else
			{
				wxMessageBox(wxT("名字格式错误 或者 重名！"));
				event.Veto();
			}
		}
		else
		{
			if (Space::GSpaceMgr->CheckNameValid(pSrc, strNewName.wc_str()))
			{
				GCmdMgr->AddCommand(GetDocumentID(), new Space::RenameNodeCmd(pSrc, strNewName.wc_str()));
			}
			else
			{
				wxMessageBox(wxT("名字格式错误 或者 重名！"));
				event.Veto();
			}
		}
	}
}

void GalleryTree::OnItemActivated(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeData *item = (GWTreeData *)GetItemData(itemId);
	if ( IsExpanded( itemId ) )
		Collapse( itemId );
	else 
		Expand( itemId );

	//event.Skip();
}

void GalleryTree::OnItemMenu(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	wxPoint clientpt = event.GetPoint();
	wxPoint screenpt = ClientToScreen(clientpt);

	ShowMenu(itemId, clientpt);
	event.Skip();
}

void GalleryTree::OnContextMenu(wxContextMenuEvent& event)
{
	wxPoint pt = event.GetPosition();
}

void GalleryTree::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	Space::SNode* pNode = GetSpaceNode( id );
	if( pNode == NULL )
		return;

	wxMenu menu;
	menu.Append( TGR_CLASSIFY_TREE_CUT,				wxT("剪切(&C)") );
	menu.Enable( TGR_CLASSIFY_TREE_CUT,				pNode->Cutable() );
	menu.Append( TGR_CLASSIFY_TREE_COPY,			wxT("复制(&C)") );
	menu.Enable( TGR_CLASSIFY_TREE_COPY,			pNode->Copyable() );
	menu.Append( TGR_CLASSIFY_TREE_PASTE,			wxT("粘贴(&P)") );
	menu.Enable( TGR_CLASSIFY_TREE_PASTE,			true );
	menu.Append( TGR_CLASSIFY_TREE_DELETE,			wxT("删除(&D)") );
	menu.Enable( TGR_CLASSIFY_TREE_DELETE,			pNode->Deleteable() );

	menu.AppendSeparator();
	menu.Append(TGR_CLASSIFY_TREE_NEW_GROUP, wxT("新子分组(&G)"));
	menu.Append(TGR_CLASSIFY_TREE_NEW_TRIGGER, wxT("新触发器(&T)"));
	menu.Append(TGR_CLASSIFY_TREE_NEW_FUNCTION, wxT("新函数(&F)"));

	menu.AppendSeparator();
	if (pNode->Disableable())
	{
		menu.Append(TGR_CLASSIFY_TREE_EMPLOY, pNode->Working() ? wxT("禁用(&S)") : wxT("启用(&G)"));
	}

	PopupMenu(&menu, pt);

	return;
}

void GalleryTree::OnItemRClick(wxTreeEvent& event)
{
	wxTreeItemId itemId = event.GetItem();
	GWTreeData *item = itemId.IsOk() ? (GWTreeData *)GetItemData(itemId) : NULL;
	SelectItem( itemId );

	event.Skip();
}
void	GalleryTree::OnBeginDrag( wxTreeEvent& event )
{
	m_draggingItem = nullptr;
	Space::SNode* pNode = GetSpaceNode( event.GetItem() );
	if ( pNode != nullptr && pNode->ClassType() != Space::SNT_ROOT )
	{
		m_draggingItem = event.GetItem();
		event.Allow();
	}
	event.Skip();
}

void	GalleryTree::OnEndDrag(wxTreeEvent& event)
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

void	GalleryTree::OnProcessAny(wxCommandEvent& event)
{
	switch( event.GetId() )
	{
	case TGR_CLASSIFY_TREE_CUT:
		OnCut();
		break;
	case TGR_CLASSIFY_TREE_COPY:
		OnCopy();
		break;
	case TGR_CLASSIFY_TREE_PASTE:
		OnPaste();
		break;
	case TGR_CLASSIFY_TREE_DELETE:
		OnDelete();
		break;
	case TGR_CLASSIFY_TREE_NEW_GROUP:
		OnNewGroup();
		break;
	case TGR_CLASSIFY_TREE_NEW_TRIGGER:
		OnNewTrigger();
		break;
	case TGR_CLASSIFY_TREE_NEW_FUNCTION:
		OnNewFunction();
		break;
	case TGR_CLASSIFY_TREE_EMPLOY:
		OnOperation();
		break;
	default:
		break;
	}
}


bool GetGroupLocality( Space::SNode* pSelNode, Space::SNode*& pParent, Space::SNode*& pPrev, bool bSubGroup)
{
	if( pSelNode == NULL )
		return false;

	switch( pSelNode->ClassType() )
	{
	case Space::SNT_ROOT:
		pParent = pSelNode;
		pPrev = pParent ? pParent->LastChild() : NULL;
		break;
	case Space::SNT_GROUP:
		if (bSubGroup)
		{
			pParent = pSelNode;
			pPrev = pParent ? pParent->LastChild() : NULL;
		}
		else
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
		break;
	case Space::SNT_TRIGGER:
		if( pSelNode->Parent() != NULL )
		{
			pParent = pSelNode->Parent();
			pPrev = pSelNode;
		}
		else
		{
			assert( false );
			return false;
		}
		break;
	default: 
		assert( false );
		return false;
	}
	return true;
}
bool GetTriggerLocality( Space::SNode* pSelNode, Space::SNode*& pParent, Space::SNode*& pPrev )
{
	if( pSelNode == NULL )
		return false;

	switch( pSelNode->ClassType() )
	{
	case Space::SNT_GROUP:
		pParent = pSelNode;
		pPrev = pParent ? pParent->LastChild() : NULL;
		break;

	case Space::SNT_TRIGGER:
	case Space::SNT_FUNCTION:
		pParent = pSelNode->Parent();
		pPrev = pSelNode;
		break;
	default: 
		assert( false );
		return false;
	}
	return true;
}
void	GalleryTree::OnNewGroup()
{
	Space::SNode* pSelNode = GetSelectedNode(true);
	if( pSelNode == NULL )
		return;

	Space::SNode* pParent = NULL;
	Space::SNode* pPrev = NULL;
	if( GetGroupLocality( pSelNode, pParent, pPrev , true ) )
	{
		GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd( Space::GSpaceMgr->BuildDefault<Space::SNT_GROUP>(), pParent, pPrev ));
	}
}

void	GalleryTree::OnNewTrigger()
{
	Space::SNode* pSelNode = GetSelectedNode(true);
	if (pSelNode == NULL)
		return;

	Space::SNode* pParent = NULL;
	Space::SNode* pPrev = NULL;
	if( GetTriggerLocality( pSelNode, pParent, pPrev ) )
	{
		GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd( Space::GSpaceMgr->BuildDefault<Space::SNT_TRIGGER>(), pParent, pPrev ));
	}
}

void	GalleryTree::OnNewFunction()
{
	Space::SNode* pSelNode = GetSelectedNode(true);
	if (pSelNode == NULL)
		return;

	Space::SNode* pParent = NULL;
	Space::SNode* pPrev = NULL;
	if( GetTriggerLocality( pSelNode, pParent, pPrev ) )
	{
		GCmdMgr->AddCommand(GetDocumentID(), new Space::AddNodeCmd( Space::GSpaceMgr->BuildDefault<Space::SNT_FUNCTION>(), pParent, pPrev ));
	}
}

void	GalleryTree::UpdateNewRoot( Space::SNode* pRoot )
{
	this->DeleteAllItems();

	if( pRoot == NULL || pRoot->ClassType() != Space::SNT_ROOT )
		return ;
	
	wxTreeItemId rootId = AddRoot( pRoot->Name(), TriggerIcon_Folder, TriggerIcon_Folder, new GWTreeData( pRoot ) );
	SetItemImage(rootId, TriggerIcon_FolderOpen, wxTreeItemIcon_Expanded);
	pRoot->UserData( "itemid", (rootId.m_pItem) );
	pRoot->UserData( "tree", reinterpret_cast<void*>(this) );

	for (Space::SNode* pNode = pRoot->FirstChild(); pNode; pNode = pNode->Next())
	{
		if (pNode->ClassType() == Space::SNT_VARSET)
			continue;
		CreateNodeWidgets(pNode, rootId, 0, this);
	}

	Expand(rootId);
}

Space::SNode*	GalleryTree::GetSpaceNode( const wxTreeItemId &id )
{
	if( !id.IsOk() )
		return NULL;

	GWTreeData *pData = (GWTreeData*)GetItemData( id );
	if( pData == NULL )
		return NULL;

	return pData->UserData();
}

Space::SNode* GalleryTree::GetSelectedNode(bool bLastOne)
{
	wxArrayTreeItemIds Selections;
	if (GetSelections(Selections) > 0)
	{
		return GetSpaceNode(bLastOne ? Selections.Last() : Selections.front());
	}
	return nullptr;
}

void GalleryTree::SelectNode(Space::SNode* pNode)
{
	if (pNode == nullptr) return;
	wxTreeItemId srcId = pNode->UserData("itemid");
	wxTreeCtrl* pTree = reinterpret_cast<wxTreeCtrl*>(pNode->UserData("tree"));
	if (!srcId.IsOk() || pTree != this) return;
	UnselectAll();
	SelectItem(srcId);
}

void	GalleryTree::OnDelete()
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
}
void	GalleryTree::OnCut()
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
void	GalleryTree::OnCopy()
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
void	GalleryTree::OnOperation()
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

bool	GalleryTree::OnPaste()
{
	Space::SNode* pSelNode = GetSelectedNode(true);
	if (pSelNode == NULL)
		return false;

	vector<Space::SNode*> CuttingNodes = GCmdMgr->GetCuttingNodes();
	vector<Space::SNode*> CopyingNodes;
	if (CuttingNodes.empty())
	{
		CopyingNodes = GCmdMgr->CreateNewPastingNodes();
	}
	vector<Space::SNode*>& PastingNodes = CuttingNodes.size()>0 ? CuttingNodes : CopyingNodes;
	if( PastingNodes.empty() )
		return false;
	bool bSuccess = PasteNodes(PastingNodes, pSelNode, CuttingNodes.size() > 0 ? PST_CUT : PST_COPY);
	if (!bSuccess && CopyingNodes.size() > 0)
	{
		for (size_t index = 0; index < CopyingNodes.size(); ++index)
		{
			SAFE_DELETE(CopyingNodes[index]);
		}
	}
	return bSuccess;
}

bool GalleryTree::PasteNodes(vector<Space::SNode*>& PastingNodes, Space::SNode* pSelNode, EPasteSrcType srcType)
{
	if (PastingNodes.size() == 0) return false;

	for (UINT Index = 0; Index < PastingNodes.size(); ++Index)
	{
		if (!pSelNode->Pasteable(PastingNodes[Index]))
		{
			return false;
		}
	}

	//查询应该粘贴在那里
	Space::SNode* pParent = NULL;
	Space::SNode* pPrev = NULL;
	switch (PastingNodes.front()->ClassType())
	{
	case Space::SNT_GROUP:
	{
		if (!GetGroupLocality(pSelNode, pParent, pPrev, false))
		{
			return false;
		}
	}
	break;
	case Space::SNT_TRIGGER:
	{
		if (!GetTriggerLocality(pSelNode, pParent, pPrev))
		{
			return false;
		}
	}
	break;
	default: return false;
	}

	if (srcType == PST_CUT || srcType == PST_DRAG)
	{
		bool bIsTargetBeenCutting = std::find(PastingNodes.begin(), PastingNodes.end(), pPrev) != PastingNodes.end();
		for (Space::SNode* pNode = pParent; pNode != NULL && !bIsTargetBeenCutting; pNode = pNode->Parent())
		{
			bIsTargetBeenCutting |= std::find(PastingNodes.begin(), PastingNodes.end(), pNode) != PastingNodes.end();
		}
		if (!bIsTargetBeenCutting)
		{
			vector<Space::ICommand*> Commands;
			for (auto ritr = PastingNodes.rbegin(); ritr != PastingNodes.rend(); ++ritr)
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
		for (auto itr = PastingNodes.begin(); itr != PastingNodes.end(); ++itr)
		{
			Space::SNode* pCopyingNode = *itr;
			if (pCopyingNode->ClassType() == Space::SNT_GROUP || pCopyingNode->ClassType() == Space::SNT_TRIGGER)
			{
				wstring name = pCopyingNode->Name();
				Space::GSpaceMgr->UniqueName(pCopyingNode);
			}
		}

		vector<Space::ICommand*> Commands;
		for (auto ritr = PastingNodes.rbegin(); ritr != PastingNodes.rend(); ++ritr)
		{
			Space::SNode* pCopyingNode = *ritr;
			Commands.push_back(new Space::PasteNodeCmd(pCopyingNode, pParent, pPrev, false, NULL, NULL));
		}
		GCmdMgr->AddCommand(GetDocumentID(), new Space::MacroCmd(Commands));

		GCmdMgr->OnPaste();
	}

	return true;
}