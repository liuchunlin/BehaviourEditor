#include "Tag.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "Adjust.h"
#include "NodeVerify.h"

using namespace TEE;

IMPLEMENT_BLOCK_ALLOCATE(TagRoot, 1)

TagRoot::TagRoot()
{
	m_classType = (NT_TAGROOT);
}

TiXmlElement*	TagRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}

bool	TagRoot::FromXMLElement(const TiXmlElement* pXMLElement)
{
	return NodeBase::FromXMLElement(pXMLElement);
}

bool	TagRoot::FromString(const wstring& content)
{
	RemoveChildren();
	GNodeFactory.BuildTagChild(content, this);

	NodeBase* root = Ancestor(NT_ROOT);
	if (root) NodeBase::CheckErrorTree(root);
	return true;
}

bool	TagRoot::GetNotesViewString(list<TEE::RichText>& records)
{
	RichText record;
	record.content = _T("Ô¤ÀÀ: ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back(record);

	NodeBase::GetNotesViewString(records);
	return true;
}

ErrorMask	TagRoot::CheckError()
{
	ClearErrorFlag(ERR_EVERYTHING);
	for (NodeBase* child = FirstChild(); child; child = child->Next())
	{
		if (!child->IsA(NT_LABEL)) continue;
		child->ClearErrorFlag(ERR_EVERYTHING);
		wstring labelName = dynamic_cast<Label*>(child)->m_strLabel;

		vector<NodeBase*> TagRoots;
		GTEEMgr->GetNodesByType(NT_TAGROOT, TagRoots);
		vector<NodeBase*> tags;
		GTEEMgr->FindChildren(TagRoots, VerifyLabelName(labelName), tags);
		if (tags.size() > 1)
		{
			child->AddErrorFlag(ERR_REPETITION_NAME);
			AddErrorFlag(child->GetErrorMask());
		}
	}
	return GetErrorMask();
}
