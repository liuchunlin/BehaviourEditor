#include "Commands.h"
#include "CmdManager.h"
#include "SpaceManager.h"
#include "SFunction.h"
#include "TgrApp.h"
#include "TgrFrame.h"
#include "UI/GalleryTree.h"
#include "UI/EditTree.h"
#include "UI/GWTreeData.h"
#include "UI/VarDlg.h"
using namespace Space;

extern intptr_t GetDocumentIDFromNode(const Space::SNode*);

namespace Space
{
	bool IsNodeVisible( const SNode* pSrc )
	{
		if( pSrc == NULL )
			return false;
		switch( pSrc->ClassType() )
		{
		case SNT_ROOT:
		case SNT_GROUP:
		case SNT_TRIGGER:
		case SNT_FUNCTION:
			return true;
		}
		bool bSyncTree = 
			pSrc->Ancestor(SNT_TRIGGER) == wxGetApp().GetTgrFrame()->GetEditingNode() ||
			pSrc->Ancestor(SNT_FUNCTION) == wxGetApp().GetTgrFrame()->GetEditingNode();
		return bSyncTree;
	}

	void SetItemImageForNode(SNode* pSrc, wxTreeCtrl* pTree, wxTreeItemId srcId)
	{
		if (pSrc == nullptr) return;

		bool isFolder = false;
		ETriggerIcon normalIcon = TriggerIcon_File;
		switch (pSrc->ClassType())
		{
		case SNT_TFUNCCALL:	normalIcon = TriggerIcon_Action; break;
		case SNT_SFUNCCALL:	normalIcon = TriggerIcon_Action; break;
		case SNT_EVENT:		normalIcon = TriggerIcon_Event; break;
		case SNT_CONDITION:	normalIcon = TriggerIcon_Condition; break;
		case SNT_VARIABLE:	normalIcon = TriggerIcon_Variable; break;
		case SNT_MULTIPLE_IF: normalIcon = TriggerIcon_Branch; break;
		case SNT_WHILE:		normalIcon = TriggerIcon_Loop; break;
		case SNT_FOR_EACH:	normalIcon = TriggerIcon_Loop; break;
		case SNT_RETURN:	normalIcon = TriggerIcon_Return; break;
		case SNT_TRIGGER:	normalIcon = TriggerIcon_Trigger; break;
		case SNT_FUNCTION:	normalIcon = TriggerIcon_Function; break;
		case SNT_ROOT:
		case SNT_EVENTROOT:
		case SNT_CONDITIONROOT:
		case SNT_ACTIONROOT:
		case SNT_PARAMROOT:
		case SNT_GROUP:
		case SNT_IF:
		case SNT_THEN:
		case SNT_ELSE:
		case SNT_LOOP:
			normalIcon = TriggerIcon_Folder;
			isFolder = true;
			break;
		}
		pTree->SetItemImage(srcId, normalIcon, wxTreeItemIcon_Normal);
		pTree->SetItemImage(srcId, normalIcon, wxTreeItemIcon_Selected);

		if (isFolder)
		{
			pTree->SetItemImage(srcId, TriggerIcon_FolderOpen, wxTreeItemIcon_Expanded);
		}
	}

