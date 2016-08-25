#pragma once

#include "SNode.h"

namespace Space
{
	class SWhile: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SWhile();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_ACTIONROOT; }
		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual	bool			Deleteable();
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};
}