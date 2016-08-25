#include "CmdManager.h"
#include "TgrApp.h"
#include "TgrFrame.h"
#include <algorithm>

using namespace Space;
extern void appClipboardCopy(const char* Str);
extern string appClipboardPaste();
CmdManager* GCmdMgr = CmdManager::GetInstance();

void KickOutChildNodes(vector<SNode*>& Nodes)
{
	// É¾³ý×Ó½Úµã
	vector<SNode*>::iterator itr = Nodes.begin();
	while (itr != Nodes.end())
	{
		bool bIsChild = false;
		for (SNode* pParent = (*itr)->Parent(); pParent != NULL; pParent = pParent->Parent())
		{
			if (std::find(Nodes.begin(), Nodes.end(), pParent) != Nodes.end())
			{
				bIsChild = true;
				break;
			}
		}

		if (bIsChild)
		{
			itr = Nodes.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

//-------------------------------------------------------------------------------------
CmdList::CmdList()
{
	m_undoIter = m_cmds.begin();
	m_redoIter = m_cmds.end();
}

CmdList::~CmdList()
{
	list<ICommand*>::iterator iter(m_cmds.begin());
	for (; iter != m_cmds.end(); ++iter)
	{
		ICommand* pCommand = (*iter);
		SAFE_DELETE(pCommand);
	}
	m_cmds.clear();
	m_undoIter = m_cmds.begin();
	m_redoIter = m_cmds.end();
}

bool CmdList::AddCommand(ICommand* pCommand, bool bExecute)
{
	auto leavingIter = m_redoIter;
	for (; leavingIter != m_cmds.end(); ++leavingIter)
	{
		ICommand* pCommand = (*leavingIter);
		SAFE_DELETE(pCommand);
	}
	m_cmds.erase(m_redoIter, m_cmds.end());

	m_cmds.push_back(pCommand);
	m_undoIter = m_cmds.end();
	m_redoIter = m_undoIter--;

	if (bExecute)
	{
		pCommand->Execute();
	}
	return true;
}

bool CmdList::UnDo()
{
	if (m_undoIter != m_cmds.end())
	{
		ICommand *pCommand = (*m_undoIter);
		pCommand->UnExecute();
		if (m_undoIter != m_cmds.begin())
		{
			m_redoIter = m_undoIter--;
		}
		else
		{
			m_redoIter = m_cmds.begin();
			m_undoIter = m_cmds.end();
		}
		return true;
	}
	return false;
}

bool CmdList::ReDo()
{
	if (m_redoIter != m_cmds.end())
	{
		ICommand *pCommand = (*m_redoIter);
		pCommand->Execute();
		m_undoIter = m_redoIter++;
		return true;
	}
	return false;
}

bool CmdList::CanUnDo()
{
	return m_undoIter != m_cmds.end();
}

bool CmdList::CanReDo()
{
	return m_redoIter != m_cmds.end();
}

//-------------------------------------------------------------------------------------
CmdManager::CmdManager()
: m_pSelected(NULL)
{
	Flush();
}
CmdManager::~CmdManager()
{
	Flush();
}

int	CmdManager::Flush()
{
	for each (auto var in m_docCmds)
	{
		SAFE_DELETE(var.second);
	}
	m_docCmds.clear();

	m_CuttingNodes.clear();

	m_pSelected = NULL;
	m_VarNames.clear();
	return 1;
}

bool	CmdManager::AddCommand( intptr_t docID, ICommand* pCommand, bool bExecute )
{
	if (pCommand == nullptr) return false;
	CmdList* pCmdList = nullptr;
	auto listIter = m_docCmds.find(docID);
	if (listIter == m_docCmds.end())
	{
		pCmdList = m_docCmds[docID] = new CmdList();
	}
	else
	{
		pCmdList = listIter->second;
	}
	pCmdList->AddCommand(pCommand, bExecute);

	wxGetApp().GetTgrFrame()->UpdateCommandControls();
	wxGetApp().GetTgrFrame()->SetFileChanged( true );

	return true;
}

void	CmdManager::RemoveCommands(intptr_t docID)
{
	auto listIter = m_docCmds.find(docID);
	if (listIter != m_docCmds.end())
	{
		SAFE_DELETE(listIter->second);
		m_docCmds.erase(listIter);
	}
}

bool	CmdManager::UnDo(intptr_t docID)
{
	auto listIter = m_docCmds.find(docID);
	if (listIter == m_docCmds.end()) return false;
	listIter->second->UnDo();

	wxGetApp().GetTgrFrame()->UpdateCommandControls();
	return true;
}
bool	CmdManager::ReDo(intptr_t docID)
{
	auto listIter = m_docCmds.find(docID);
	if (listIter == m_docCmds.end()) return false;
	listIter->second->ReDo();

	wxGetApp().GetTgrFrame()->UpdateCommandControls();
	return true;
}
bool	CmdManager::CanUnDo(intptr_t docID)
{
	auto listIter = m_docCmds.find(docID);
	if (listIter == m_docCmds.end()) return false;
	return listIter->second->CanUnDo();
}
bool	CmdManager::CanReDo(intptr_t docID)
{
	auto listIter = m_docCmds.find(docID);
	if (listIter == m_docCmds.end()) return false;
	return listIter->second->CanReDo();
}
const vector<SNode*>	CmdManager::CreateNewPastingNodes() const 
{
	// Paste from clipboard text.
	string NodeText = appClipboardPaste();
	vector<SNode*> Nodes;
	SNode::StringToNodeTrees(NodeText, Nodes);
	return Nodes;
}
const vector<SNode*>&	CmdManager::GetCuttingNodes()
{
	return m_CuttingNodes;
}
bool	CmdManager::Cut( const vector<SNode*>& Nodes )
{
	vector<SNode*> VerifiedNodes = Nodes;
	KickOutChildNodes(VerifiedNodes);

	Copy(VerifiedNodes);

	for (UINT Index = 0; Index < VerifiedNodes.size(); ++Index)
	{
		SNode* pNode = VerifiedNodes[Index];
		if (pNode)
		{
			wxTreeItemId itemId = 0;
			itemId.m_pItem = pNode->UserData("itemid");
			wxTreeCtrl *pTree = reinterpret_cast<wxTreeCtrl *>( pNode->UserData("tree") );
			assert( pTree != NULL );
			if( pTree != NULL )
			{
				pTree->SetItemBold(itemId, true);
			}
		}
	}

	m_CuttingNodes = VerifiedNodes;

	return true;
}
bool	CmdManager::Copy( const vector<SNode*>& Nodes )
{
	vector<SNode*> VerifiedNodes = Nodes;
	KickOutChildNodes(VerifiedNodes);

	if( VerifiedNodes.size() > 0 )
	{
		// Copy to clipboard as text.
		string NodeText;
		SNode::NodeTreesToString(VerifiedNodes, NodeText);
		appClipboardCopy(NodeText.c_str());
	}
	for (UINT Index = 0; Index < m_CuttingNodes.size(); ++Index)
	{
		UpdateNodeWidgets(m_CuttingNodes[Index], true);
	}
	m_CuttingNodes.clear();

	return true;
}

void	CmdManager::OnPaste()
{
	m_CuttingNodes.clear();
}

void	CmdManager::CmdNodeDeleting( SNode* pNode )
{
	for (vector<SNode*>::const_iterator itr = m_CuttingNodes.begin(); itr != m_CuttingNodes.end(); ++itr)
	{
		if (*itr == pNode)
		{
			m_CuttingNodes.erase(itr);
			break;
		}
	}
	wxGetApp().GetTgrFrame()->CmdNodeDeleting( pNode );
}
