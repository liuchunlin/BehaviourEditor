#pragma once
#include "wx/wxprec.h"
#include "wx/wx.h"

class wxListCtrl;
class wxListEvent;
class GWTextCtrl;
class wxChoice;
class wxCheckBox;

namespace Space
{
	class SNode;
}
namespace TEE
{
	class NodeBase;
}

enum ETriggerIcon
{
	TriggerIcon_File = 0,
	TriggerIcon_Folder,
	TriggerIcon_FolderOpen,
	TriggerIcon_Action,
	TriggerIcon_Event,
	TriggerIcon_Condition,
	TriggerIcon_Variable,
	TriggerIcon_Branch,
	TriggerIcon_Loop,
	TriggerIcon_Return,
	TriggerIcon_Trigger,
	TriggerIcon_Function,
};

enum EPasteSrcType
{
	PST_COPY,
	PST_CUT,
	PST_DRAG,
};