#include "SEditValue.h"
#include "NodeVerify.h"
#include "TEEManager.h"

using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SEditValue
IMPLEMENT_BLOCK_ALLOCATE(SEditValue, 32)
SEditValue::SEditValue()
{
	ClassType(SNT_EDIT_VALUE);
}
TiXmlElement*	SEditValue::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	if( TEETmp() != NULL )
	{
		pXmlElement->SetAttribute( "Code", FTCHARToUTF8(TEETmp()->CodeName().c_str()) );
	}

	return pXmlElement;
}
bool	SEditValue::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	wstring editValue;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Code", editValue );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindOptionFromCode(editValue);
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	return true;
}

bool	SEditValue::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}

wstring	SEditValue::GetReadText() const
{
	wstring text;
	GetRealParam( text );
	return text;
}
ErrorMask	SEditValue::CheckError()
{
	assert( GetErrorMask() == VS_OK );

	if( Name().empty() )
	{
		AddErrorFlag( VS_EDIT_VALUE_ERR );
	}
	TEE::NodeBase* pTEETmp = TEETmp();
	if( pTEETmp == NULL )
	{
		AddErrorFlag( VS_TEETEMP_ERR );
	}
	
	if( pTEETmp == NULL || !TEE::GTEEMgr->CheckEditValueValidate( pTEETmp, Name() ) )
	{
		AddErrorFlag( VS_EDIT_VALUE_ERR );
	}

	return GetErrorMask();
}
bool	SEditValue::GetRealParam( wstring& realParam ) const
{
	if( TEETmp() == NULL )
	{
		realParam = Name();
	}
	else
	{
		switch(TEE::GetValueTypeEnum( TEETmp()->CodeName() ))
		{
		case TEE::VT_INT:
		case TEE::VT_REAL:
			realParam = Name();
			break;
		case TEE::VT_STRING:
			realParam += _T("\"");
			realParam += Name();
			realParam += _T("\"");
			break;
		default:
			assert(false);
			break;
		}
	}

	return true;
}
bool	SEditValue::GetLuaCode( wstring& luaCode, int depth )
{
	return GetRealParam( luaCode );
}
