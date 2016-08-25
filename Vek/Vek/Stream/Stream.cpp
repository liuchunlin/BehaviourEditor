#include "Stream.h"
#include "../Base/Exception.h"
#include <algorithm>

using namespace Vek;

Stream::Stream()
{
	clear();
}

void Stream::readData(void * pData, int n)// throw (OutOfBoundException *)
{
	if( m_pRead + n > m_pBuf + NETWORK_BUF_SIZE_)
		throw new Exception("[ID:%d] Message::ReadData > NETWORK_BUF_SIZE",ID());
	if( m_pRead + n > m_pBuf + getSize())
		throw new Exception("[ID:%d] Message::ReadData > GetSize()",ID());

	memcpy(pData, m_pRead, n);
	m_pRead += n;
}

void Stream::skipData(int n)
{
	if( m_pRead + n > m_pBuf + NETWORK_BUF_SIZE_)
		throw new Exception("[ID:%d] Message::ReadData > NETWORK_BUF_SIZE",ID());
	if( m_pRead + n > m_pBuf + getSize())
		throw new Exception("[ID:%d] Message::ReadData > GetSize()",ID());
	m_pRead += n;
}

void Stream::writeData(const void * pData, int n)// throw (OutOfBoundException *)
{
	if( m_pWrite + n > m_pBuf + NETWORK_BUF_SIZE_ )
		throw new Exception("[ID:%d] Message::WriteData > NETWORK_BUF_SIZE",ID());
	
	memcpy(m_pWrite, pData, n);
	m_pWrite += n;
	*m_pSize += n;
}

static inline int z_strlen( const char * p, int nMaxLen)
{
	int i;
	for ( i = 0 ; i < nMaxLen ; i++ )
	{
		if ( p[i] == '\0' )
			break;
	}
	if ( i == nMaxLen )
		return -1;
	return i;
}
	
static inline int z_strlen( const wchar_t * p, int nMaxLen)
{
	int i;
	for ( i = 0 ; i < nMaxLen ; i++ )
	{
		if ( p[i] == 0 )
			break;
	}
	if ( i == nMaxLen )
		return -1;
	return i;
}

Stream & Stream::operator >> ( std::string &val )
{
	int nLen = z_strlen( reinterpret_cast<const char *>(m_pRead), static_cast<int>( m_pBuf + getSize() - m_pRead ));
	if ( nLen != -1)
	{
		nLen++;
		val=reinterpret_cast<const char *>(m_pRead);
		m_pRead+=nLen;
	}
	else
	{
		val = "(null)";
		throw new Exception("[ID:%d] Message>>string",ID());
	}
	return * this;
}

Stream & Stream::operator >> ( std::wstring &val )
{
	int nLen = z_strlen( reinterpret_cast<const wchar_t *>(m_pRead), static_cast<int>( m_pBuf + getSize() - m_pRead ) / sizeof(wchar_t) );
	if ( nLen != -1)
	{
		nLen++;
		val = reinterpret_cast<const wchar_t *>(m_pRead);
		m_pRead += nLen * sizeof(wchar_t);
	}
	else
	{
		val = L"(null)";
		throw new Exception("[ID:%d] Message>>string",ID());
	}
	return * this;
}

Stream & Stream::operator >> ( PCSTR &val )
{
	int nLen = z_strlen( reinterpret_cast<const char *>(m_pRead), static_cast<int>( m_pBuf + getSize() - m_pRead ));
	if ( nLen != -1)
	{
		nLen++;
		val=reinterpret_cast<const char *>(m_pRead);
		m_pRead+=nLen;
	}
	else
	{
		val = "(null)";
		throw new Exception("[ID:%d] Message>>PPCSTR",ID());
	}
	return * this;
}

void Stream::readString(char *buffer, int maxLen)
{
	int nLen = z_strlen( (const char *)m_pRead, std::min(maxLen,static_cast<int>( m_pBuf + getSize() - m_pRead )));
	if( nLen!=-1)
	{
		nLen++;
		memcpy(buffer,m_pRead,nLen);
		m_pRead+=nLen;
	}
	else
	{
		throw new Exception("[ID:%d] Message::readString",ID());
	}
}

namespace Vek
{
	void throwOutOfBoundException(
			const char * format,	//!< ¸ñÊ½¿ØÖÆ×Ö·û´®
			...)
	{
		const char * m_str;
		MAKE_EXCEPTION_STRING(format)
			throw new Exception("%s",m_str);
	}

	void setString(Stream & msg,const char *str,WORD buffSize)
	{
		size_t strLength = strlen(str);
		if( strLength >= static_cast<size_t>( buffSize ) )
		{
			Vek::Exception("[ID:%d] Message::<< String size(%d) larger than %d",msg.ID(),strLength,buffSize-1);
			//msg.writeData(str, buffSize - 1 );
			//msg.writeData("\0", 1);
			//return;
		}
		msg.writeData(str, static_cast<int>( strLength + 1) );
	}

	void getString(Stream & msg,char *str,WORD buffSize)
	{
		msg.readString(str,buffSize);
	}
}

