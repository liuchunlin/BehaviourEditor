#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class OptionRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		OptionRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );

		//Operation
		virtual bool			NewOptionable();
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }
	};

	class Option: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Option();
		virtual bool			FromString( const wstring& content = _T("") );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
		virtual bool			ToRichString(list<TEE::RichText>& records);
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual ErrorMask		CheckError();
		virtual bool			GetOptionCode( wstring& valueType );
		virtual wstring			GetReadableString() const;

		//Operation
		virtual bool			NewOptionable();
		virtual bool			Deleteable();
		virtual bool			Propertyable();
		virtual bool			Dragable(){ return true; }
	public:
		VALUE_TYPE				m_eValueType;
	};


}