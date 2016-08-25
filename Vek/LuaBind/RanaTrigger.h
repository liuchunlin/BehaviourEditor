#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <string>
#include "RanaHeader.h"
#include "RanaTriggerStat.h"
#include "RanaEvent.h"

class RanaSpace;
class RanaTrigger
{
	friend class RanaTriggerBank;
public:
	RanaTrigger();
	virtual ~RanaTrigger();

	bool					checkCondtion();
	void					doAction(RanaMessage* msg);

	RanaSpace*				getSelfSpace();
	void					setName(const RanaTriggerName &_name);
	const RanaTriggerName&	getName();
	void					setOnOff(bool isOn);
	bool					getOnOff();
	RanaWORD				getHandle();
	RanaTriggerStat*		getTriggerStat();
private:
	RanaWORD				m_handle;
	RanaTriggerName			m_name;
	bool					m_isOn;
	RanaSpace*				m_pRanaSpace;

	RanaTriggerStat*		m_triggerStat;
};

inline RanaWORD RanaTrigger::getHandle()
{
	return m_handle;
}

inline RanaTriggerStat* RanaTrigger::getTriggerStat()
{
	return m_triggerStat;
}

#endif //__TRIGGER_H__
