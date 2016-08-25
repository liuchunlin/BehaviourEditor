#pragma once 


#include "NodeBase.h"

namespace TEE
{
	struct Adjust: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Adjust();

		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		TiXmlElement*			ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			GetOptionCode( wstring& valueType );
		virtual wstring			GetReadableString() const;

	public:
	};

}