	wxTreeItemId CreateNodeWidgets( SNode* pSrc, wxTreeItemId parentId, wxTreeItemId prevId, wxTreeCtrl* pTree, bool bFirstChild )
	{
		if( pSrc == NULL || pTree == NULL || !parentId.IsOk() )
			return 0;

		wxTreeItemId srcId;
		if (bFirstChild)
		{
			srcId = pTree->PrependItem(parentId, pSrc->GetReadText(), 0, 0, new GWTreeData(pSrc));
		}
		else if( prevId.IsOk() )
		{
			srcId = pTree->InsertItem( parentId, prevId, pSrc->GetReadText(), 0, 0, new GWTreeData( pSrc ) );
		}
		else
		{
			srcId = pTree->AppendItem(parentId, pSrc->GetReadText(), 0, 0, new GWTreeData(pSrc));
		}
		SetItemImageForNode(pSrc, pTree, srcId);

		pSrc->UserData( "itemid", srcId.m_pItem );
		pSrc->UserData( "tree", reinterpret_cast<void*>(pTree) );
		UpdateNodeWidgets( pSrc, false );

		if( pSrc->ClassType() == SNT_GROUP )
		{
			for (SNode* pChild = pSrc->FirstChild(); pChild != NULL; pChild = pChild->Next())
			{
				CreateNodeWidgets(pChild, srcId, 0, pTree, false);
			}
		}
		else if( pSrc->ClassType() == SNT_MULTIPLE_IF ||
			pSrc->ClassType() == SNT_WHILE )
		{
			for (SNode* pChild = pSrc->FirstChild(); pChild != NULL; pChild = pChild->Next())
			{
				wxTreeItemId childId = CreateNodeWidgets(pChild, srcId, 0, pTree, false);
				for ( SNode* pCC = pChild->FirstChild(); pCC != NULL; pCC = pCC->Next() )
				{
					CreateNodeWidgets(pCC, childId, 0, pTree, false );
				}
			}
		}
		else if( pSrc->ClassType() == SNT_FOR_EACH )
		{
			SNode* pLoop = pSrc->LastChild();
			if( pLoop != NULL )
			{
				wxTreeItemId loopId = CreateNodeWidgets(pLoop, srcId, 0, pTree, false);
				for ( SNode* pChild = pLoop->FirstChild(); pChild != NULL; pChild = pChild->Next() )
				{
					CreateNodeWidgets( pChild, loopId, 0, pTree, false );
				}
			}
		}

		pTree->Expand( parentId );

		return srcId;
	}

	void UpdateNodeWidgets( const SNode* pSrc, bool bUpdateParents )
	{
		if (pSrc == NULL) return;

		if (bUpdateParents)
			UpdateNodeWidgets(pSrc->Parent(), bUpdateParents);

		if (!IsNodeVisible(pSrc)) return;

		wxTreeItemId srcId = pSrc->UserData("itemid");
		wxTreeCtrl* pTree = reinterpret_cast<wxTreeCtrl *>( pSrc->UserData("tree") );
		if( pTree && srcId.IsOk() )
		{
			wxString newText = pSrc->GetReadText();
			pTree->SetItemText(srcId, newText);
			if (pSrc->GetErrorMask() != VS_OK)
			{
				pTree->SetItemTextColour(srcId, *wxRED);
			}
			else if (!pSrc->Working())
			{
				pTree->SetItemTextColour(srcId, *wxLIGHT_GREY);
			}
			else
			{
				pTree->SetItemTextColour(srcId, *wxBLACK);
			}
			pTree->SetItemBold(srcId, false);
		}
	}

	void UpdateTreeWidgets(const SNode* pSrc, bool bUpdateParents)
	{
		if (!IsNodeVisible(pSrc))
		{
			if (bUpdateParents && pSrc)
				UpdateNodeWidgets(pSrc->Parent(), bUpdateParents);
			return;
		}

		UpdateNodeWidgets(pSrc, bUpdateParents);

		for (SNode* child = pSrc->FirstChild(); child; child = child->Next())
		{
			UpdateTreeWidgets(child, false);
		}
	}

	bool UpdateReferenceNodes(SNode* pSrc)
	{
		if (pSrc == NULL) return false;

		set<SNode*> references = pSrc->GetReferences();
		for (set<SNode*>::iterator iter(references.begin()); iter != references.end(); ++iter)
		{
			SNode* pRef = (*iter);
			if (pSrc->IsA(SNT_TRIGGER))
			{
				if (pSrc->Parent() == NULL)
				{
					pRef->Name(_T(""));
				}
				else
				{
					pRef->Name(pSrc->Name());
				}
			}
			else if (pSrc->IsA(SNT_VARIABLE))
			{
				if (pRef->TEETmp() != pSrc->TEETmp() || pSrc->Parent() == NULL || pSrc->IsArray() != pRef->IsArray())
				{
					pRef->Name(_T(""));
				}
				else
				{
					pRef->Name(pSrc->Name());
				}
			}
			else if (pSrc->IsA(SNT_FUNCTION))
			{
				if (pSrc->Parent() == NULL)
				{
					pRef->Name(_T(""));
				}
				else
				{
					pRef->Name(pSrc->Name());
				}
			}

			SNode::CheckErrorTree(pRef);
			UpdateNodeWidgets(pRef, true);
		}

		//如果是函数参数的话，还应该更新相应函数的引用们（调用者）
		if (pSrc->IsA(SNT_VARIABLE) && pSrc->Parent()->IsA(SNT_PARAMROOT))
		{
			if (SNode* pFunc = pSrc->Ancestor(SNT_FUNCTION))
				UpdateReferenceNodes(pFunc);
		}
		return true;
	}

