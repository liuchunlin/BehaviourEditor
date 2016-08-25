#include <LuaBind/RanaSpace.h>

#include <LuaBind/RanaPush.h>
#include <LuaBind/RanaInputStack.h>
#include <Vek/Base/Exception.h>
#include "RanaSpaceType.h"
#include "RanaType.h"


//TEST
extern void r_TEST(RanaState L, const RNT_INT::TYPE& k);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, TEST);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, TEST);
int l_TEST(RanaState L)
{
	RNT_INT::TYPE k;
	RANA_INPUT_BEGIN
		s>>k;
	RANA_INPUT_END(TEST)
		r_TEST(L,k);
	return 0;
}
//无动作
extern void r_DoNothing(RanaState L);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, DoNothing);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, DoNothing);
int l_DoNothing(RanaState L)
{
	RANA_INPUT_BEGIN
		s;
	RANA_INPUT_END(DoNothing)
		r_DoNothing(L);
	return 0;
}

//等待条件
extern void r_WaitForCondition(RanaState L, const RNT_CONDITION::TYPE& condition, const RNT_INT::TYPE& checkTick);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, WaitForCondition);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, WaitForCondition);
int l_WaitForCondition(RanaState L)
{
	RNT_CONDITION::TYPE condition;
	RNT_INT::TYPE checkTick;
	RANA_INPUT_BEGIN
		s>>condition>>checkTick;
	RANA_INPUT_END(WaitForCondition)
		r_WaitForCondition(L,condition,checkTick);
	return 0;
}
//略过剩余动作
extern void r_SkipRemainingActions(RanaState L);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, SkipRemainingActions);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, SkipRemainingActions);
int l_SkipRemainingActions(RanaState L)
{
	RANA_INPUT_BEGIN
		s;
	RANA_INPUT_END(SkipRemainingActions)
		r_SkipRemainingActions(L);
	return 0;
}
//运行触发器
extern void r_RunTrigger(RanaState L, const RNT_TRIGGER::TYPE& tgr, const RNT_ISCHECK::TYPE& bCheckCondition);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, RunTrigger);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, RunTrigger);
int l_RunTrigger(RanaState L)
{
	RNT_TRIGGER::TYPE tgr;
	RNT_ISCHECK::TYPE bCheckCondition;
	RANA_INPUT_BEGIN
		s>>tgr>>bCheckCondition;
	RANA_INPUT_END(RunTrigger)
		r_RunTrigger(L,tgr,bCheckCondition);
	return 0;
}

//激活计时器
extern void r_ActiveTimer(RanaState L, const RNT_ACTIVE::TYPE& active, const RNT_TIMER::TYPE& timer);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, ActiveTimer);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, ActiveTimer);
int l_ActiveTimer(RanaState L)
{
	RNT_ACTIVE::TYPE active;
	RNT_TIMER::TYPE timer;
	RANA_INPUT_BEGIN
		s>>active>>timer;
	RANA_INPUT_END(ActiveTimer)
		r_ActiveTimer(L,active,timer);
	return 0;
}
////运行计时器
//extern void r_StartTimer(RanaState L, const RNT_TIMER::TYPE& timer, const RNT_TIMER_PERIOD::TYPE& rp, const RNT_INT::TYPE& expireTime);
//REG_RANA_FUNC(RANA_SPACETYPE_Battle, StartTimer);
//REG_RANA_FUNC(RANA_SPACETYPE_Scene, StartTimer);
//int l_StartTimer(RanaState L)
//{
//	RNT_TIMER::TYPE timer;
//	RNT_TIMER_PERIOD::TYPE rp;
//	RNT_INT::TYPE expireTime;
//	RANA_INPUT_BEGIN
//		s>>timer>>rp>>expireTime;
//	RANA_INPUT_END(StartTimer)
//		r_StartTimer(L,timer,rp,expireTime);
//	return 0;
//}

//当前触发器
extern RNT_TRIGGER::TYPE r_ThisTrigger(RanaState L);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, ThisTrigger);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, ThisTrigger);
int l_ThisTrigger(RanaState L)
{
	RANA_INPUT_BEGIN
		s;
	RANA_INPUT_END(ThisTrigger)
		rana_push( L, r_ThisTrigger(L) );
	return 1;
}
//刚过期的计时器
extern RNT_TIMER::TYPE r_ExpiringTimer(RanaState L);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, ExpiringTimer);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, ExpiringTimer);
int l_ExpiringTimer(RanaState L)
{
	RANA_INPUT_BEGIN
		s;
	RANA_INPUT_END(ExpiringTimer)
		rana_push( L, r_ExpiringTimer(L) );
	return 1;
}
//运行计时器
extern void r_StartTimer(RanaState L, const RNT_TIMER::TYPE& timer, const RNT_TIMER_PERIOD::TYPE& rp, const RNT_REAL::TYPE& expireTime);
REG_RANA_FUNC(RANA_SPACETYPE_Battle, StartTimer);
REG_RANA_FUNC(RANA_SPACETYPE_Scene, StartTimer);
int l_StartTimer(RanaState L)
{
	RNT_TIMER::TYPE timer;
	RNT_TIMER_PERIOD::TYPE rp;
	RNT_REAL::TYPE expireTime;
	RANA_INPUT_BEGIN
		s>>timer>>rp>>expireTime;
	RANA_INPUT_END(StartTimer)
		r_StartTimer(L,timer,rp,expireTime);
	return 0;
}