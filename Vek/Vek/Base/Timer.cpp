#include "Timer.h"

#include "TimerBank.h"

using namespace Vek;

Timer::Timer( double delay, double interval, TIMBER_CALLBACK fnInterval, TIMBER_CALLBACK fnFinish, double lift, const vector<void*>& uds )
:Delay(delay)
,Interval(interval)
,cbInterval(fnInterval)
,cbFinish(fnFinish)
,Start(0)
,Lift(lift)
,LastTick(0)
,LastInterval(0)
,UserDatas(uds)
,bAlive(true)
{
	TimerBank::GetInstance()->Add( this );
}
Timer::~Timer()
{

}
int	Timer::Tick( float AppTime )
{
	if( LastTick == 0 )
		LastTick = AppTime;
	DWORD nTick = AppTime - LastTick;
	LastTick = AppTime;
	if( Start <= 0 )
	{
		Delay -= nTick;
		if( Delay <= 0 )
		{
			Start = AppTime + Delay;
			LastInterval = Start;
		}
	}
	else
	{
		if( Lift != 0 && Start + Lift <= AppTime )
		{
			bAlive = false;
			if( cbFinish != NULL  )
			{
				cbFinish( UserDatas, this );
			}
		}
		else if( cbInterval != NULL && AppTime - LastInterval > Interval )
		{
			cbInterval( UserDatas, this );
			LastInterval += Interval;
		}
	}

	return 0;
}
