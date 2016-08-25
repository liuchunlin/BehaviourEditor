#pragma once

#include "SNode.h"

namespace Space
{
	class SStaticLabel: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SStaticLabel();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
	};
}