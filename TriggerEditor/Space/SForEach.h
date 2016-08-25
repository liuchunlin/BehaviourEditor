#pragma once

#include "SNode.h"

namespace Space
{
	class SForEach: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SForEach();
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

	class SLoop: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SLoop();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_FOR_EACH; }
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};


}