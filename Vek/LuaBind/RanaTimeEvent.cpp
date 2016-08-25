#include "RanaTimeEvent.h"
#include "RanaTimeMessage.h"

RanaTimeEvent::RanaTimeEvent()
{
	setType(RANA_TIME_EVENT);
	m_timerHandle = RANA_INVALID_TIMERHANDLE;
}

RanaTimeEvent::~RanaTimeEvent()
{
}

void RanaTimeEvent::setTimerHandle(RanaTimerHandle _handle)
{
	m_timerHandle = _handle;
}

bool RanaTimeEvent::preValidate(RanaMessage *_msg)
{
	if( _msg == NULL || _msg->getEventType() != RANA_TIME_EVENT )
	{
		RANA_ASSERT( false );
		return false;
	}

	RanaTimeMessage *timeMsg = (RanaTimeMessage*)(_msg);
	if (!timeMsg)
	{
		debug_ranaLog("RanaTimeEvent::preValidate : error msg RTTI!\n");
		return false;
	}
	RanaTimerHandle handle = timeMsg->getTimerHandle();
	if (m_timerHandle == handle)
	{
		return true;
	}

	return false;
}

void RanaTimeEvent::getArgument(std::vector<RanaVariant> & args)
{
}
