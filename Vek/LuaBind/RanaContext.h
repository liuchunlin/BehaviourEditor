#ifndef __RANACONTEXT_H__
#define __RANACONTEXT_H__

#include "RanaSpace.h"

class RanaContext
{
	friend class RanaSpaceBank;
private:
	virtual RanaSpace::EXCEPTION_METHOD onRanaException(RanaSpace::EXCEPTION_CODE) = 0;
};

#endif
