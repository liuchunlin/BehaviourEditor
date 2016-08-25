#include "Tick.h"

#include <LuaBind/RanaHeader.h>
#include <LuaBind/RanaSpaceBank.h>
#include <LuaBind/RanaMessage.h>

#include "RanaSpaceType.h"
#include "RanaMsgImpl.h"

#include "../LuaCli/RanaStrategy.h"
#include "RanaService.h"
#include <Windows.h>

#include <Vek/Base/Timer.h>

extern std::string MakeValidPathName(const char* pPathName);

void	CTick::Init()
{
	char LuaGeneratePath[256];
	DWORD nBuffer = GetPrivateProfileStringA( ("LuaGeneratePath"), ("path"), (""), LuaGeneratePath, sizeof(LuaGeneratePath)/sizeof(char), ("Profile.ini") );
	string strLuaGeneratePath = MakeValidPathName(LuaGeneratePath);

	RanaStrategy *Strategy = new(std::nothrow) RanaStrategy;
	if( Strategy )
	{
		g_RanaService->setStrategy(Strategy);
		Strategy->setBaseBaseScriptPath( LuaGeneratePath );
		const char *ip="127.0.0.1";
		WORD port=9000;
		g_RanaService->setMaxSessions(5);
		g_RanaService->setListenAddress(ip,port);
	}
	RanaSpaceBank::getInstance()->initMaxSpaces(100);

	m_BattleSpace = RanaSpaceBank::getInstance()->createSpace("Battle", string(string(LuaGeneratePath)+"Battle.lua").c_str(), RANA_SPACETYPE_Battle, NULL);
	RanaSpaceBank::getInstance()->createSpace("Repetition", string(string(LuaGeneratePath)+"Repetition.lua").c_str(), RANA_SPACETYPE_Repetition, NULL);
	RanaSpaceBank::getInstance()->createSpace("Scene", string(string(LuaGeneratePath)+"Scene.lua").c_str(), RANA_SPACETYPE_Scene, NULL);
	if( m_BattleSpace != NULL )
	{
		//m_BattleSpace->onMessage( &UseItemEventMsg(1) );
		//m_BattleSpace->onMessage( &UseItemEventMsg(2) );
	//	{
	//		UseSkillEventMsg msg(99);
	//		space->onMessage( &msg );
	//	}
	//	{
	//		UseItemEventMsg msg(78777);
	//		space->onMessage( &msg );
	//	}
	}



	GVekService->Init();

}
void	CTick::Tick()
{
	GVekService->Tick( GetTickCount() );

	short state = GetKeyState( VK_F1 );
	if( state & 0x80 )
	{
		m_BattleSpace->onMessage( &TestKeyEventMsg(1) );
	}
	state = GetKeyState( VK_F2 );
	if( state & 0x80 )
	{
		m_BattleSpace->onMessage( &TestKeyEventMsg(2) );
	}
}
void	CTick::Exit()
{
	GVekService->Flush();
} 