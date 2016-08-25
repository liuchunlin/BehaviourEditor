#pragma once

#include "RanaHeader.h"

#include <Vek/Base/Singleton.h>
#include <Vek/Base/TimerBank.h>

class FLuaWaiter
{
public:
	FLuaWaiter() : m_LuaThread(NULL) {}
	virtual ~FLuaWaiter(){}

	// End wait when return false.
	virtual bool Tick(float DeltaTime) = 0;

public:
	lua_State* m_LuaThread;
};

class LuaWaitBridge: public Vek::Singleton<LuaWaitBridge>
{
	friend class Vek::Singleton<LuaWaitBridge>;
public:
	int					Add( lua_State* T, int time );
	void				Add( int eType, Vek::Timer* pEntry );
	void				Remove( int eType, Vek::Timer* pEntry );
	int					Add( lua_State* T, FLuaWaiter* pWait );

	void				RemoveTimers( int eType );
	void				RemoveWaiters( int eType );
	void				DeleteSpace( int eType );
	void				OnWorldExit();

	void				Tick(FLOAT DeltaTime);
private:
	map<int, set<Vek::Timer*> >		m_ThreadTimers;		//<Space, LuaWaitTimers>
	map<int, set<FLuaWaiter*> >		m_ThreadWaiters;		//<Space, LuaWait>
};

extern LuaWaitBridge*	GLuaWaitGridge;
extern void cb_LuaThreadAwake( const vector<void*>& uds, Vek::Timer* pSelf );
