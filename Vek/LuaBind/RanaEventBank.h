 #ifndef __RANA_EVENTBANK_H__
#define __RANA_EVENTBANK_H__

#pragma warning (disable : 4786)

#include <list>
#include <map>
#include <set>
#include <vector>
using namespace std;

#include "RanaEvent.h"
#include "RanaSortLable.h"

class RanaTriggerBank;
class RanaEventBank 
{
	friend class RanaSpace;
public:
	RanaEventBank(RanaSpace* pSpace);
	~RanaEventBank();
	RanaTriggerBank*	getTriggerBankInstance();

	void				delAllEvent();

	void				setEventOnOffByTrigger( RanaWORD _triggerHandle, bool _isOn );
	void				setEventOnOffByType( RanaEventType _eventType, bool _isOn );
	void				setEventOnOff( bool _isOn );

	// 打开/关闭一个指定的事件
	int					setEventOnOff( RanaWORD _eventHandle, bool _isOn);
	int					setEventOnOff( const RanaEventName& _eventName, bool _isOn);

/*
	// 删除一个指定的事件
	int					delEvent( RanaWORD _eventHandle );
	void				delEventByTrigger( RanaWORD _triggerHandle);
*/

	bool				getEventOnOff( RanaWORD _eventHandle);
	bool				listEventByTrigger( RanaWORD _triggerHandle, set<RanaEvent*>& events );
	void				listEvents( vector<RanaEvent*>& output );

	RanaEvent*			getEvent( RanaWORD _eventHandle);
	RanaEvent*			getEvent( const RanaEventName& _eventName);
private:
	int					registerEvent( RanaEvent *_event );			// 注册一个事件/触发器
	bool				onMessage( RanaMessage *_msg );				// 触发事件
	bool				onInvokeEvent( RanaEvent *_event, RanaMessage* _msg );
private:
	list<RanaWORD>		m_unusedHandle;
	vector<RanaEvent*>	m_eventsByHandle;

	RanaSpace*							m_pRanaSpace;
#if 0	//FORNAME
	map<RanaEventName,RanaEvent*>		m_nameMap;
#endif
	map<RanaWORD,set<RanaEvent*> >		m_triggerMap;

	typedef multimap<const RanaSortLable*,RanaEvent*,RanaSortLable::CmpSortLable> LABLE_MMAP;
	struct EventTypeMaintainer
	{
		vector<RanaEvent*> events;
		LABLE_MMAP		sortLables;
		map< RanaEvent*,LABLE_MMAP::iterator> lableIndex;
	};
	map< RanaEventType,EventTypeMaintainer > m_typeMap;
};

#endif //__RANA_EVENTBANK_H__
