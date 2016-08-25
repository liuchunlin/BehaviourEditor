#pragma once

#include "NodeBase.h"

namespace TEE
{
	class Event;
	class Condition;
	class Function;
	class Option;
	class NodeFactory
	{
	public:
		static NodeBase*	CreateNode( NODE_TYPE eType );
		static NODE_TYPE	NodeNameToType( const wstring& nodeName );
		static const wstring& NodeTypeToName( NODE_TYPE eType );
		static NodeBase*	BuildEvent( const wstring& displayName, const wstring& codeName, const wstring& tags, NodeBase* pNode = NULL );
		static NodeBase*	BuildCondition( const wstring& displayName, const wstring& codeName, const wstring& tags, NodeBase* pNode = NULL );
		static NodeBase*	BuildFunction( const wstring& displayName, const wstring& codeName, const wstring& returnCode, int eAry, const wstring& tags, bool canBeAction, NodeBase* pNode = NULL );
		static NodeBase*	BuildOption( const wstring& displayName, const wstring& codeName, int eValueType, const wstring& tags, NodeBase* pNode = NULL );
		static NodeBase*	BuildSpace( const wstring& displayName, const wstring& codeName, NodeBase* pNode = NULL );
		static NodeBase*	BuildRoot( const wstring& displayName, const wstring& codeName, NodeBase* pNode = NULL );

		static NodeBase*	BuildNormalChild( const wstring& content, NodeBase* pParent );
		static NodeBase*	BuildTagChild( const wstring& content, NodeBase* pParent );
		static NodeBase*	BuildIncludesChild( const set<wstring>& contents, NodeBase* pParent );
		static NodeBase*	BuildOptionChild( const wstring& content, NodeBase* pParent );
	private:
	};
	static NodeFactory	GNodeFactory;		//目前有没有这个对象，无所谓
} 