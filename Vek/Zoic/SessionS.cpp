#include "include/SessionS.h"
#include "include/CommandFactory.h"

using namespace Zoic;

SessionS::SessionS():
	m_ready(true),
	m_commandFactory(NULL)
{
}

bool SessionS::parseMessage(Message &msg)
{
	return m_commandFactory->parseMessage(this,msg);
}
