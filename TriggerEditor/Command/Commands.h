#pragma once

#include "Command.h"
#include "SNode.h"
#include "wx/treectrl.h"

namespace Space
{
	//helper function
	bool IsNodeVisible(const SNode* pSrc);
	wxTreeItemId CreateNodeWidgets(SNode* pSrc, wxTreeItemId parentId, wxTreeItemId prevId, wxTreeCtrl* pTree, bool bFirstChild = false);
	void UpdateNodeWidgets(const SNode* pSrc, bool bUpdateParents);
	void UpdateTreeWidgets(const SNode* pSrc, bool bUpdateParents);
	bool UpdateReferenceNodes( Space::SNode* pSrc );
	bool AddNodeItem( Space::SNode* pSrc, Space::SNode *pParent, Space::SNode *pPrev, bool bFirstChild );
	bool RemoveItemNode( Space::SNode* pSrc );

	class MacroCmd: public ICommand
	{
	public:
		MacroCmd( const vector<ICommand*>& cmds );
		virtual ~MacroCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		vector<ICommand*>	Cmds;
	};

	class AddNodeCmd: public ICommand
	{
	public:
		AddNodeCmd( SNode *src, SNode *parent, SNode *prev );
		virtual ~AddNodeCmd();
		bool	Execute();
		bool	UnExecute();

	protected:
		SNode				*pSrc;
		SNode				*pHanging;
		SNode				*pParent;
		SNode				*pPrev;
	};

	class DeleteNodeCmd: public ICommand
	{
	public:
		DeleteNodeCmd( SNode *src );
		virtual ~DeleteNodeCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pSrc;
		SNode				*pHanging;
		SNode				*pParent;
		SNode				*pPrev;
		bool				bExpand;
		bool				bFirstChild;
	};

	class RenameNodeCmd: public ICommand
	{
	public:
		RenameNodeCmd( SNode* src, const wstring& exchangeName );
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode*				m_pSrc;
		wstring				m_exchangeName;
	};

	class RenameFunctionCmd : public RenameNodeCmd
	{
	public:
		RenameFunctionCmd(SNode* src, const wstring& exchangeName, const wstring& exchangeDesc);
		virtual bool		Execute();
	protected:
		wstring				m_exchangeDesc;
	};

	class PasteNodeCmd: public ICommand
	{
	public:
		PasteNodeCmd( SNode *src, SNode* dstParent, SNode *dstPrev, bool bCut = false, SNode *orgParent = NULL, SNode *orgPrev = NULL );
		virtual ~PasteNodeCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pSrc;
		SNode				*pDstParent;
		SNode				*pDstPrev;
		bool				bCut;
		SNode				*pOrgParent;
		SNode				*pOrgPrev;
		SNode				*pHanging;
	};

	class ExChangeNodeCmd: public ICommand
	{
	public:
		ExChangeNodeCmd( SNode *bein, SNode* beout );
		virtual ~ExChangeNodeCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pBeIn;
		SNode				*pBeOut;
		SNode				*pParent;
		SNode				*pPrev;
		bool				bFirstChild;
	};

	class ExChangeNodeChildCmd: public ICommand
	{
	public:
		ExChangeNodeChildCmd( SNode *bein, SNode* beout );
		virtual ~ExChangeNodeChildCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pBeIn;
		SNode				*pBeOut;
		SNode				*pParent;
		SNode				*pPrev;
	};

	class ExChangeNodeContentCmd: public ICommand
	{
	public:
		ExChangeNodeContentCmd( SNode *bein, SNode* beout );
		virtual ~ExChangeNodeContentCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pBeIn;
		SNode				*pBeOut;
		SNode				*pParent;
		SNode				*pTemp;
	};

	class ExChangeOperateNodeCmd: public ICommand
	{
	public:
		ExChangeOperateNodeCmd( SNode *src );
		virtual ~ExChangeOperateNodeCmd();
		virtual bool		Execute();
		virtual bool		UnExecute();
	protected:
		SNode				*pSrc;
	};
}