	bool AddNodeItem( SNode* pSrc, SNode *pParent, SNode *pPrev, bool bFirstChild )
	{
		if( pParent == NULL || pSrc == NULL )
			return false;

		if( bFirstChild )
		{
			pParent->InsertBefore(pSrc, pParent->FirstChild());
		}
		else
		{
			pParent->InsertOrAppend(pSrc, pPrev);
		}
		pSrc->FixTEETmp();

		wxTreeItemId parentId = 0, prevId = 0;
		parentId.m_pItem = pParent->UserData("itemid");
		prevId.m_pItem = pPrev != NULL ? pPrev->UserData("itemid") : 0;

		if( IsGlobalVariable(pSrc) )
		{
			wxListCtrl *pVarSet = reinterpret_cast<wxListCtrl *>( pParent->UserData("list") );
			if( pVarSet != NULL )
			{
				SNode* pVarItem = pSrc;
				long tmp = 0;
				if( bFirstChild )
				{
					tmp = pVarSet->InsertItem( 0, pVarItem->Name().c_str(), 0 );
				}
				else
				{
					long index = pVarSet->FindItem( -1, reinterpret_cast<wxUIntPtr>(pPrev) );
					tmp = pVarSet->InsertItem( index==-1?pVarSet->GetItemCount():index+1, pVarItem->Name().c_str(), 0 );
				}
				pVarSet->SetItemData( tmp, long(pVarItem) );

				VarListDlg *pVarDlg = reinterpret_cast<VarListDlg *>( pParent->UserData("varlistdlg") );
				if( pVarDlg != NULL )
				{
					pVarDlg->RefreshLayout( pSrc );
				}
			}
		}
		else
		{
			SNode::PostLoadedSpace( pSrc );
			SNode::CheckErrorTree( pSrc );

			bool bSyncTree = IsNodeVisible( pParent );
			if( bSyncTree )
			{
				wxTreeCtrl* pTree = reinterpret_cast<wxTreeCtrl*>(pParent->UserData("tree"));
				wxTreeItemId widgetID = CreateNodeWidgets(pSrc, parentId, prevId, pTree, bFirstChild);
				if(widgetID.IsOk())
				{
					if (pSrc->IsA(SNT_TRIGGER) ||
						pSrc->IsA(SNT_FUNCTION))
					{
						pTree->UnselectAll();
						pTree->SelectItem(widgetID);
					}
				}
			}
		}

		UpdateReferenceNodes(pSrc);
		return true;
	}

