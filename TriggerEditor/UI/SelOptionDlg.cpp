#include "SelOptionDlg.h"
#include "wx/sizer.h"
#include "wx/imaglist.h"
#include "wx/listctrl.h"
#include "wx/spinctrl.h"
#include "EnumType.h"
#include "TgrIDs.h"
#include "CmdManager.h"
#include "SpaceManager.h"
#include "TEEManager.h"
#include "NodeVerify.h"
#include "CancelableWindow.h"

BEGIN_EVENT_TABLE(SelOptionDlg, wxDialog)
EVT_CHOICE(TGR_SELECT_OPTION, SelOptionDlg::OnVarTypeChoice)
EVT_BUTTON(wxID_ANY, SelOptionDlg::OnButton)
END_EVENT_TABLE()

SelOptionDlg::SelOptionDlg(wxWindow *parent, Space::SNode* pOrgNode)
	:wxDialog(parent, wxID_ANY, _T("类型选择"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
	, m_pOrgNode(pOrgNode)
	, m_pWorkNode(NULL)
	, m_pTEETmp(NULL)
{
	InitControls();
	InitContents();
}

SelOptionDlg::~SelOptionDlg()
{
}

void	SelOptionDlg::InitContents()
{
	vector<TEE::NodeBase*> Options;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTION, Options);
	vector<TEE::NodeBase*>::iterator iter(Options.begin());
	for (; iter != Options.end(); ++iter)
	{
		TEE::NodeBase* pNode = (*iter);
		if (Space::GSpaceMgr->IsInternalType(pNode->CodeName()))
			continue;
		m_pVarType->Append(pNode->DisplayName(), reinterpret_cast<void*>(pNode));
	}
	if (m_pOrgNode != NULL)
	{
		m_pVarType->SetStringSelection(m_pOrgNode->TEETmp()->DisplayName());
	}

	m_pWorkNode = Space::SNode::Clone(m_pOrgNode);
}
void	SelOptionDlg::InitControls()
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *subsizer;

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(new wxStaticText(this, wxID_ANY, _T("变量类型(&T):"), wxDefaultPosition, wxSize(100, 20)), 0, wxALL, 5);
	m_pVarType = new wxChoice(this, TGR_SELECT_OPTION, wxDefaultPosition, wxSize(140, 20), 0, NULL, wxCB_SORT);
	subsizer->Add(m_pVarType, 1, wxALL, 3);
	sizer->Add(subsizer, 0, wxALL | wxEXPAND, 0);

	//
	subsizer = new wxBoxSizer(wxHORIZONTAL);
	m_pOk = new wxButton(this, wxID_OK, _T("确定"));
	m_pOk->SetDefault();
	subsizer->Add(m_pOk, 0, wxALIGN_RIGHT | wxALL, 3);
	wxButton *cancel = new wxButton(this, wxID_CANCEL, _T("取消"));
	subsizer->Add(cancel, 0, wxALIGN_RIGHT | wxALL, 3);
	sizer->Add(subsizer, 0, wxALL | wxALIGN_RIGHT, 3);

	//
	SetSizer(sizer);
	sizer->Fit(this);
}

extern CancelableWindow* GetParentCancelableWindow(wxWindow* window);

Space::SNode*	SelOptionDlg::EditShowModal()
{
	if (m_pWorkNode == NULL)
	{
		return NULL;
	}

	if (ShowModal() == wxID_OK)
	{
		TEE::NodeBase* pTEETmp = reinterpret_cast<TEE::NodeBase *>(m_pVarType->GetClientData(m_pVarType->GetSelection()));
		m_pWorkNode->TEETmp(pTEETmp);

		intptr_t docID = (intptr_t)-1;
		if (CancelableWindow* cancelableWindow = GetParentCancelableWindow(GetParent()))
		{
			docID = cancelableWindow->GetDocumentID();
		}
		GCmdMgr->AddCommand(docID, new Space::ExChangeNodeContentCmd(m_pOrgNode, m_pWorkNode));
		m_pWorkNode = NULL;
	}
	else
	{
		if (m_pWorkNode != NULL)
		{
			SAFE_DELETE(m_pWorkNode);
		}
		return NULL;
	}
	return m_pOrgNode;
}

void	SelOptionDlg::OnVarTypeChoice(wxCommandEvent &event)
{
	if (m_pWorkNode)
	{
		TEE::NodeBase *pTEETmp = reinterpret_cast<TEE::NodeBase *>(m_pVarType->GetClientData(m_pVarType->GetSelection()));
		m_pWorkNode->TEETmp(pTEETmp);
	}
}

void	SelOptionDlg::OnButton(wxCommandEvent &event)
{
	this->EndModal(event.GetId());
	event.Skip();
}
