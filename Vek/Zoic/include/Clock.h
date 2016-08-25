/*! @file
	@ingroup buildin
	@brief 定义时钟类
*/

#ifndef __ZOIC_CLOCK_H
#define __ZOIC_CLOCK_H

#include "Types.h"
#include <Vek/Base/Module.h>
#include <time.h>
#ifdef HAVE_MINGW32
#include <winsock2.h>
#ifdef _MSC_VER
#pragma comment (lib, "winmm.lib")
#endif
#endif

namespace Zoic
{
	/*! @brief 类信息存储模板
		@ingroup buildin
	*/
	class Clock: public Vek::Module<Clock>
	{
		friend class Vek::Singleton<Clock>;
	public :
		TIME_SEC			getTime();				//获取时间
		const timeval&		getTimeVal();			//获取时间
		DWORD				getSeconds();			//取得逝去的秒数
		const TIME_MSEL&	getMilliSeconds();		//取得逝去的毫秒数
		void				setTime();				//设置时间,更新时钟状态

	private:
		Clock();
		virtual int			Tick( float DeltaTime );//运行例程，更新时间
	private:
		timeval				m_current;				//当前时间
		DWORD				m_second;				//逝去的秒数
		TIME_MSEL 			m_millisecond;			//逝去的毫秒数
	#ifdef HAVE_MINGW32
		static int			gettimeofday(struct timeval *tp, struct timezone *tzp);
		DWORD				m_tailMillisecond;
	#endif
	};

	inline Clock::Clock()
	{
	#ifdef HAVE_MINGW32
		m_millisecond=m_tailMillisecond=timeGetTime();
	#endif
		setTime();
	}

	inline int Clock::Tick(float AppTime)
	{
		setTime();
		return 0;
	}
	
	inline void Clock::setTime()
	{
		gettimeofday(&m_current, NULL);
	#ifdef HAVE_MINGW32
		DWORD now = timeGetTime();
		DWORD up = now - m_tailMillisecond;
		m_tailMillisecond = now;
		m_millisecond +=up;
		m_second = static_cast<DWORD>(m_millisecond / 1000);
	#else
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC,&ts);
		m_second=ts.tv_sec;
		m_millisecond=(TIME_MSEL)ts.tv_sec*1000+ts.tv_nsec/1000000;
	#endif
	}

	inline TIME_SEC Clock::getTime()
	{
		return m_current.tv_sec;
	}

	inline const timeval & Clock::getTimeVal()
	{
		return m_current;
	}

	inline unsigned long Clock::getSeconds()
	{
		return m_second;
	}

	inline const TIME_MSEL & Clock::getMilliSeconds()
	{
		return m_millisecond;
	}
	#define g_Clock Zoic::Clock::GetInstance()
}
#endif
