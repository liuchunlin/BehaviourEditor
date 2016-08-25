#pragma once 

#include "NodeBase.h"

namespace TEE
{
	class TagRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		TagRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			FromString( const wstring& content = _T("") );
		virtual bool			GetNotesViewString(list<TEE::RichText>& records);
		virtual bool			Viewable(){ return true; }
		virtual ErrorMask		CheckError();
	};
}