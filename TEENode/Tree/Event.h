#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class EventRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		EventRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			NewEventable();
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }
	};
	class Event: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Event();
		virtual bool			FromString( const wstring& content = _T("") );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual ErrorMask		CheckError();
		virtual bool			GetOptionCode( wstring& valueType );

		//Operation
		virtual bool			NewEventable();
		virtual bool			Deleteable();
		virtual bool			Propertyable();
		virtual bool			Dragable(){ return true; }
	};
}