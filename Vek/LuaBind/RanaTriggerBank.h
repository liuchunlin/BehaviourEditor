 /* Generated by Together */

#ifndef __TRIGGERBANK_H__
#define __TRIGGERBANK_H__

#pragma warning (disable : 4786)

#include <map>
#include <list>
using namespace std;
#include "RanaTrigger.h"

class RanaTriggerBank
{
	friend class RanaSpace;
public:
	
	int				addTrigger(RanaTrigger* trigger);		// 添加一个触发器
	//int				deleteTrigger(RanaWORD triggerHandle);	// 删除一个触发器
	void			deleteTrigger();						// 删除所有触发器
	int				burstTrigger(RanaWORD triggerHandle, RanaMessage* msg);	// 触发一个触发器
	int				burstTrigger(RanaWORD triggerHandle, RanaMessage* msg, bool bCheckCondition);	// 触发一个触发器（是否进行条件判断）
	int				setTriggerOnOff(RanaWORD triggerHandle, bool isOn);	// 打开/关闭一个触发器
	bool			getTriggerOnOff(RanaWORD triggerHandle);// 获得触发器开关状态
	
	bool			tagMessageCount(RanaWORD triggerHandle);// 触发器状态统计：收到消息
	void			setAllTriggerOnOff(bool isOn);
	void			listTriggers(std::vector<RanaTrigger*>& output);

	RanaTrigger*	getTrigger(RanaWORD triggerHandle);
	RanaTrigger*	getTrigger(const RanaTriggerName& triggerName);
	RanaTrigger*	BurstingTrigger(){ return m_BurstingTrigger; }

	RanaTriggerBank(RanaSpace* pSpace);
	~RanaTriggerBank();
private:
	list<RanaWORD>						m_unusedHandle;
	vector<RanaTrigger*>				m_triggersByHandle;
	map<RanaTriggerName,RanaTrigger*>	m_nameMap;
	RanaTrigger*						m_BurstingTrigger;

	RanaSpace* m_pRanaSpace;
};

#endif //__TRIGGERBANK_H__
