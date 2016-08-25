#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class ConditionRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		ConditionRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );

		//Operation
		virtual bool			NewConditionable();
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }
	};
	class Condition: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Condition();
		virtual bool			FromString( const wstring& content = _T("") );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual ErrorMask		CheckError();
		bool					GetNotesViewString( list<TEE::RichText>& records );
		virtual bool			GetOptionCode( wstring& valueType );

		//Operation
		virtual bool			NewConditionable();
		virtual bool			Deleteable();
		virtual bool			Propertyable();
		virtual bool			Dragable(){ return true; }
	};
}