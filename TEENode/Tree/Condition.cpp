#include "Condition.h"
#include "NodeFactory.h"
#include "TEEManager.h"
using namespace TEE;

//////////////////////////////////////////////////////////////////////////
//ConditionRoot
IMPLEMENT_BLOCK_ALLOCATE(ConditionRoot, 1)
ConditionRoot::ConditionRoot()
{
	m_classType = (NT_CONDITIONROOT);
}

TiXmlElement*	ConditionRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool			ConditionRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	ConditionRoot::NewConditionable()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Condition
IMPLEMENT_BLOCK_ALLOCATE(Condition, 64)
Condition::Condition()
{
	m_classType = (NT_CONDITION);
}
bool	Condition::FromString( const wstring& content )
{
	RemoveChildren();
	GNodeFactory.BuildNormalChild( content, this );
	NodeBase::CheckErrorTree(this);

	return true;
}


TiXmlElement*	Condition::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool			Condition::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
ErrorMask	Condition::CheckError()
{
	ClearErrorFlag( ERR_EVERYTHING );

	ErrorMask flag = GTEEMgr->CheckTagValidate( m_tag );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckParametersValidate( this );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckNameValidate( this );
	AddErrorFlag( flag );
	return GetErrorMask();
}
bool	Condition::GetNotesViewString( list<TEE::RichText>& records )
{
	RichText record;

	record.content = _T("Ô¤ÀÀ: ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );
	
	record.Clear();
	record.content = _T("±êÇ©: ");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );

	GTEEMgr->GetTagViewString( records, m_tag );

	return NodeBase::GetNotesViewString( records );
}
bool	Condition::NewConditionable()
{
	return true;
}
bool	Condition::Deleteable()
{
	return true;
}
bool	Condition::Propertyable()
{
	return true;
}
bool	Condition::GetOptionCode( wstring& valueType )
{
	valueType = _T("CONDITION");
	return true;
}