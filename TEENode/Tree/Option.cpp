#include "Option.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "Adjust.h"
using namespace TEE;
//////////////////////////////////////////////////////////////////////////
//OptionRoot
IMPLEMENT_BLOCK_ALLOCATE(OptionRoot, 16)
OptionRoot::OptionRoot()
{
	m_classType = (NT_OPTIONROOT);
}
TiXmlElement*	OptionRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool	OptionRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	OptionRoot::ToRichString( list<TEE::RichText>& records )
{
	return true;
}
bool	OptionRoot::GetNotesViewString( list<TEE::RichText>& records )
{
	NodeBase* pChile = FirstChild();
	for( ; pChile != NULL; pChile = pChile->Next() )
	{
		pChile->GetNotesViewString( records );
	}
	return true;
}
bool	OptionRoot::NewOptionable()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Option
IMPLEMENT_BLOCK_ALLOCATE(Option, 64)
Option::Option()
{
	m_classType = (NT_OPTION);
	m_eValueType = VT_INT;
}

bool	Option::FromString( const wstring& content )
{
	RemoveChildren();
	GNodeFactory.BuildOptionChild( content, this );
	NodeBase::CheckErrorTree(this);
	return true;
}

TiXmlElement*	Option::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	//pXmlElement->SetAttribute( "Type", (int)m_eValueType );
	pXmlElement->SetAttribute( "Type", FTCHARToUTF8( GetValueTypeName(m_eValueType).second.c_str()) );
	return pXmlElement;
}

bool	Option::FromXMLElement( const TiXmlElement* pXMLElement )
{
	wstring valueType;
	GetXMLElementAttribute( pXMLElement, "Type", valueType );
	m_eValueType = GetValueTypeEnum( valueType );
	//GetXMLAttributeInfo<int>( pXMLElement, "Type", *(int*)(&m_eValueType) );

	return NodeBase::FromXMLElement( pXMLElement );
}

NodeBase* Option::DoClone() const
{
	Option* pClone = (Option*)NodeBase::DoClone();
	pClone->m_eValueType = m_eValueType;
	return pClone;
}

bool	Option::ToRichString( list<TEE::RichText>& records )
{
	NodeBase::ToRichString( records );
	return true;
}
bool	Option::GetNotesViewString( list<TEE::RichText>& records )
{
	const wstring& valueName = GetValueTypeName( m_eValueType ).first;
	RichText record;

	record.content = _T("‘§¿¿: ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );

	record.Clear();
	record.bUnderLine = false;
	record.eColor = TEE::RC_BLUE;
	record.content = m_DisplayName + _T("<") + m_CodeName + _T(">") + _T(":") + valueName + _T("\n");
	records.push_back(record);

	NodeBase::GetNotesViewString( records );
	return true;
}

ErrorMask	Option::CheckError()
{
	ClearErrorFlag(ERR_EVERYTHING);

	ErrorMask flag = GTEEMgr->CheckTagValidate( m_tag );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckNameValidate( this );
	AddErrorFlag( flag );
	return GetErrorMask();
}
bool	Option::NewOptionable()
{
	return true;
}
bool	Option::Deleteable()
{
	return true;
}
bool	Option::Propertyable()
{
	return true;
}
bool	Option::GetOptionCode( wstring& valueType )
{
	valueType = CodeName();
	return true;
}
wstring	Option::GetReadableString() const
{
	return CodeName();
}
