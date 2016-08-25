#include "SParameter.h"

#include "NodeVerify.h"
#include "TEEManager.h"
using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SPreferences
IMPLEMENT_BLOCK_ALLOCATE(SParameter, 32)
SParameter::SParameter()
{
	ClassType(SNT_PARAMETER);
}

TiXmlElement*	SParameter::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	if( TEETmp() != NULL )
	{
		pXmlElement->SetAttribute( "Op", FTCHARToUTF8(TEETmp()->CodeName().c_str()) );
	}

	return pXmlElement;
}
bool	SParameter::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	wstring opValue;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Op", opValue );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindOptionFromCode(opValue);
	assert( pTmpNode != NULL );

	TEETmp( pTmpNode );

	return true;
}

bool	SParameter::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = false;
	cnxt.bPreferences = true;
	cnxt.bEnable = TEETmp() != NULL;

	SNode* pChild = FirstChild();
	for( ; pChild != NULL; pChild = pChild->Next() )
	{
		pChild->GetContextView( cnxt );
	}

	if( cnxt.text.empty() && TEETmp() != NULL )
	{
		TEE::GTEEMgr->GetParameterReadName( TEETmp(), cnxt.text );
	}
	if( cnxt.text.empty() )
	{
		cnxt.text = Name();
	}

	return true;
}
void	SParameter::GetContextViews( vector<ContextView>& cnxts )
{
	ContextView cnxt;
	GetContextView( cnxt );
	cnxts.push_back( cnxt );
}

wstring	SParameter::GetReadText() const
{
	wstring name;
	if( !TEE::GTEEMgr->GetParameterReadName( TEETmp(), name ) )
		name = Name();
	return name;
}
ErrorMask	SParameter::CheckError()
{
	//AddValidity( Space::VS_REFERENCE_ERR );
	return GetErrorMask();
}
