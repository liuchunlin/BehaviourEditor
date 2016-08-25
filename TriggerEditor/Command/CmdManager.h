#pragma once

#include "Commands.h"
#include <Vek/Base/Module.h>

namespace Space
{
	class CmdList
	{
	public:
		CmdList();
		~CmdList();
		bool AddCommand(ICommand* pCommand, bool bExecute);
		bool UnDo();
		bool ReDo();
		bool CanUnDo();
		bool CanReDo();

	private:
		list<ICommand*> m_cmds;
		list<ICommand*>::iterator m_undoIter;
		list<ICommand*>::iterator m_redoIter;
	};

	class CmdManager : public Vek::Module<CmdManager>
	{
		friend class Vek::Singleton<CmdManager>;
	public:
		//undo/redo
		int Flush();
		bool AddCommand(intptr_t docID, ICommand* pCommand, bool bExecute = true);
		void RemoveCommands(intptr_t docID);
		bool UnDo(intptr_t docID);
		bool ReDo(intptr_t docID);
		bool CanUnDo(intptr_t docID);
		bool CanReDo(intptr_t docID);

		//select
		SNode* Selected() const { return m_pSelected; }
		void Selected(SNode* val) { m_pSelected = val; }

		//copy/paste
		bool Cut(const vector<SNode*>& Nodes);
		bool Copy(const vector<SNode*>& Nodes);
		const vector<SNode*> CreateNewPastingNodes() const;
		const vector<SNode*>& GetCuttingNodes();
		void OnPaste();
		void CmdNodeDeleting(SNode* pNode);

		//Variable
		bool HasVarItemName(const wstring& name)
		{
			return m_VarNames.find(name) != m_VarNames.end();
		}
		void AddVarItemName(const wstring& name)
		{
			m_VarNames.insert(name);
		}
		void RemoveVarItemName(const wstring& name)
		{
			m_VarNames.erase(name);
		}
	private:
		CmdManager();
		~CmdManager();
	private:
		map<intptr_t, CmdList*>	m_docCmds;

		vector<SNode*> m_CuttingNodes;
		SNode* m_pSelected;

		//Variable
		set<wstring> m_VarNames;
	};
}

extern Space::CmdManager* GCmdMgr;
