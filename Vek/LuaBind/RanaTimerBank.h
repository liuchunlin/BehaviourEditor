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

	RanaTimerHandle			createTimer(const RanaSpaceHandle& _spaceHandle);	// ����һ���µļ�ʱ�������ظü�ʱ����Handle
	bool					delTimer(const RanaSpaceHandle& _spaceHandle);		// ɾ��һ��RanaSpace��Ӧ�����м�ʱ��
	void					routine(RanaTIME _expiretime);						// ��ʱ������time΢��

	void					setTimerPeriodTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, RanaTIME _periodTime);	// ���ü�ʱ����ʱ����
	RanaTIME				getTimerPeriodTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);							// ��ü�ʱ����ʱ����
	void					setTimerRepeat( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, bool _isRepeat);	// ���ü�ʱ�����ظ�ѡ��
	bool					getTimerRepeat( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);					// ��ü�ʱ�����ظ�ѡ��
    void					setTimerOnOff( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle, bool _isOn);		// ���ü�ʱ������״̬
	bool					getTimerOnOff( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);					// ��ü�ʱ������״̬
	RanaTIME				getTimerRestTime( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);// ��ü�ʱ��ʣ��ʱ�䣨��λ�����룩
	void					resetTimer( const RanaSpaceHandle& _spaceHandle, RanaTimerHandle _timerHandle);	// ���ü�ʱ��
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
