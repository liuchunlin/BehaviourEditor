#include "ParameterEditor.h"

#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"

#include "GWHyperlink.h"

void ParamEditor::Build( wxWindow* pParent, wxPoint beginPos, wxSize allSize, Space::SNode *pSNode, vector<wxControl*>& ctrls )
{
	vector<wxControl*>::iterator iterCtrl(ctrls.begin());
	for( ; iterCtrl != ctrls.end(); ++iterCtrl )
	{
		SAFE_DELETE( (*iterCtrl) );
	}
	ctrls.clear();

	if( pParent == NULL )
		return;
	if( pSNode == NULL )
	{
		assert( false );
		return ;
	}
	vector<Space::ContextView>	cnxts;
	pSNode->GetContextViews( cnxts );

	wxPoint currentPos = beginPos;

	vector<Space::ContextView>::iterator iter(cnxts.begin());
	for( ; iter != cnxts.end(); ++iter )
	{
		const Space::ContextView cnt = (*iter);

		int w=0,h=0;

		pParent->GetTextExtent(cnt.text.c_str(), &w, &h);
		if( currentPos.x + w - beginPos.x > allSize.x )
		{
			currentPos.x = beginPos.x;
			currentPos.y += 16;
		}

		wxControl* pControl = NULL;
		if( cnt.bPreferences )
		{
			GWHyperlinkCtrl* pHyperlink = new GWHyperlinkCtrl( pParent, TGR_MAIN_HYPERLINK, cnt.text, currentPos, wxSize(w, h) );
			pHyperlink->SetClientData( static_cast<void*>(cnt.pNode) );
			if( !cnt.bValid )
			{
				pHyperlink->SetNormalColour( cnt.bEnable ? *wxRED : *wxLIGHT_GREY );
			}
			pHyperlink->Enable( cnt.bEnable );
			pControl = pHyperlink;
		}
		else
		{
			pControl = new wxStaticText( pParent, TGR_MAIN_HYPERLINK, cnt.text, currentPos, wxSize(w, h) );
		}
		if( pControl != NULL )
		{
			ctrls.push_back( pControl );
		}

		currentPos.x += w+4;
	}
}
void	ParamEditor::Resize( wxPoint beginPos, wxSize allSize, const vector<wxControl*>& ctrls )
{
	wxPoint currentPos = beginPos;

	vector<wxControl*>::const_iterator iterCtrl(ctrls.begin());
	for( ; iterCtrl != ctrls.end(); ++iterCtrl )
	{
		wxControl *pControl = (*iterCtrl);
		wxSize size = pControl->GetSize();
		if( currentPos.x-beginPos.x + size.x > allSize.x )
		{
			currentPos.x = beginPos.x;
			currentPos.y += 16;
		}

		pControl->SetPosition( currentPos );
		pControl->Show();

		currentPos.x += size.x+4;
	}
}
