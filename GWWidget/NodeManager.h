#pragma once

#include "Vek/Base/Singleton.h"
#include "NodeBase.h"

class NodeManager: public Vek::Singleton<NodeManager>
{
	friend class Vek::Singleton<NodeManager>;
public:
	TEE::NodeBase*		OnNodeProperty( TEE::NodeBase* pParent, TEE::NODE_TYPE eType, TEE::NodeBase* pNode );
private:
	NodeManager();
};

extern NodeManager*	GNodeMgr ;