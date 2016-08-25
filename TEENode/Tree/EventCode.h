#pragma once 


#include "NodeBase.h"

namespace TEE
{
	class EventCode: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		EventCode();
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
	public:
		wstring					m_strCode;
	};

	class MsgEventCode: public EventCode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		MsgEventCode();
		virtual bool			ToRichString( list<TEE::RichText>& records );
	};
}