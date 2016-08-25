#include "OptionEditDlg.h"
#include "EnumType.h"
#include "TgrIDs.h"
#include "GWTextCtrl.h"
#include "GWHyperlink.h"
#include "EditTree.h"
#include "CmdManager.h"
#include "SpaceManager.h"
#include "TEEManager.h"
#include "NodeVerify.h"
#include "wx/sizer.h"
#include "wx/richtext/richtextctrl.h"

BEGIN_EVENT_TABLE(OptionEditDlg, wxDialog)
EVT_RICHTEXT_CONTENT_INSERTED(TGR_OPTION_EDIT, OptionEditDlg::OnEditNotesChanged)
EVT_RICHTEXT_CONTENT_DELETED(TGR_OPTION_EDIT, OptionEditDlg::OnEditNotesChanged)
END_EVENT_TABLE()

OptionEditDlg::OptionEditDlg(wxWindow *parent, TEE::NodeBase* pOrgNode)
	: wxDialog(parent, wxID_ANY, TEXT("编辑选项"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
	, m_pECAEdit(nullptr)
	, m_pOrgNode(pOrgNode)
	, m_pEditCopy(nullptr)
{
	InitControls();
	InitContents();
}

OptionEditDlg::~OptionEditDlg()
{
	SAFE_DELETE(m_pEditCopy);
}

void	OptionEditDlg::InitControls()
{
	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_pECAEdit = new wxRichTextCtrl(this, TGR_OPTION_EDIT, wxEmptyString, wxDefaultPosition, wxSize(200, 300), wxHSCROLL | wxVSCROLL | wxSUNKEN_BORDER | wxWANTS_CHARS);
	sizer->Add(m_pECAEdit, 1, wxEXPAND | wxRIGHT, 10);

	wxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* ok = new wxButton(this, wxID_OK, _T("确定"));
	ok->SetDefault();
	subsizer->Add(ok, 0, wxALIGN_RIGHT | wxALL, 3);

	wxButton* cancel = new wxButton(this, wxID_CANCEL, _T("取消"));
	subsizer->Add(cancel, 0, wxALIGN_RIGHT | wxALL, 3);

	sizer->Add(subsizer, 0, wxALL | wxALIGN_RIGHT, 3);

	SetSizer(sizer);
	sizer->Fit(this);
}

void OptionEditDlg::InitContents()
{
	if (m_pOrgNode == NULL)
		return;

	m_pEditCopy = TEE::NodeBase::Clone(m_pOrgNode);
	list<TEE::RichText> records;
	m_pOrgNode->ToRichString(records);

	for (list<TEE::RichText>::iterator iter = records.begin(); iter != records.end(); ++iter)
	{
		const TEE::RichText& record = (*iter);
		m_pECAEdit->WriteText(record.content);
	}

	m_pECAEdit->SetModified(false);
}

void	OptionEditDlg::OnEditNotesChanged(wxRichTextEvent& event)
{
/*
	pNode->FromString(m_pECAEdit->GetValue().wc_str());
	RefreshTEEValidate(m_pECATree->GetRootItem());
	RefreshNodeView(selItemId);
	SetChanged(true);
*/

}

bool OptionEditDlg::DoShowModal()
{
	if (m_pOrgNode == nullptr || m_pOrgNode->ClassType() != TEE::NT_OPTION)
		return false;
	if (ShowModal() == wxID_OK)
	{
		m_pOrgNode->FromString(m_pECAEdit->GetValue().wc_str());
		TEE::RootNode* root = dynamic_cast<TEE::RootNode*>(m_pOrgNode->Ancestor(TEE::NT_ROOT));
		root->Save();
		Space::GSpaceMgr->ChiefRoot()->FixTEETmp();
		Space::SNode::CheckErrorTree(Space::GSpaceMgr->ChiefRoot());
		return true;
	}
	return false;
}