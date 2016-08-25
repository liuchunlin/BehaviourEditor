#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class SpaceRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SpaceRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual bool			NewSpaceable();
		virtual bool			Deleteable(){ return true; }
		virtual bool			Propertyable(){ return true; }
		virtual bool			Includeable(){ return true; }
		virtual bool			Editable(){ return false; }
	};

}