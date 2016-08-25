#pragma once

#include <Vek/Base/Types.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using namespace std;
#include "tinyXML/tinyxml.h"
#include "TgrIDs.h"
#include "XmlUtility.h"
#include "NodeBase.h"
#include <strsafe.h>
#include "Vek/Vek/Base/Tree.h"

namespace Space
{
	class	SNode;
	class SVerify;
	enum	SPACE_NODE_TYPE
	{
		SNT_NULL,				//无
		SNT_ROOT,				//空间根
		SNT_VARSET,				//变量设置
			SNT_VARIABLE,		//变量列表项
		SNT_GROUP,				//空间分组
		//------------------------------
		SNT_TRIGGER,			//触发器
			SNT_EVENTROOT,			//事件根
			SNT_EVENT,				//事件
			SNT_CONDITIONROOT,		//条件根
			SNT_CONDITION,			//条件
			SNT_ACTIONROOT,			//动作根
		//---------------------------
		SNT_FUNCTION,			//函数
			SNT_PARAMROOT,			//形参根
			SNT_RETURNTYPE,			//返回类型
			SNT_RETURN,				//返回语句
		//------------------------------
			SNT_MULTIPLE_IF,		//条件分支
				SNT_IF,				//条件判断
				SNT_THEN,			//条件合法动作
				SNT_ELSE,			//条件非法动作
			SNT_FOR_EACH,			//for循环
				SNT_LOOP,			//Loop
			SNT_SET_VARIABLE,		//设置变量
			SNT_WHILE,				//while循环
		//------------------------------
		SNT_PARAMETER,		//参数选择(Tmp)
		SNT_STATICLABEL,		//静态文本
		SNT_ADJUST,				//调整值(Tmp)
		SNT_GLOBAL_VARREF,			//全局变量(Tmp)
		SNT_LOCAL_VARREF,			//局部变量
		SNT_PARAM_VARREF,			//参数变量
		SNT_CONTEXT_VARREF,		//参数变量(Tmp)
		SNT_TFUNCCALL,			//TEE函数调用(Tmp)
		SNT_SFUNCCALL,			//Space函数调用
		SNT_BOOLEAN,			//布尔
		SNT_EDIT_VALUE,			//编辑

		//////////////////////////////////////////////////////////////////////////
		SNT_END,
	};
	enum EErrorFlag
	{
		VS_OK					= 0x00000000,		//正确
		VS_PARAM_PENDING		= 0x00000001,		//未被初始化
		VS_PARAM_NUMBER_ERR		= 0x00000002,		//参数数量错误
		VS_PARAM_TYPE_ERR		= 0x00000004,		//参数类型错误
		VS_REFERENCE_ERR		= 0x00000008,		//引用丢失或者类型不匹配
		VS_EDIT_VALUE_ERR		= 0x00000010,		//编辑的值错误
		VS_TEETEMP_ERR			= 0x00000020,		//TEE模板错误
		VS_NAME_FOMAT_ERR		= 0x00000040,		//名字格式错误
		VS_NAME_DUP_ERR			= 0x00000080,		//名字重复错误

		VS_EVERYTHING			= 0xFFFFFFFF,		//...
	};
	typedef	DWORD	ErrorMask;

	class ContextView
	{
	public:
		ContextView(){ bValid = false; bPreferences = false; bEnable = true; pNode = NULL; }
		wstring			text;			//阅读文本
		bool			bValid;			//是否有效
		bool			bPreferences;	//是否是可选参数
		bool			bEnable;		//是否可用
		SNode			*pNode;			//对应INode
	};

	class SNode : public Vek::TreeNode<SNode>
	{
	public:
		//static functions
		static TiXmlElement*	CreateNodeXml( SNode* pSrc, TiXmlElement* pXmlParent );
		static SNode*			Clone( const SNode* pSrc, SNode* pParent = NULL, bool bCloneNext = false );
		static ErrorMask		CheckErrorTree( SNode* pSrc, bool bRecursively = true );
		static void				PostLoadedSpace( SNode* pSrc );
		static bool				NodeTreesToString(const vector<SNode*>& Nodes, string& OutStr);
		static bool				StringToNodeTrees(const string& InStr, vector<SNode*>& OutNodes);

		static const char		EventParamName[];

