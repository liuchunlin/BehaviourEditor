#include "TimerBank.h"
#include <algorithm>

using namespace Vek;

TimerBank::TimerBank()
{
}
DWORD	TimerBank::Add( Timer* pEntry )
{
	m_Timers.push_back( pEntry );
	return 0;
}
bool	TimerBank::Remove( Timer* pEntry )
{
	//TODO:remove_if
	TimerEntrys::iterator iter( m_Timers.begin() );
	for( ; iter != m_Timers.end(); ++iter )
	{
		Timer* pIter = (*iter);
		if( pEntry == pIter )
		{
			m_Timers.erase( iter );
			return true;
		}
	}

	return false;
}

int	TimerBank::Flush()
{
	std::for_each( m_Timers.begin(), m_Timers.end(), Deletor<Timer>() );
	m_Timers.clear();
	return 0;
}

int	TimerBank::Tick( float AppTime )
{
	for each (auto timer in m_Timers)
	{
		timer->Tick(AppTime);
	}

	TimerEntrys::iterator iter( m_Timers.begin() );
	for( ; iter != m_Timers.end(); )
	{
		const Timer* pTE = (*iter);
		if( !pTE->Alive() )
		{
			SAFE_DELETE( pTE );
			iter = m_Timers.erase( iter );
		}
		else
		{
			++iter;
		}
	}
	return 0;
}
