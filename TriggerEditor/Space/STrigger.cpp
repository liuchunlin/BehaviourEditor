#include "STrigger.h"
#include "SpaceManager.h"

using namespace Space;
//////////////////////////////////////////////////////////////////////////
//STrigger
IMPLEMENT_BLOCK_ALLOCATE(STrigger, 64)

STrigger::STrigger()
	: m_objlink(this)
{
	ClassType(SNT_TRIGGER);
}

STrigger::~STrigger()
{
	DWORD id = reinterpret_cast<DWORD>(UserData( "id" ) );
	Space::GSpaceMgr->RemoveTriggerID( id );
}
TiXmlElement*	STrigger::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );
	pXmlElement->SetAttribute( "id", reinterpret_cast<DWORD>(UserData("id")) );

	return pXmlElement;
}
bool	STrigger::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );

	int id = 0;
	pXMLElement->Attribute( "id", &id );
	UserData( "id", reinterpret_cast<void*>(id) );
	if (!Space::GSpaceMgr->AddTriggerID(id))
	{
		UserData( "id", reinterpret_cast<void*>(Space::GSpaceMgr->CreateTriggerID()) );
	}
	return true;
}
bool	STrigger::Cutable()
{
	return true;
}
bool	STrigger::Copyable()
{
	return true;
}
bool	STrigger::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	return true;
}
bool	STrigger::Deleteable()
{
	return true;
}
bool	STrigger::UniqueName() const
{
	return true;
}
