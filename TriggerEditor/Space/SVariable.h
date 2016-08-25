#pragma once

#include "SNode.h"
#include "NodeBase.h"

#define LOCAL_VAR_PREFIX TEXT("l_")
#define GLOBAL_VAR_PREFIX TEXT("g_")
#define PARAM_VAR_PREFIX TEXT("param_")

namespace Space
{
	//变量设置
	class SVarSet: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SVarSet();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual wstring			GetReadText() const;
		virtual ErrorMask	CheckError();
	};

	//变量列表项
	class SVariable: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SVariable();
		~SVariable();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual wstring			GetReadText() const;
		virtual bool			GetLuaCode( wstring& luaCode, int depth );
		virtual ErrorMask		CheckError();
		virtual void			Name( const wstring& val );
		using SNode::Name;
		virtual	bool			Copyable() { return true; }
		virtual	bool			Cutable() { return true; }
		virtual bool			Pasteable(const SNode* pastingNode);
		wstring					GetDefaultValue();
	};

	//变量
	class SVarRef: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SVarRef();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual	void			GetContextViews( vector<ContextView>& cnxts );
		virtual	void			GetChildViews( vector<ContextView>& cnxts );
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual void			DominoOffect();
		virtual bool			GetLuaCode( wstring& luaCode, int depth );
		virtual bool			GetRealParam( wstring& realParam ) const;

		virtual bool			Paramerterable(){ return true; }
	};

	//事件环境参数
	class SContextVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SContextVarRef();
		virtual bool			PostXMLLoaded();
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual ErrorMask	CheckError();
	};

	//全局变量
	class SGlobalVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SGlobalVarRef();
		virtual bool			PostXMLLoaded();
	};

	//局部变量
	class SLocalVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SLocalVarRef();
		virtual bool			PostXMLLoaded();
	};

	//参数变量
	class SParamVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SParamVarRef();
		virtual bool			PostXMLLoaded();
	};

	//设置变量
	class SSetVariable: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SSetVariable();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual	void			GetContextViews( vector<ContextView>& cnxts );
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual SPACE_NODE_TYPE CommonParentType()const { return SNT_ACTIONROOT; }
		virtual bool			Pasteable( const SNode* pSrc );
		virtual	bool			Cutable() { return true; }
		virtual	bool			Copyable() { return true; }
		virtual bool			NewActionable() { return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	};
}