
#include "RanaEventBank.h"
#include "RanaTimeMessage.h"
#include "RanaTimeEvent.h"
#include "RanaTriggerBank.h"
#include "RanaSpaceBank.h"
///
//#include "RanaContext.h"
///

RanaEventBank::RanaEventBank(RanaSpace* pSpace)
:m_pRanaSpace(pSpace)
{
}

RanaEventBank::~RanaEventBank()
{
	delAllEvent();
}

// 注册一个事件/触发器
int RanaEventBank::registerEvent(RanaEvent *_event)
{
	//检查有效性
	if (!_event)
	{
		return -1;
	}
#if 0	//FORNAME
	const RanaEventName& eventName = _event->getName();
	if(!strcmp(eventName,""))
	{
		return -1;
	}
	if( m_nameMap.find(eventName) != m_nameMap.end() )
	{
		return -1;
	}
#endif
	vector<RanaEvent*>::iterator iterEvent(m_eventsByHandle.begin());
	for( ; iterEvent != m_eventsByHandle.end(); ++iterEvent )
	{
		RanaEvent* pEvent = (*iterEvent);
		if( pEvent->operator ==( *_event ) )
		{
			return -1;
		}
	}

	RanaWORD handle = 0;
	if( m_unusedHandle.empty() )
	{
		const RanaWORD maxHandle = ~0;
		if( m_eventsByHandle.size() >= maxHandle )
		{
			return -1;
		}
		handle = static_cast<RanaWORD>( m_eventsByHandle.size() );
		m_eventsByHandle.push_back(NULL);
	}
	else
	{
		handle = m_unusedHandle.front();
		m_unusedHandle.pop_front();
	}
	_event->m_handle = handle;
	_event->m_pRanaSpace = m_pRanaSpace;
	RANA_ASSERT( getEvent(handle) == NULL );
	m_eventsByHandle[handle] = _event;
#if 0	//FORNAME
	m_nameMap[eventName] = _event;
#endif
	RanaEventType eventType = _event->getType();
	EventTypeMaintainer& maintainer = m_typeMap[eventType];
	maintainer.events.push_back(_event);
	if(_event->getSortLable())
	{
		if( maintainer.lableIndex.find(_event) != maintainer.lableIndex.end() )
		{
			RANA_ASSERT(0);
		}
		else
		{
			LABLE_MMAP::iterator itLable = maintainer.sortLables.insert( make_pair(_event->getSortLable(),_event) );
			maintainer.lableIndex[_event] = itLable;
		}
	}
	RanaWORD triggerHandle = _event->getTriggerHandle();
	if( !m_triggerMap[triggerHandle].insert(_event).second )
	{
		RANA_ASSERT(0);
	}

	return 0;
}

/*
int RanaEventBank::delEvent( RanaWORD _eventHandle )
{
	RanaEvent* pEvent = getEvent(_eventHandle);
	if( !pEvent )
	{
		return -1;
	}
	m_eventsByHandle[_eventHandle] = NULL;
#if 0	//FORNAME
	m_nameMap.erase(pEvent->getName());
#endif 
	m_unusedHandle.push_back(_eventHandle);
	map< RanaEventType,EventTypeMaintainer >::iterator itType = m_typeMap.find(pEvent->getType());
	if(itType!=m_typeMap.end())
	{
		EventTypeMaintainer& maitainer = itType->second;
		set<RanaEvent*>::iterator it = maitainer.events.find(pEvent);
		if(it!=maitainer.events.end())
		{
			maitainer.events.erase(it);
		}
		else
		{
			RANA_ASSERT(0);
		}
		if( maitainer.lableIndex.find(pEvent) != maitainer.lableIndex.end() )
		{
			maitainer.sortLables.erase(maitainer.lableIndex[pEvent]);
			maitainer.lableIndex.erase(pEvent);
		}
		if( maitainer.events.empty() )
		{
			RANA_ASSERT(maitainer.lableIndex.empty() && maitainer.sortLables.empty() );
			m_typeMap.erase(itType);
		}
	}
	else
	{
		RANA_ASSERT(0);
	}
	map< RanaWORD,set<RanaEvent*> >::iterator itTrigger = m_triggerMap.find(pEvent->getTriggerHandle());
	if( itTrigger!=m_triggerMap.end() )
	{
		if( !itTrigger->second.erase(pEvent) )
		{
			RANA_ASSERT(0);
		}
	}
	else
	{
		RANA_ASSERT(0);
	}
	delete pEvent;

	return 0;
}*/


