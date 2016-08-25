#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class Label: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Label();
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
		virtual wstring			GetReadableString() const;
		virtual	bool			GetCPPCode( wstring& cppCode ) const;
	public:
		void					AddExtraNotesContent( wstring& content );;
		wstring					m_strLabel;
	};
}