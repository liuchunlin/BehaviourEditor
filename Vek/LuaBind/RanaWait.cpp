#include "RanaWait.h"
#include "RanaSpace.h"
#include "RanaSpaceBank.h"


void cb_LuaThreadAwake( const vector<void*>& uds, Vek::Timer* pSelf )
{
	if( uds.empty() )
	{
		RANA_ASSERT( false );
		return ;
	}
	lua_State *T = reinterpret_cast<lua_State*>(*uds.begin());
	if( lua_status( T ) == LUA_YIELD )
	{
		lua_resume( T, 0 );
	}

	RanaSpace *pSpace = GetRanaSpace(T);
	if( pSpace != NULL )
	{
		GLuaWaitGridge->Remove( pSpace->getSpaceType(), pSelf );
		pSpace->ThreadBreak( T, true );
	}
}
//////////////////////////////////////////////////////////////////////////
LuaWaitBridge *GLuaWaitGridge = LuaWaitBridge::GetInstance();

void LuaWaitBridge::Tick(FLOAT DeltaTime)
{
	map<int, set<FLuaWaiter*> >::iterator itrSpaceWaiters = m_ThreadWaiters.begin();
	for ( ; itrSpaceWaiters != m_ThreadWaiters.end(); ++itrSpaceWaiters)
	{
		set<FLuaWaiter*>& SpaceWaiters = itrSpaceWaiters->second;
		set<FLuaWaiter*>::iterator itrWaiter = SpaceWaiters.begin();
		while (itrWaiter != SpaceWaiters.end())
		{
			FLuaWaiter* pWaiter = (*itrWaiter);
			bool bContinueWait = pWaiter->Tick(DeltaTime);
			if (!bContinueWait)
			{
				lua_State* T = pWaiter->m_LuaThread;
				if (T)
				{
					if( lua_status( T ) == LUA_YIELD )
					{
						lua_resume( T, 0 );
					}
					RanaSpace *pSpace = GetRanaSpace(T);
					if( pSpace != NULL )
					{
						pSpace->ThreadBreak( T, true );
					}
				}
				itrWaiter = SpaceWaiters.erase(itrWaiter);
				SAFE_DELETE(pWaiter);
			}
			else
			{
				++itrWaiter;
			}
		}
	}
}

int	LuaWaitBridge::Add( lua_State* T, int time )
{
	RanaSpace* pSpace = GetRanaSpace( T );
	if( pSpace != NULL )
	{
		int eType = pSpace->getSpaceType();
		if( pSpace->Status() == RanaSpace::SS_RUNING )
		{
			vector<void*> uds;
			uds.push_back( reinterpret_cast<void*>(T) );
			Vek::Timer *pTimer = new Vek::Timer( 0, 0, NULL, cb_LuaThreadAwake, time/1000.f, uds );
			if( pTimer == NULL )
				return 0;

			m_ThreadTimers[eType].insert( pTimer );
		}
		else
		{
			return 0;
		}
	}
	return lua_yield( T, 0 );
}

void	LuaWaitBridge::Add( int eType, Vek::Timer* pEntry )
{
	assert( false );
	if( pEntry == NULL )
		return;

	m_ThreadTimers[eType].insert( pEntry );
}
void	LuaWaitBridge::Remove( int eType, Vek::Timer* pEntry )
{
	if( m_ThreadTimers.find( eType ) == m_ThreadTimers.end() )
		return;
	m_ThreadTimers[eType].erase( pEntry );
}

int LuaWaitBridge::Add( lua_State* T, FLuaWaiter* pWait )
{
	RanaSpace* pSpace = GetRanaSpace( T );
	if( pSpace && pSpace->Status() == RanaSpace::SS_RUNING )
	{
		if( pWait )
		{
			pWait->m_LuaThread = T;
			int eType = pSpace->getSpaceType();
			m_ThreadWaiters[eType].insert( pWait );
			return lua_yield( T, 0 );
		}
	}
	return 0;
}

void	LuaWaitBridge::RemoveTimers( int eType )
{
	if( m_ThreadTimers.find( eType ) == m_ThreadTimers.end() )
		return;
	RanaSpace* pSpace = RanaSpaceBank::getInstance()->GetLuaSpace( eType );
	if( NULL == pSpace )
		return;

	set<Vek::Timer*>::iterator iter( m_ThreadTimers[eType].begin() );
	for ( ; iter != m_ThreadTimers[eType].end(); ++iter )
	{
		Vek::Timer* pTimer = (*iter);
		bool bRemoved = Vek::TimerBank::GetInstance()->Remove( pTimer );
		assert( bRemoved );
		if( !pTimer->UserDatas.empty() )
		{
			lua_State *T = reinterpret_cast<lua_State*>(pTimer->UserDatas[0]);
			pSpace->ThreadBreak( T, false );
		}
		SAFE_DELETE(pTimer);
	}

	m_ThreadTimers.erase( eType );
}

void	LuaWaitBridge::RemoveWaiters( int eType )
{
	if( m_ThreadWaiters.find( eType ) == m_ThreadWaiters.end() )
		return;
	RanaSpace* pSpace = RanaSpaceBank::getInstance()->GetLuaSpace( eType );
	if( NULL == pSpace )
		return;

	set<FLuaWaiter*>::iterator iter( m_ThreadWaiters[eType].begin() );
	for ( ; iter != m_ThreadWaiters[eType].end(); ++iter )
	{
		FLuaWaiter* pWaiter = (*iter);
		if( pWaiter->m_LuaThread )
		{
			pSpace->ThreadBreak( pWaiter->m_LuaThread, false );
		}
		SAFE_DELETE(pWaiter);
	}

	m_ThreadWaiters.erase( eType );
}

void	LuaWaitBridge::DeleteSpace( int eType )
{
	RemoveTimers( eType );
	RemoveWaiters( eType );
}

void	LuaWaitBridge::OnWorldExit()
{
	while (!m_ThreadTimers.empty())
	{
		RemoveTimers( m_ThreadTimers.begin()->first );
	}

	while (!m_ThreadWaiters.empty())
	{
		RemoveWaiters( m_ThreadWaiters.begin()->first );
	}
}