	private:
		inline static ErrorMask	DoCheckErrorTree( SNode* pSrc, bool bRecursively = true );
	public:
		SNode();
		virtual ~SNode();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			PostXMLLoaded();
		virtual void			CopyFrom(const SNode* src);
		virtual void			DominoOffect() { }
		virtual void			FixTEETmp();
		virtual bool			IsStatementRoot(){ return false; }
		//Menu
		virtual	bool			Cutable() { return false; }
		virtual	bool			Copyable() { return false; }
		virtual	bool			Pasteable( const SNode* pSrc ) { return false; }
		virtual	bool			Deleteable() { return true; }
		virtual	bool			Disableable() { return true; }
		virtual bool			NewActionable() { return false; }
		virtual bool			Paramerterable(){ return false; }
		virtual bool			UniqueName() const { return false; }

		//
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual	void			GetContextViews( vector<ContextView>& cnxts );
		virtual	void			GetChildViews( vector<ContextView>& cnxts );
		virtual	wstring			GetReadText() const;
		virtual bool			GetRealParam( wstring& realParam ) const;
		virtual bool			GetLuaCode( wstring& luaCode, int depth );

		SPACE_NODE_TYPE			ClassType() const { return m_classType; }
		void					ClassType(SPACE_NODE_TYPE val) { m_classType = val; }
		bool					IsA(SPACE_NODE_TYPE val) const { return this != nullptr && m_classType == val; }
		virtual SPACE_NODE_TYPE CommonParentType() const { assert(false); return SNT_NULL; }
		std::string				TypeName();
		const wstring&			Name() const { return m_Name; }
		virtual void			Name( const wstring& val ) { m_Name = val; }
		SNode*					Ancestor( SPACE_NODE_TYPE val, bool bCheckThis = false)	const;
		int						ChildCount( SPACE_NODE_TYPE eType ) const;

		virtual TEE::NodeBase *	TEETmp() const { return m_pTEETmp; }
		virtual void			TEETmp(TEE::NodeBase* val)	{ m_pTEETmp = val; }

		//Validity
		virtual ErrorMask		CheckError();
		ErrorMask				GetErrorMask() const { return m_errorMask; }
		void					AddErrorFlag( ErrorMask val ) { m_errorMask |= val; }
		void					ClearErrorFlag( ErrorMask val ) { m_errorMask &= ~val; }

		//User Data
		void*					UserData( const string& key ) const;
		void					UserData( const string& key, void* pValue );
		void					IsArray(bool isArray) { UserData("bAry", isArray ? (void*)1 : nullptr); }
		bool					IsArray() const { return UserData("bAry") != nullptr; }

		//reference
		void AddReference( SNode* pNode )
		{
			if( pNode != NULL )
			{
				m_references.insert( pNode );
			}
		}
		void RemoveReference( SNode* pNode )
		{
			if( pNode != NULL )
			{
				m_references.erase( pNode );
			}
		}
		const set<SNode*>&		GetReferences(){ return m_references; }
		void					ClearReferences(){ m_references.clear(); }
		virtual void			Definition( SNode* pImpl );
		SNode*					Definition() const { return reinterpret_cast<SNode*>(UserData("definition"));	}
		bool					Working() const { return m_bWorking; }
		void					Working(bool val) { m_bWorking = val; }
		bool					IsStatementFunctionCall() const;
		bool					IsVoidFunctionCall();
		bool					IsVoidFunction();

	private:
		FORCEINLINE bool		DoFromXMLElement( const TiXmlElement* pXMLElement );

		TEE::NodeBase			*m_pTEETmp;

		SPACE_NODE_TYPE			m_classType;
		wstring					m_Name;
		ErrorMask				m_errorMask;
		map<string, void*>		m_UserData;
		set<SNode*>				m_references;
		bool					m_bWorking;

	};

	inline bool IsLocalVariable(SNode* pNode)
	{
		return pNode && pNode->IsA(SNT_VARIABLE) && pNode->Parent() && pNode->Parent()->IsA(SNT_ACTIONROOT);
	}
	inline bool IsGlobalVariable(SNode* pNode)
	{
		return pNode && pNode->IsA(SNT_VARIABLE) && pNode->Parent() && pNode->Parent()->IsA(SNT_VARSET);
	}
	inline bool IsParamVariable(SNode* pNode)
	{
		return pNode && pNode->IsA(SNT_VARIABLE) && pNode->Parent() && pNode->Parent()->IsA(SNT_PARAMROOT);
	}
}