#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class IncludeRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		IncludeRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );

		//Operation
		virtual bool			Propertyable();
		virtual bool			Editable(){ return false; }
	};
	class Include: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Include();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual wstring			GetReadableString() const;
		virtual ErrorMask		CheckError();

		virtual bool			Deleteable();
		virtual bool			Dragable(){ return true; }
		virtual bool			Editable(){ return false; }
	public:
	};
}