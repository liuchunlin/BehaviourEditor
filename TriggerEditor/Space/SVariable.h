#pragma once

#include "SNode.h"
#include "NodeBase.h"

#define LOCAL_VAR_PREFIX TEXT("l_")
#define GLOBAL_VAR_PREFIX TEXT("g_")
#define PARAM_VAR_PREFIX TEXT("param_")

namespace Space
{
	//��������
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

	//�����б���
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

	//����
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

	//�¼���������
	class SContextVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SContextVarRef();
		virtual bool			PostXMLLoaded();
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual ErrorMask	CheckError();
	};

	//ȫ�ֱ���
	class SGlobalVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SGlobalVarRef();
		virtual bool			PostXMLLoaded();
	};

	//�ֲ�����
	class SLocalVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SLocalVarRef();
		virtual bool			PostXMLLoaded();
	};

	//��������
	class SParamVarRef: public SVarRef
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SParamVarRef();
		virtual bool			PostXMLLoaded();
	};

	//���ñ���
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