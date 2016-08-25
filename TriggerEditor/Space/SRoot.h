#pragma once

#include "SNode.h"

namespace Space
{
	class SRoot: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );

		//Menu
		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual	bool			Deleteable();
		virtual	bool			Disableable();
		virtual bool			Permitable();
	};
}