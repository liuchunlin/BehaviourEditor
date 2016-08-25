#pragma once

#include "SNode.h"

namespace Space
{
	class SMultiIF: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SMultiIF();
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

	class SIF: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SIF();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_MULTIPLE_IF; }
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};

	class SThen: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SThen();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_MULTIPLE_IF; }
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};

	class SElse: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SElse();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_MULTIPLE_IF; }
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};
}