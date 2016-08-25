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
		SNT_NULL,				//��
		SNT_ROOT,				//�ռ��
		SNT_VARSET,				//��������
			SNT_VARIABLE,		//�����б���
		SNT_GROUP,				//�ռ����
		//------------------------------
		SNT_TRIGGER,			//������
			SNT_EVENTROOT,			//�¼���
			SNT_EVENT,				//�¼�
			SNT_CONDITIONROOT,		//������
			SNT_CONDITION,			//����
			SNT_ACTIONROOT,			//������
		//---------------------------
		SNT_FUNCTION,			//����
			SNT_PARAMROOT,			//�βθ�
			SNT_RETURNTYPE,			//��������
			SNT_RETURN,				//�������
		//------------------------------
			SNT_MULTIPLE_IF,		//������֧
				SNT_IF,				//�����ж�
				SNT_THEN,			//�����Ϸ�����
				SNT_ELSE,			//�����Ƿ�����
			SNT_FOR_EACH,			//forѭ��
				SNT_LOOP,			//Loop
			SNT_SET_VARIABLE,		//���ñ���
			SNT_WHILE,				//whileѭ��
		//------------------------------
		SNT_PARAMETER,		//����ѡ��(Tmp)
		SNT_STATICLABEL,		//��̬�ı�
		SNT_ADJUST,				//����ֵ(Tmp)
		SNT_GLOBAL_VARREF,			//ȫ�ֱ���(Tmp)
		SNT_LOCAL_VARREF,			//�ֲ�����
		SNT_PARAM_VARREF,			//��������
		SNT_CONTEXT_VARREF,		//��������(Tmp)
		SNT_TFUNCCALL,			//TEE��������(Tmp)
		SNT_SFUNCCALL,			//Space��������
		SNT_BOOLEAN,			//����
		SNT_EDIT_VALUE,			//�༭

		//////////////////////////////////////////////////////////////////////////
		SNT_END,
	};
	enum EErrorFlag
	{
		VS_OK					= 0x00000000,		//��ȷ
		VS_PARAM_PENDING		= 0x00000001,		//δ����ʼ��
		VS_PARAM_NUMBER_ERR		= 0x00000002,		//������������
		VS_PARAM_TYPE_ERR		= 0x00000004,		//�������ʹ���
		VS_REFERENCE_ERR		= 0x00000008,		//���ö�ʧ�������Ͳ�ƥ��
		VS_EDIT_VALUE_ERR		= 0x00000010,		//�༭��ֵ����
		VS_TEETEMP_ERR			= 0x00000020,		//TEEģ�����
		VS_NAME_FOMAT_ERR		= 0x00000040,		//���ָ�ʽ����
		VS_NAME_DUP_ERR			= 0x00000080,		//�����ظ�����

		VS_EVERYTHING			= 0xFFFFFFFF,		//...
	};
	typedef	DWORD	ErrorMask;

	class ContextView
	{
	public:
		ContextView(){ bValid = false; bPreferences = false; bEnable = true; pNode = NULL; }
		wstring			text;			//�Ķ��ı�
		bool			bValid;			//�Ƿ���Ч
		bool			bPreferences;	//�Ƿ��ǿ�ѡ����
		bool			bEnable;		//�Ƿ����
		SNode			*pNode;			//��ӦINode
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