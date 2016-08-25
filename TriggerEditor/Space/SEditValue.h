#pragma once

#include "SNode.h"
#include "NodeBase.h"

namespace Space
{
	class SEditValue: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SEditValue();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual bool			GetRealParam( wstring& realParam ) const;
		virtual bool			GetLuaCode( wstring& luaCode, int depth );
		virtual bool			Paramerterable(){ return true; }
	private:
	};
}