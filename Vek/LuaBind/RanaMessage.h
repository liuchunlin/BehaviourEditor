#ifndef __RANA_MESSAGE_H__
#define __RANA_MESSAGE_H__

#include "RanaHeader.h"
#include "RanaSortLable.h"
#include <vector>

/*
Desc: 消息类，用于触发相应类型的事件
author: YYBear
*/
class RanaMessage
{
public:
	RanaMessage();
	virtual ~RanaMessage();

	void			setEventType(RanaEventType _eventType);
	RanaEventType	getEventType();

	virtual const RanaSortLable* getSortLable() const
	{
		return NULL;
	}

	const std::vector<RanaVariant>& GetContexts()
	{
		return m_contexts;
	}

	bool GetTriggerFirstOnly() { return m_bTriggerFirstOnly; }
	void SetTriggerFirstOnly(bool bFirstOnly) { m_bTriggerFirstOnly = bFirstOnly; }

protected:
	std::vector<RanaVariant>	m_contexts;
private:
	RanaEventType	m_eventType;	// 触发事件的事件类型
	bool			m_bTriggerFirstOnly;
};

#endif //__RANA_MESSAGE_H__
