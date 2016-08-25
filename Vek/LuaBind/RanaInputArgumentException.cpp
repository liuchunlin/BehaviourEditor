#include "RanaInputArgumentException.h"

RanaInputArgumentException::RanaInputArgumentException(const char *str):
	m_str(str),
	m_level(0),
	m_array(false)
{
}

RanaInputArgumentException::~RanaInputArgumentException() throw()
{
}

const char *RanaInputArgumentException::what() const throw()
{
	return m_str.c_str();
}

void RanaInputArgumentException::appendPrefix(const char *name,bool array)
{
	if(m_level&&!m_array)
	{
		m_str=name+("."+m_str);
	}
	else
	{
		m_str=name+m_str;
	}
	m_array=array;
	m_level++;
}
