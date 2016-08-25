#ifndef __RANA_SORT_LABLE_H_
#define __RANA_SORT_LABLE_H_

#include "RanaHeader.h"

class RanaSortLable
{
public:
	virtual bool sortLableFront(const RanaSortLable& o) const
	{
		return false;
	};
	class CmpSortLable
	{
	public:
		bool operator()(const RanaSortLable* a,const RanaSortLable* b) const
		{
			return a->sortLableFront(*b);
		}
	};
};

#endif //__RANA_EVENT_H_