// 删除一个RanaSpace对应的所有事件
void RanaEventBank::delAllEvent()
{
#ifdef _DEBUG
	int eventDiff = 0;	//this is for test
#endif
	for(size_t i = 0; i< m_eventsByHandle.size(); ++i)
	{
		RanaEvent* pEvent = m_eventsByHandle[i];
		if(!pEvent)
		{
			continue;
		}
		delete pEvent;
		m_eventsByHandle[i] = NULL;
#ifdef _DEBUG
		++eventDiff;
#endif
	}
#ifdef _DEBUG
	map<RanaEventType, EventTypeMaintainer>::iterator mapIt = m_typeMap.begin();
	for( ;mapIt != m_typeMap.end();++mapIt )
	{
		EventTypeMaintainer& maintainer = mapIt->second;
		vector<RanaEvent*>::iterator it = maintainer.events.begin();
		int lableCount = static_cast<int>( maintainer.events.size() );
		eventDiff -= lableCount;
		if( maintainer.sortLables.size() != 0 )
		{
			RANA_ASSERT(maintainer.lableIndex.size() == lableCount);
		}
	}
	RANA_ASSERT(eventDiff == 0);
#endif
	m_eventsByHandle.clear();
#if 0	//FORNAME
	m_nameMap.clear();
#endif
	m_typeMap.clear();
	m_triggerMap.clear();
	m_unusedHandle.clear();
}

/*
void RanaEventBank::delEventByTrigger( RanaWORD _triggerHandle )
{
	map< RanaWORD, set<RanaEvent*> >::iterator mapTriggerIt = m_triggerMap.find(_triggerHandle);
	if(mapTriggerIt != m_triggerMap.end())
	{
		//枚举map中的元素
		set<RanaEvent*>::iterator itSet = mapTriggerIt->second.begin();
		for( ;itSet != mapTriggerIt->second.end(); ++itSet )
		{
			RanaEvent* pEvent = (*itSet);
			RanaWORD _eventHandle = pEvent->getHandle();
			if( getEvent(_eventHandle) != pEvent )
			{
				RANA_ASSERT(0);
			}
			else
			{

				m_eventsByHandle[_eventHandle] = NULL;
#if 0	//FORNAME
				m_nameMap.erase(pEvent->getName());
#endif
				m_unusedHandle.push_back(_eventHandle);
			}
			map< RanaEventType,EventTypeMaintainer >::iterator itType = m_typeMap.find(pEvent->getType());
			if(itType!=m_typeMap.end())
			{
				EventTypeMaintainer& maintainer = itType->second;
				set<RanaEvent*>::iterator it = maintainer.events.find(pEvent);
				if(it!=maintainer.events.end())
				{
					maintainer.events.erase(it);
				}
				if( maintainer.lableIndex.find(pEvent) != maintainer.lableIndex.end() )
				{
					maintainer.sortLables.erase(maintainer.lableIndex[pEvent]);
					maintainer.lableIndex.erase(pEvent);
				}
				if( maintainer.events.empty() )
				{
					RANA_ASSERT(maintainer.sortLables.empty() && maintainer.lableIndex.empty() );
					m_typeMap.erase(itType);
				}
			}
			else
			{
				RANA_ASSERT(0);
			}
			delete pEvent;
		}
		mapTriggerIt->second.clear();
		m_triggerMap.erase(mapTriggerIt);
	}
}
*/

// 触发事件
bool RanaEventBank::onMessage(RanaMessage* _msg)
{
	if( _msg == NULL )
		return false;

	bool bSuccess = false;
	RanaEventType eventType = _msg->getEventType();
	map< RanaEventType,EventTypeMaintainer >::iterator mapIt = m_typeMap.find(eventType);
	if(mapIt == m_typeMap.end())
	{
		return false;
	}
	EventTypeMaintainer& maintainer = mapIt->second;
	if( _msg->getSortLable() )
	{
		if(maintainer.sortLables.empty())
		{
			return false;
		}
		LABLE_MMAP::iterator itBegin = maintainer.sortLables.lower_bound(_msg->getSortLable());
		LABLE_MMAP::iterator itEnd = maintainer.sortLables.upper_bound(_msg->getSortLable());
		for(;itBegin != itEnd;++itBegin)
		{
			RanaEvent *event = itBegin->second;
			bSuccess |= onInvokeEvent( event, _msg );
			if (bSuccess && _msg->GetTriggerFirstOnly())
			{
				break;
			}
		}
	}
	else
	{
		vector<RanaEvent*>::iterator it = maintainer.events.begin();
		for(;it != maintainer.events.end();++it)
		{
			RanaEvent *event = *it;
			bSuccess |= onInvokeEvent( event, _msg );
			if (bSuccess && _msg->GetTriggerFirstOnly())
			{
				break;
			}
		}
	}
	return bSuccess;
}
bool	RanaEventBank::onInvokeEvent( RanaEvent *_event, RanaMessage* _msg )
{
	if( _event == NULL )
		return false;
	getTriggerBankInstance()->tagMessageCount( _event->getTriggerHandle() );
	if ( !_event->getOnOff() )
		return false;
	if( !_event->preValidate(_msg) )
		return false;
	return 0 == getTriggerBankInstance()->burstTrigger(	_event->getTriggerHandle(), _msg );
}

