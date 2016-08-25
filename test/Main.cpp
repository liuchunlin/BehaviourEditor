#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <Windows.h>
using namespace std;

#include "LuaSrv/Tick.h"


int main(int argc, char* argv[])
{
	CTick tick;
	tick.Init();
	while(true)
	{
		tick.Tick();
		Sleep( 10 );
	}
	tick.Exit();
	//int k = getchar();
	//while(k)
	//{
	//	switch(k)
	//	{
	//	case 49://1
	//		{
	//			RanaSpaceBank::getInstance()->reloadSpace(space->getHandle(), "F:/GW/Export/Script/battle.lua");
	//		}
	//		break;
	//	case 50:
	//		UseSkillEventMsg msg(0);
	//		space->onMessage( &msg );
	//		{
	//			UseSkillEventMsg msg(99);
	//			space->onMessage( &msg );
	//		}
	//		{
	//			UseItemEventMsg msg(78777);
	//			space->onMessage( &msg );
	//		}
	//		break;
	//	}
	//	k = getchar();
	//}
	return 0;
}








