#pragma once

#include "Types.h"

namespace Vek
{
	class Timer
	{
		typedef void		(*TIMBER_CALLBACK)( const vector<void*>& uds, Timer* pSelf );
	public:
		Timer( double delay, double interval, TIMBER_CALLBACK fnInterval, TIMBER_CALLBACK fnFinish, double lift, const vector<void*>& uds );
		virtual ~Timer();
		virtual int			Tick( float AppTime );
		bool				Alive() const { return bAlive; }
	public:
		//In
		double				Delay;			//延迟等待时间
		double				Interval;		//间隔触发时间
		TIMBER_CALLBACK		cbInterval;		//间隔触发函数
		TIMBER_CALLBACK		cbFinish;		//结束触发函数
		vector<void*>		UserDatas;		//数据
		//In\Update
		double				Lift;			//生命剩余时间
		//Update
		double				LastTick;		//上次嘀嗒时间
		double				Start;			//开始时间
		double				LastInterval;	//上次触发间隔
		bool				bAlive;			//是否健存
	};
}