void RanaEventBank::setEventOnOffByTrigger( RanaWORD _triggerHandle, bool _isOn )
{
	map< RanaWORD,set<RanaEvent*> >::iterator mapTriggerIt = m_triggerMap.find(_triggerHandle);
	if(mapTriggerIt != m_triggerMap.end())
	{
		//枚举map中的元素
		set<RanaEvent*>::iterator it = mapTriggerIt->second.begin();
		while(it != mapTriggerIt->second.end())
		{
			(*it)->setOnOff(_isOn);
			it++;
		}
	}
}

void RanaEventBank::setEventOnOffByType( RanaEventType _eventType, bool _isOn)
{
	map< RanaEventType,EventTypeMaintainer>::iterator mapIt = m_typeMap.find(_eventType);
	if(mapIt != m_typeMap.end())
	{
		//枚举map中的元素
		EventTypeMaintainer& maintainer = mapIt->second;
		vector<RanaEvent*>::iterator it = maintainer.events.begin();
		for( ;it != maintainer.events.end() ; ++it)
		{
			(*it)->setOnOff(_isOn);
		}
	}
}

void RanaEventBank::setEventOnOff( bool _isOn )
{
	for(size_t i = 0; i< m_eventsByHandle.size(); ++i)
	{
		RanaEvent* pEvent = m_eventsByHandle[i];
		if(!pEvent)
		{
			continue;
		}
		pEvent->setOnOff(_isOn);
	}
}

RanaEvent* RanaEventBank::getEvent( RanaWORD _eventHandle)
{
	if( _eventHandle >= m_eventsByHandle.size() )
	{
		return NULL;
	}
	return m_eventsByHandle[_eventHandle];
}

RanaEvent* RanaEventBank::getEvent( const RanaEventName& _eventName)
{
#if 0	//FORNAME
	map<RanaEventName,RanaEvent*>::iterator itEvent = m_nameMap.find(_eventName);
	if( itEvent != m_nameMap.end() )
	{
		return itEvent->second;
	}
#endif
	return NULL;
}

int RanaEventBank::setEventOnOff( RanaWORD _eventHandle, bool _isOn)
{
	RanaEvent* pRanaEvent = getEvent(_eventHandle);
	if(!pRanaEvent)
	{
		return -1;
	}
	pRanaEvent->setOnOff(_isOn);
	return 0;
}

int RanaEventBank::setEventOnOff( const RanaEventName& _eventName, bool _isOn)
{
	RanaEvent* pRanaEvent = getEvent(_eventName);
	if(!pRanaEvent)
	{
		return -1;
	}
	pRanaEvent->setOnOff(_isOn);
	return 0;
}

bool	RanaEventBank::listEventByTrigger( RanaWORD _triggerHandle, set<RanaEvent*>& events )
{
	map< RanaWORD,set<RanaEvent*> >::iterator mapTriggerIt = m_triggerMap.find(_triggerHandle);
	if(mapTriggerIt != m_triggerMap.end())
	{
		events = (*mapTriggerIt).second;
		return true;
	}
	return false;
}

void RanaEventBank::listEvents( vector<RanaEvent*>& output )
{
	for(size_t i = 0; i< m_eventsByHandle.size(); ++i)
	{
		RanaEvent* pEvent = m_eventsByHandle[i];
		if(!pEvent)
		{
			continue;
		}
		output.push_back(pEvent);
	}
}

RanaTriggerBank* RanaEventBank::getTriggerBankInstance()
{
	return m_pRanaSpace->getTriggerBank();
}
