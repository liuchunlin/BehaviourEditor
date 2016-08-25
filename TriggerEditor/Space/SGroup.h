#pragma once

#include "SNode.h"

namespace Space
{
	class SGroup: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SGroup();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );

		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_ROOT; }
		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual	bool			Deleteable();
		virtual bool			UniqueName() const;
	};
}