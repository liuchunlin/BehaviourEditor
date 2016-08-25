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

	// ��/�ر�һ��ָ�����¼�
	int					setEventOnOff( RanaWORD _eventHandle, bool _isOn);
	int					setEventOnOff( const RanaEventName& _eventName, bool _isOn);

/*
	// ɾ��һ��ָ�����¼�
	int					delEvent( RanaWORD _eventHandle );
	void				delEventByTrigger( RanaWORD _triggerHandle);
*/

	bool				getEventOnOff( RanaWORD _eventHandle);
	bool				listEventByTrigger( RanaWORD _triggerHandle, set<RanaEvent*>& events );
	void				listEvents( vector<RanaEvent*>& output );

	RanaEvent*			getEvent( RanaWORD _eventHandle);
	RanaEvent*			getEvent( const RanaEventName& _eventName);
private:
	int					registerEvent( RanaEvent *_event );			// ע��һ���¼�/������
	bool				onMessage( RanaMessage *_msg );				// �����¼�
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
