#include "TagDialogs.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxUSE_SPINCTRL
#include "wx/spinctrl.h"
#endif
#include "GWIDs.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "Label.h"
#include <regex>

IMPLEMENT_CLASS(TagDialog, wxDialog)

BEGIN_EVENT_TABLE(TagDialog, wxDialog)
EVT_BUTTON(wxID_ANY, TagDialog::OnButton)
EVT_CHECKLISTBOX(CTRL_TEE_TAG_CHECKBOX, TagDialog::OnCheckboxToggle)
END_EVENT_TABLE()

TagDialog::TagDialog( )
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create( NULL, wxID_ANY, _("Tag"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );

	wxBoxSizer *pTopSizer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer *pBox1 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *pBox2 = new wxBoxSizer( wxVERTICAL );

	m_pTagListBox = new wxCheckListBox(	this, CTRL_TEE_TAG_CHECKBOX, wxPoint(10, 10), wxSize(200, 150));

	pBox1->Add( m_pTagListBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pTopSizer->Add( pBox1, 0, wxGROW|wxALL, 0 );

	wxButton *btn = new wxButton(this, CTRL_TEE_TAG_OK, _T("È·¶¨"));
	pBox2->Add( btn, 0, wxALL|wxALIGN_CENTER, 5 );

	pTopSizer->Add( pBox2, 0, wxGROW|wxALL, 0 );

	SetSizer(pTopSizer);
	pTopSizer->Fit(this);
}

TagDialog::~TagDialog()
{
}


void	TagDialog::Init( const wxString& tags )
{
	m_Tag = tags;

	vector<TEE::NodeBase*> TagRoots;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_TAGROOT, TagRoots);

	vector<TEE::NodeBase*> Tags;
	TEE::GTEEMgr->FindChildren(TagRoots, TEE::VerifyNodeType(TEE::NT_LABEL), Tags);

	vector<TEE::NodeBase*>::iterator iter(Tags.begin());
	for( int iIndex = 0; iter != Tags.end(); ++iter, ++iIndex )
	{
		const TEE::Label* pNode = (TEE::Label*)(*iter);
		if( !pNode->m_strLabel.empty() )
		{
			m_pTagListBox->Append( pNode->m_strLabel );
			if( m_Tag == pNode->m_strLabel )
			{
				m_pTagListBox->Check( iIndex );
			}
		}
	}
}

void	TagDialog::OnButton(wxCommandEvent& event)
{
	if( event.GetId() == CTRL_TEE_TAG_OK )
	{
		m_Tag.clear();

		wxArrayInt aSelections;
		m_pTagListBox->GetCheckedItems(aSelections);
		if (aSelections.Count() > 0)
			m_Tag = m_pTagListBox->GetString(aSelections.front());

		this->EndModal( CTRL_TEE_TAG_OK );
	}
	event.Skip();
}

void	TagDialog::OnCheckboxToggle(wxCommandEvent& event)
{
	unsigned int nToggleItemId = event.GetInt();

	if (m_pTagListBox->IsChecked(nToggleItemId))
	{
		for (unsigned int item = 0; item < m_pTagListBox->GetCount(); ++item)
		{
			if (item != nToggleItemId)
				m_pTagListBox->Check(item, false);
		}
	}
}
