#pragma once

#include "Types.h"
#include "Module.h"
#include "Timer.h"

namespace Vek
{
	typedef		list<Timer*> TimerEntrys;
	class TimerBank: public Vek::Module<TimerBank>
	{
		friend class Vek::Singleton<TimerBank>;
	public :
		DWORD				Add( Timer* pEntry );
		bool				Remove( Timer* pEntry );
	protected:
		TimerBank();
		virtual int			Tick( float AppTime );
		virtual int			Flush();
	private:
		TimerEntrys			m_Timers;
	};
}
