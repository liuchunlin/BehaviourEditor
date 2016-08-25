#include "RanaMessage.h"

RanaMessage::RanaMessage()
{
	m_eventType = RANA_INVALID_WORD;
	m_bTriggerFirstOnly = false;
}

RanaMessage::~RanaMessage()
{
}

void RanaMessage::setEventType(RanaEventType _eventType)
{
	m_eventType = _eventType;
}

RanaWORD RanaMessage::getEventType()
{
	return m_eventType;
}

