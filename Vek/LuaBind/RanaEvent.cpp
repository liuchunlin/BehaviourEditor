#include "RanaEvent.h"

RanaEvent::RanaEvent()
{
	m_type = RANA_INVALID_EVENTTYPE;
	m_isOn = false;
}

RanaEvent::~RanaEvent()
{
}
bool RanaEvent::operator ==(const RanaEvent &obj) const
{
	if( m_triggerHandle != obj.m_triggerHandle )
	{
		return false;
	}
	if( m_type != obj.m_type )
	{
		return false;
	}
	if( m_args.size() != obj.m_args.size() )
	{
		return false;
	}
	for( std::vector<RanaVariant>::size_type i=0; i < m_args.size(); ++i )
	{
		const RanaVariant& l = m_args[i];
		const RanaVariant& r = obj.m_args[i];
		if( l.operator ==( r ) )
			continue;
		else
			return false;
	}
	return true;
}
void RanaEvent::setType(RanaEventType _type)
{
	m_type = _type;
}

RanaEventType RanaEvent::getType()
{
	return m_type;
}

void RanaEvent::setOnOff(bool _isOn)
{
	m_isOn = _isOn;
}

bool RanaEvent::getOnOff()
{
	return m_isOn;
}

RanaSpace* RanaEvent::getSelfSpace()
{
	return m_pRanaSpace;
}
#if 0	//FORNAME
void RanaEvent::setName(const RanaEventName &_name)
{
	m_name = _name;
}

const RanaEventName &RanaEvent::getName()
{
	return m_name;
}
#endif