	bool RemoveItemNode( SNode* pSrc )
	{
		if( pSrc == NULL ) return false;
		if( IsGlobalVariable(pSrc) )
		{
			wxListCtrl *pVarSet = reinterpret_cast<wxListCtrl *>(pSrc->Parent()->UserData("list"));
			if (pVarSet != NULL)
			{
				long index = pVarSet->FindItem(-1, reinterpret_cast<wxUIntPtr>(pSrc));
				pVarSet->DeleteItem(index);
			}
			pSrc->Unlink();
		}
		else
		{
			bool bSyncTree = IsNodeVisible( pSrc );
			if( bSyncTree )
			{
				wxTreeCtrl *pTree = reinterpret_cast<wxTreeCtrl*>( pSrc->UserData("tree") );
				assert( pTree != NULL );
				if( pTree != NULL )
				{
					wxTreeItemId srcId(pSrc->UserData("itemid"));
					if( srcId.IsOk() )
					{
						pTree->Delete( srcId );
					}
					if( pSrc->ClassType() == SNT_TRIGGER )
					{
						wxGetApp().GetTgrFrame()->GetEditTree()->OnSelectGalleryNode( NULL );
					}
				}
				pSrc->UserData( "itemid", (NULL) );
				pSrc->UserData( "tree", (NULL) );
			}

			SNode* pParent = pSrc->Parent();
			pSrc->Unlink();
			SNode::CheckErrorTree( pParent );

			//delete command list for trigger
			if (pSrc->ClassType() == SNT_TRIGGER)
			{
				GCmdMgr->RemoveCommands(GetDocumentIDFromNode(pSrc));
			}
			else if (pSrc->ClassType() == SNT_GROUP)
			{
				vector<Space::SNode*> triggers;
				pSrc->FindChildren(SVerifyNodeType(SNT_TRIGGER), triggers, true);
				for each (auto pTrigger in triggers)
				{
					GCmdMgr->RemoveCommands(GetDocumentIDFromNode(pTrigger));
				}
			}
		}

		UpdateReferenceNodes(pSrc);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
//BatchCmd
MacroCmd::MacroCmd( const vector<ICommand*>& cmds )
:Cmds(cmds)
{
}
MacroCmd::~MacroCmd()
{
	vector<ICommand*>::iterator it( Cmds.begin() );
	for ( ; it != Cmds.end(); ++it )
	{
		ICommand* pCmd = (*it);
		SAFE_DELETE( pCmd );
	}
	Cmds.clear();
}
bool MacroCmd::Execute()
{
	vector<ICommand*>::iterator it( Cmds.begin() );
	for ( ; it != Cmds.end(); ++it )
	{
		ICommand* pCmd = (*it);
		pCmd->Execute();
	}
	return true;
}
bool MacroCmd::UnExecute()
{
	vector<ICommand*>::reverse_iterator rit( Cmds.rbegin() );
	for ( ; rit != Cmds.rend(); ++rit )
	{
		ICommand* pCmd = (*rit);
		pCmd->UnExecute();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//AddNodeCmd
AddNodeCmd::AddNodeCmd( SNode *src, SNode *parent, SNode *prev )
:pSrc(src)
,pParent(parent)
,pPrev(prev)
{
	assert( pSrc != NULL );
}
AddNodeCmd::~AddNodeCmd()
{
	if( pHanging != NULL && pHanging->Parent() == NULL )
	{
		SAFE_DELETE( pHanging );
	}
}
bool AddNodeCmd::Execute()
{
	AddNodeItem( pSrc, pParent, pPrev, pPrev == NULL );
	pHanging = NULL;
	return true;
}
bool AddNodeCmd::UnExecute()
{
	RemoveItemNode( pSrc );
	pHanging = pSrc;
	return true;
}
//////////////////////////////////////////////////////////////////////////
//DeleteNodeCmd
DeleteNodeCmd::DeleteNodeCmd( SNode *src )
:pSrc(src)
,pHanging(NULL)
,pParent(NULL)
,pPrev(NULL)
,bExpand(false)
,bFirstChild(false)
{
	GCmdMgr->CmdNodeDeleting( src );
	assert( pSrc != NULL );

	pParent = pSrc->Parent();
	pPrev = pSrc->Prev();

	assert( pParent != NULL );
	if( pParent != NULL && pParent->FirstChild() == pSrc )
	{
		bFirstChild = true;
	}
}
DeleteNodeCmd::~DeleteNodeCmd()
{
	if( pHanging != NULL && pHanging->Parent() == NULL )
	{
		SAFE_DELETE( pHanging );
	}
}
bool DeleteNodeCmd::Execute()
{
	RemoveItemNode( pSrc );
	pHanging = pSrc;
	return true;
}
bool DeleteNodeCmd::UnExecute()
{
	AddNodeItem( pSrc, pParent, pPrev, bFirstChild );
	pHanging = NULL;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//RenameNodeCmd
RenameNodeCmd::RenameNodeCmd( SNode *src, const wstring& ExchangeName )
:m_pSrc(src)
,m_exchangeName(ExchangeName)
{
}

bool RenameNodeCmd::Execute()
{
	if( m_pSrc == NULL ) return false;

	wstring tempName = m_pSrc->Name();
	m_pSrc->Name(m_exchangeName);
	m_exchangeName = tempName;

	SNode::CheckErrorTree(m_pSrc, false);
	UpdateNodeWidgets(m_pSrc, true);
	EditTree* editTree = wxGetApp().GetTgrFrame()->GetEditTree();
	if (editTree->GetEditingNode() == m_pSrc)
	{
		wxTreeItemId rootId = editTree->GetRootItem();
		if (rootId.IsOk())
		{
			editTree->SetItemText(rootId, m_pSrc->GetReadText());
		}
	}
	UpdateReferenceNodes( m_pSrc );
	return true;
}
bool RenameNodeCmd::UnExecute()
{
	Execute();
	return true;
}

//////////////////////////////////////////////////////////////////////////
//RenameFunctionCmd
RenameFunctionCmd::RenameFunctionCmd(SNode* src, const wstring& exchangeName, const wstring& exchangeDesc)
	: RenameNodeCmd(src, exchangeName)
	, m_exchangeDesc(exchangeDesc)
{
}

bool RenameFunctionCmd::Execute()
{
	SFunction* pFunc = dynamic_cast<SFunction*>(m_pSrc);
	if (pFunc == NULL)	return false;

	wstring temp = pFunc->m_desc;
	pFunc->m_desc = m_exchangeDesc;
	m_exchangeDesc = temp;

	return RenameNodeCmd::Execute();
}

//////////////////////////////////////////////////////////////////////////
//PasteNodeCmd
PasteNodeCmd::PasteNodeCmd( SNode *src, SNode* dstParent, SNode *dstPrev, bool cut /* = false */, SNode *orgParent /* = NULL */, SNode *orgPrev /* = NULL */)
:pSrc(src)
,pDstParent(dstParent)
,pDstPrev(dstPrev)
,bCut(cut)
,pOrgParent(orgParent)
,pOrgPrev(orgPrev)
,pHanging(NULL)
{
	assert( pSrc != NULL && pDstParent != NULL );
	if( bCut )
	{
		assert( pOrgParent != NULL );
	}
}
PasteNodeCmd::~PasteNodeCmd()
{
	if( pHanging != NULL && pHanging->Parent() == NULL )
	{
		SAFE_DELETE( pHanging );
	}
}
bool PasteNodeCmd::Execute()
{
	if( bCut )
	{
		//Delete
		RemoveItemNode( pSrc );
	}
	//Add
	AddNodeItem( pSrc, pDstParent, pDstPrev, pDstPrev == NULL );
	if( !bCut )
		pHanging = NULL;

	return true;
}

bool PasteNodeCmd::UnExecute()
{
	//Delete
	RemoveItemNode( pSrc );
	if( !bCut )
		pHanging = pSrc;
	if( bCut )
	{
		//Add
		AddNodeItem( pSrc, pOrgParent, pOrgPrev, pOrgPrev == NULL );
	}
	return true;
} 
//////////////////////////////////////////////////////////////////////////
//ExChangeNodeCmd
ExChangeNodeCmd::ExChangeNodeCmd( SNode *bein, SNode* beout )
:pBeIn(bein)
,pBeOut(beout)
,bFirstChild(false)
{
	assert( pBeIn != NULL );
	assert( pBeOut != NULL );

	if( pBeIn != NULL && pBeOut != NULL )
	{
		pParent = pBeIn->Parent();
		pPrev = pBeIn->Prev();

		if( pParent != NULL )
		{
			bFirstChild = pParent->FirstChild() == bein;
		}
	}
}
ExChangeNodeCmd::~ExChangeNodeCmd()
{
	if (pBeOut) pBeOut->Remove();
}

bool ExChangeNodeCmd::Execute()
{
	if( pBeIn == NULL || pBeOut == NULL || pParent == NULL )
	{
		assert( false );
		return false;
	}
	SNode::Swap( pBeIn, pBeOut );
	SNode* pTemp = pBeOut;
	pBeOut = pBeIn;
	pBeIn = pTemp;

	pBeIn->UserData( "itemid", pBeOut->UserData("itemid") );
	pBeIn->UserData( "tree", pBeOut->UserData("tree") );

	SNode* pParent = pBeIn->Parent();
	SNode* pPrev = pBeIn->Prev();
	RemoveItemNode( pBeIn );
	AddNodeItem( pBeIn, pParent, pPrev, bFirstChild );
	return true;
}
bool ExChangeNodeCmd::UnExecute()
{
	return Execute();
}

//////////////////////////////////////////////////////////////////////////
//ExChangeNodeChildCmd
ExChangeNodeChildCmd::ExChangeNodeChildCmd( SNode *bein, SNode* beout  )
:pBeIn(bein)
,pBeOut(beout)
{
	assert( pBeIn != NULL );
	assert( pBeOut != NULL );

	if( pBeIn != NULL )
	{
		pParent = pBeIn->Parent();
		pPrev = pBeIn->Prev();
	}

	if( pBeIn != NULL && pBeOut != NULL )
	{
		SNode* pInChild = pBeIn->FirstChild();
		SNode* pOutChild = pBeOut->FirstChild();

		for( ; ; pInChild = pInChild->Next(), pOutChild = pOutChild->Next() )
		{
			assert( ( pInChild != NULL && pOutChild != NULL ) || (pInChild == NULL && pOutChild == NULL) );
			if( pInChild == NULL || pOutChild == NULL )
				break;
		}
		assert( pBeIn->Name() == pBeOut->Name() );
	}
}
ExChangeNodeChildCmd::~ExChangeNodeChildCmd()
{
	SAFE_DELETE( pBeOut );
}

bool ExChangeNodeChildCmd::Execute()
{
	if( pBeIn == NULL || pBeOut == NULL || pParent == NULL )
	{
		assert( false );
		return false;
	}
	SNode* pInChild = pBeIn->FirstChild();
	SNode* pOutChild = pBeOut->FirstChild();
	for( ; ; pInChild = pInChild->Next(), pOutChild = pOutChild->Next() )
	{
		if( pInChild == NULL || pOutChild == NULL )
			break;
		SNode::Swap( pInChild, pOutChild );
	}

	{
		pBeIn->FixTEETmp();
		SNode::CheckErrorTree( pBeIn );
		UpdateNodeWidgets(pBeIn, true);
	}
	return true;
}
bool ExChangeNodeChildCmd::UnExecute()
{
	return Execute();
}

//////////////////////////////////////////////////////////////////////////
//ExChangeNodeContentCmd
ExChangeNodeContentCmd::ExChangeNodeContentCmd( SNode *bein, SNode* beout  )
:pBeIn(bein)
,pBeOut(beout)
,pTemp(NULL)
{
	assert( pBeIn != NULL );
	assert( pBeOut != NULL );

	if( pBeIn != NULL )
	{
		pParent = pBeIn->Parent();
	}

	pTemp = SNode::Clone( pBeOut );
	UpdateReferenceNodes( pBeIn );
}
ExChangeNodeContentCmd::~ExChangeNodeContentCmd()
{
	SAFE_DELETE( pBeOut );
	SAFE_DELETE( pTemp );
}

bool ExChangeNodeContentCmd::Execute()
{
	if( pBeIn == NULL || pBeOut == NULL || pTemp == NULL || pBeIn->ClassType() != pBeOut->ClassType() )
	{
		assert( false );
		return false;
	}

	void* itemid = pBeIn->UserData("itemid");
	void* tree = pBeIn->UserData("tree");

	pTemp->CopyFrom(pBeOut);
	pBeOut->CopyFrom(pBeIn);
	pBeIn->CopyFrom(pTemp);

	pBeIn->UserData( "itemid", itemid );
	pBeIn->UserData( "tree", tree );

	SNode* pInChild;
	SNode* pOutChild;
	for(pInChild = pBeIn->FirstChild(), pOutChild = pBeOut->FirstChild();
		pInChild && pOutChild; 
		pInChild = pInChild->Next(), pOutChild = pOutChild->Next() )
	{
		SNode::Swap( pInChild, pOutChild );
	}

	if (Space::IsGlobalVariable(pBeIn))
	{
		if ( VarListDlg* pVarDlg = reinterpret_cast<VarListDlg*>(pBeIn->Parent()->UserData("varlistdlg")) )
		{
			pVarDlg->RefreshLayout(pBeIn);
		}
	}
	UpdateReferenceNodes(pBeIn);
	if (Space::IsLocalVariable(pBeIn))
	{
		SNode::CheckErrorTree(pBeIn->Parent());
	}
	else
	{
		SNode::CheckErrorTree(pBeIn);
	}
	UpdateNodeWidgets(pBeIn, true);
	return true;
}
bool ExChangeNodeContentCmd::UnExecute()
{
	return Execute();
}

//////////////////////////////////////////////////////////////////////////
//ExChangeOperateNodeCmd
ExChangeOperateNodeCmd::ExChangeOperateNodeCmd( SNode *src )
: pSrc( src )
{
	assert( pSrc != NULL );
}
ExChangeOperateNodeCmd::~ExChangeOperateNodeCmd()
{
}
bool ExChangeOperateNodeCmd::Execute()
{
	if( pSrc == NULL || !pSrc->Disableable() )
		return false;

	pSrc->Working( !pSrc->Working() );

	UpdateNodeWidgets( pSrc, false);

	return true;
}

bool ExChangeOperateNodeCmd::UnExecute()
{
	return Execute();
}
