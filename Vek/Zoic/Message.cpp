#include "include/Message.h"
#include "include/OutOfBoundException.h"
#include <algorithm>

using namespace Zoic;

Message::Message()
{
	clear();
}

void Message::readData(void * pData, int n)// throw (OutOfBoundException *)
{
	if( m_pRead + n > m_pBuf + NETWORK_BUF_SIZE)
		throw new OutOfBoundException("[ID:%d] Message::ReadData > NETWORK_BUF_SIZE",ID());
	if( m_pRead + n > m_pBuf + getSize())
		throw new OutOfBoundException("[ID:%d] Message::ReadData > GetSize()",ID());

	memcpy(pData, m_pRead, n);
	m_pRead += n;
}

void Message::skipData(int n)
{
	if( m_pRead + n > m_pBuf + NETWORK_BUF_SIZE)
		throw new OutOfBoundException("[ID:%d] Message::ReadData > NETWORK_BUF_SIZE",ID());
	if( m_pRead + n > m_pBuf + getSize())
		throw new OutOfBoundException("[ID:%d] Message::ReadData > GetSize()",ID());
	m_pRead += n;
}

void Message::writeData(const void * pData, int n)// throw (OutOfBoundException *)
{
	if( m_pWrite + n > m_pBuf + NETWORK_BUF_SIZE )
		throw new OutOfBoundException("[ID:%d] Message::WriteData > NETWORK_BUF_SIZE",ID());
	
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
	
Message & Message::operator >> ( std::string &val )
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
		throw new OutOfBoundException("[ID:%d] Message>>string",ID());
	}
	return * this;
}

Message & Message::operator >> ( PCSTR &val )
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
		throw new OutOfBoundException("[ID:%d] Message>>PPCSTR",ID());
	}
	return * this;
}

void Message::readString(char *buffer, int maxLen)
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
		throw new OutOfBoundException("[ID:%d] Message::readString",ID());
	}
}

namespace Zoic
{
	void throwOutOfBoundException(
			const char * format,	//!< ��ʽ�����ַ���
			...)
	{
		const char * m_str;
		MAKE_EXCEPTION_STRING(format)
		throw new OutOfBoundException("%s",m_str);
	}
}

