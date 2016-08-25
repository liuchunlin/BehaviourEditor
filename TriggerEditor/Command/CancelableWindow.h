#pragma once
#include "wx/wx.h"
#include "CmdManager.h"

class CancelableWindow
{
public:
	virtual intptr_t GetDocumentID() = 0;

	bool Undo()
	{
		return GCmdMgr->UnDo(GetDocumentID());
	}
	bool Redo()
	{
		return GCmdMgr->ReDo(GetDocumentID());
	}
	bool CanUnDo()
	{
		return GCmdMgr->CanUnDo(GetDocumentID());
	}
	bool CanReDo()
	{
		return GCmdMgr->CanReDo(GetDocumentID());
	}
};