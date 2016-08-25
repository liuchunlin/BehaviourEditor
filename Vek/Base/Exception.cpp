#include "Types.h"
#include "Exception.h"

using namespace Vek;

Exception::Exception()
{
}

Exception::Exception(const char * format,...)
{
	MAKE_EXCEPTION_STRING(format)
}

Exception::~Exception() throw ()
{
}

const char * Exception::what() const throw()
{
	return m_str.c_str();
}
