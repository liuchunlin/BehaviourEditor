#ifndef __RANA_TIMER_BANK_H__
#define __RANA_TIMER_BANK_H__

#pragma warning (disable : 4786)

#include <map>
#include <set>
#include <list>
using namespace std;
#include "RanaTimer.h"

class RanaTimerBank
{
public:
	class CompRanaTimerPoint
	{
	public:
		const bool operator()(RanaTimer* a, RanaTimer* b) const
		{
			return a->getExpireTime() < b->getExpireTime();
		}
	};
	struct ListIterator
	{
		enum
		{
			E_ACTIVE,
			E_PAUSED
		} itType;
		multiset<RanaTimer*,CompRanaTimerPoint>::iterator activeIt;
		list<RanaTimer*>::iterator pausedIt;
	};
	typedef map<RanaTimerHandle, ListIterator> RANA_HANDLEITMAP;
	struct RanaTimerHandleManager
	{
		RanaTimerHandleManager():nextHandle(1)
		{}
		ListIterator& operator [](RanaTimerHandle handle)
		{
			return handleMap[handle];
		}
		int					nextHandle;
		RANA_HANDLEITMAP	handleMap;
	};
	typedef map<RanaWORD, RanaTimerHandleManager> RANA_SPACEITMAP;

	static RanaTimerBank*	getInstance();
	static void				release();

	RanaTimerHandle			createTimer(const RanaSpaceHandle& _spaceHandle);	// 构造一个新的计时器，返回该计时器的Handle
	bool					delTimer(const RanaSpaceHandle& _spaceHandle);		// 删除一个RanaSpace对应的所有计时器
	void					routine(RanaTIME _expiretime);						// 计时器运行time微秒

	void					setTimerPeriodTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, RanaTIME _periodTime);	// 设置计时器计时周期
	RanaTIME				getTimerPeriodTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);							// 获得计时器计时周期
	void					setTimerRepeat( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, bool _isRepeat);	// 设置计时器可重复选项
	bool					getTimerRepeat( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);					// 获得计时器可重复选项
    void					setTimerOnOff( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, bool _isOn);		// 设置计时器开关状态
	bool					getTimerOnOff( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);					// 获得计时器开关状态
	RanaTIME				getTimerRestTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);// 获得计时器剩余时间（单位：毫秒）
	void					resetTimer( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);	// 重置计时器
	void					initCurrentTime(RanaTIME _time);
	RanaTimer*				ExpiringTimer(){ return m_ExpiringTimer; }
protected:
	RanaTimer*				getTimer( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);
private:
	RanaTimerBank();
	~RanaTimerBank();
	static RanaTimerBank *INSTANCE;
private:
	multiset<RanaTimer*,CompRanaTimerPoint> m_activeTimer;
	list<RanaTimer*>						m_pausedTimer;
	RANA_SPACEITMAP							m_timerMap;
	RanaTIME								m_curTime;
	RanaTimer*								m_ExpiringTimer;
};

#endif //__RANA_TIMER_BANK_H__
