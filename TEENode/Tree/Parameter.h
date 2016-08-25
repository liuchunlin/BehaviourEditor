#pragma once 

#include "NodeBase.h"

namespace TEE
{
	class Parameter: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Parameter();
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual	bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
		virtual ErrorMask		CheckError();
		virtual wstring			GetReadableString() const;
		virtual bool			GetOptionCode( wstring& valueType );
		virtual	bool			GetCPPCode( wstring& cppCode ) const;
	protected:
		wstring					GetCppTypeCode() const;
	public:
		wstring					m_DefaultValue;
	}; 

	class ContextParam: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		ContextParam();
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual	bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual ErrorMask			CheckError();
		virtual wstring			GetReadableString() const;
		virtual bool			GetOptionCode( wstring& valueType );
	}; 

	class ContextRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		ContextRoot();
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual wstring			GetReadableString() const;
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }
	};

}