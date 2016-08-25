#include <RanaSpaceType.h>
#include <LuaBind/RanaSpace.h>
#include <LuaBind/RanaHeader.h>
#include <LuaBind/RanaPush.h>
#include <LuaBind/RanaInputStack.h>

#include <LuaBind/RanaTimerBank.h>
#include "RanaType.h"

#include <vek/Base/Timer.h>
#include <LuaBind/RanaWait.h>

//移动单位到指定点
void r_MoveUnitToPosition(RanaState L, const RNT_UNIT::TYPE& unit, const RNT_POSITION::TYPE& pos)
{
	printf( "r_MoveUnitToPosition( %d, %s )\n", unit, pos );
}
//使单位面朝方向
void r_UnitFaceTo(RanaState L, const RNT_UNIT::TYPE& unit, const RNT_DIRECTION::TYPE& dstDir, const RNT_INT::TYPE& limitTime)
{
	printf( "r_UnitFaceTo( %d, %d, %d )\n", unit, dstDir, limitTime );
}

//等待
extern void r_Wait(RanaState L, const RNT_INT::TYPE& time);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, Wait);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, Wait);
int l_Wait(RanaState L)
{
	RNT_INT::TYPE time;
	RANA_INPUT_BEGIN
	s>>time;
	RANA_INPUT_END(Wait)

	//printf( "r_Wait( 等待: %d 毫秒)\n", time );
	//vector<void*> uds;
	//uds.push_back( reinterpret_cast<void*>(L) );
	//Vek::Timer *pTimerEntry = new Vek::Timer( 0, 0, NULL, cb_LuaThreadAwake, time, uds );
	//RanaSpace* pSpace = GetRanaSpace( L );
	//if( pSpace != NULL )
	//{
	//	GLuaWait->Add( pSpace->getSpaceType(), pTimerEntry );
	//}
	//return lua_yield( L, 0 );
	return GLuaWaitGridge->Add( L, time );
}

//TEST
void r_TEST(RanaState L, const RNT_INT::TYPE& k)
{
	printf( "r_TEST( %d==== )\n", k );
}
//无动作
void r_DoNothing(RanaState L)
{
	printf( "r_DoNothing()\n" );
}

//等待条件
void r_WaitForCondition(RanaState L, const RNT_CONDITION::TYPE& condition, const RNT_INT::TYPE& checkTick)
{

}
//略过剩余动作
void r_SkipRemainingActions(RanaState L)
{

}
//运行触发器
void r_RunTrigger(RanaState L, const RNT_TRIGGER::TYPE& tgr, const RNT_ISCHECK::TYPE& bCheckCondition)
{
	RanaSpace* pSpace = GetRanaSpace(L);
	if( pSpace == NULL )
		return;
	RanaTrigger* pTirgger = pSpace->getTriggerBank()->getTrigger( tgr );
	if( pTirgger == NULL )
	{
		pTirgger = new RanaTrigger;
		pTirgger->setName(tgr);
		pTirgger->setOnOff(true);
		pSpace->getTriggerBank()->addTrigger(pTirgger);
	}
	pSpace->getTriggerBank()->burstTrigger( pTirgger->getHandle(), NULL, bCheckCondition );
}
//当前触发器
RNT_TRIGGER::TYPE r_ThisTrigger(RanaState L)
{
	RanaSpace* pSpace = GetRanaSpace(L);
	if( pSpace == NULL )
		return NULL;
	RanaTrigger* pTirgger = pSpace->getTriggerBank()->BurstingTrigger();
	if( pTirgger == NULL )
	{
		return pTirgger->getName();
	}
	return NULL;
}
//激活计时器
void r_ActiveTimer(RanaState L, const RNT_ACTIVE::TYPE& active, const RNT_TIMER::TYPE& timer)
{
	RanaSpace* pSpace = GetRanaSpace(L);
	if( pSpace == NULL )
		return;
	RanaTimerBank::getInstance()->setTimerOnOff(pSpace->getHandle(), timer, active!=0);
}
//运行计时器
void r_StartTimer(RanaState L, const RNT_TIMER::TYPE& timer, const RNT_TIMER_PERIOD::TYPE& rp, const RNT_REAL::TYPE& expireTime)
{

}
//刚过期的计时器
RNT_TIMER::TYPE r_ExpiringTimer(RanaState L)
{
	RanaSpace* pSpace = GetRanaSpace(L);
	if( pSpace == NULL )
		return 0;

	RanaTimer*	pTimer = RanaTimerBank::getInstance()->ExpiringTimer();
	if( pTimer != NULL )
	{
		return pTimer->getHandle();
	}

	return 0;
}
