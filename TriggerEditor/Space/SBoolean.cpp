#include "SBoolean.h"
#include "NodeVerify.h"
#include "TEEManager.h"

using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SBoolean
IMPLEMENT_BLOCK_ALLOCATE(SBoolean, 8)
SBoolean::SBoolean()
{
	ClassType(SNT_BOOLEAN);
}
TiXmlElement*	SBoolean::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	if( TEETmp() != NULL )
	{
		pXmlElement->SetAttribute( "Op", FTCHARToUTF8(TEETmp()->CodeName().c_str()) );
	}

	return pXmlElement;
}
bool	SBoolean::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	wstring opValue;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Op", opValue );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindOptionFromCode(opValue);
	assert( pTmpNode != NULL );

	TEETmp( pTmpNode );
	return true;
}

bool	SBoolean::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}

wstring	SBoolean::GetReadText() const
{
	if( Name() != _T("0") )
		return _T("Õæ");
	else
		return _T("¼Ù");
}
ErrorMask	SBoolean::CheckError()
{
	assert( GetErrorMask() == VS_OK );

	return GetErrorMask();
}

bool	SBoolean::GetLuaCode( wstring& luaCode, int depth )
{
	if( Name() != _T("0") )
	{
		luaCode += _T("true");
	}
	else
	{
		luaCode += _T("false");
	}
	return true;
}

bool	SBoolean::GetRealParam( wstring& realParam ) const
{
	return const_cast<SBoolean*>(this)->GetLuaCode(realParam, 0);
}