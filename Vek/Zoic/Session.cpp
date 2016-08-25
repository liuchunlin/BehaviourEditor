#include <new>
#include "include/Session.h"
#include "include/Message.h"
#include "include/Clock.h"
#include "include/SysPing.h"
#include "include/Exception.h"
#include "include/SessionError.h"

#ifndef HAVE_MINGW32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_MINGW32
#define ERRORNO_EWOULDBLOCK WSAEWOULDBLOCK
#define GET_ERRORNO (WSAGetLastError())
#else
#define ERRORNO_EWOULDBLOCK EAGAIN
#define GET_ERRORNO (errno)
#endif

#ifdef HAVE_MINGW32
typedef int socklen_t;
#endif

using namespace Zoic;

Session::Session():
	m_send_buffer_size(0),
	m_recv_buffer_size(0),
	m_init_send_buffer_size(8*NETWORK_BUF_SIZE),
	m_init_recv_buffer_size(NETWORK_BUF_SIZE),
	m_max_dynamic_sendbuf_size(10*1024*1024),	//10M
	m_sendbuf_dynamic(false),
	m_release_dynamic_sendbuf(false),
	_m_send_buffer(NULL),
	_m_recv_buffer(NULL),
	m_send_buffer(NULL),
	m_recv_buffer(NULL),
	//���±�����closeʱҪ����	
	m_socket(INVALID_SOCKET),
	m_lastReceivedTick(0),
	m_send_size(0),
	m_recv_size(0),
	m_packet_len(0),
	m_error(0),
	m_systemError(0),
	m_syspingCount(0),
	m_syspingTimes(&g_sysPingTime)
{

}

Session::~Session()
{
	close();
	clearSendBuffer();
	clearRecvBuffer();
}


bool Session::open(SOCKET sock,const sockaddr_in &addr)
{
	m_ip=inet_ntoa(addr.sin_addr);
	m_port=ntohs(addr.sin_port);
#ifdef HAVE_MINGW32
	unsigned long sw = 1;
	if(ioctlsocket(sock, FIONBIO, &sw))
	{
		return false;
	}
#else	
	if(fcntl(sock, F_SETFL, FNDELAY)==-1)
	{
		return false;
	}
#endif
	m_socket=sock;

	sockaddr_in localAddr;
	socklen_t length = sizeof(localAddr);
	if( getsockname(m_socket,(sockaddr *)&localAddr,&length) == 0 )
	{
		m_local_ip =inet_ntoa(localAddr.sin_addr);
		m_local_port=ntohs(localAddr.sin_port);
	}
	else
	{
		m_local_ip = "";
		m_local_port = 0;
	}
	
	netInitialize();
	setCanRecvData(true);
	setCanSendData(true);
	joinSelector(m_socket);
	return true;
}

void Session::netInitialize()
{
	m_error=0;
	m_systemError = 0;
	initBuffer();
	m_lastReceivedTick=g_Clock->getMilliSeconds();
}

bool Session::routine()
{
	if(canRecvData())
	{
		recvData();
		setCanRecvData(false);
	}
	{//���Է��Ƿ񵱻�
		if(m_syspingCount >= m_syspingTimes->getMaxPingCount())
		{//�ѳ������̽��ʱ��
		}
		else if(g_Clock->getMilliSeconds()-m_lastReceivedTick >= m_syspingTimes->getNextPingTime(m_syspingCount) )
		{
			++m_syspingCount;
			if(m_syspingCount >= m_syspingTimes->getMaxPingCount())
			{//�ѳ������̽��ʱ��
				setError(ERROR_PINGTIMEOUT);
			}
			else
			{//����̽���
				Message msg;
				msg.ID(SYSPING_REQ);
				setCanSendData(true);
				sendMessage(msg);
			}
		}
	}
	if(canSendData())
	{
		sendData();
		setCanSendData(false);
	}
	if(m_error)
	{//����
		//����ǰ�������һ������
		sendData();
		return false;
	}
	else
	{
		return true;
	}
}

void Session::close()
{
	if(m_socket!=INVALID_SOCKET)
	{
		quitSelector();
		closesocket(m_socket);
		m_socket=INVALID_SOCKET;
	}
	m_send_size=0;
	m_recv_size=0;
	m_packet_len=0;
	m_syspingCount=0;
	if(m_release_dynamic_sendbuf)
	{
		revertBuffer();
	}
}

void Session::clearSendBuffer()
{
	if(!m_send_buffer_size)
	{
		ASSERT(!_m_send_buffer);
		return;
	}
	ASSERT(_m_send_buffer);
	delete [] _m_send_buffer;
	_m_send_buffer = NULL;
	m_send_buffer = NULL;
	m_send_buffer_size = 0;
}

void Session::clearRecvBuffer()
{
	if(!m_recv_buffer_size)
	{
		ASSERT(!_m_recv_buffer);
		return;
	}
	ASSERT(_m_recv_buffer);
	delete [] _m_recv_buffer;
	_m_recv_buffer = NULL;
	m_recv_buffer = NULL;
	m_recv_buffer_size = 0;
}

void Session::revertBuffer()
{
	if( m_init_send_buffer_size != m_send_buffer_size )
	{
		clearSendBuffer();
	}
}

bool Session::createBuffer(DWORD & size,char * & raw,char * & data)
{
	if(size < NETWORK_BUF_SIZE)
	{
		size = NETWORK_BUF_SIZE;
	}
	char * buf = new(std::nothrow) char [offsetof(Message::Fake,buffer)+size+1];
	if(!buf)
	{
		return false;
	}
	raw = buf;
	data=raw+offsetof(Message::Fake,buffer);
	data[size-1]=0;
	return true;
}

void Session::initBuffer()
{
	if(m_init_send_buffer_size < NETWORK_BUF_SIZE)
	{
		ASSERT(0);
		m_init_send_buffer_size = NETWORK_BUF_SIZE;
	}
	if(m_send_buffer_size < m_init_send_buffer_size)
	{
		clearSendBuffer();
		m_send_buffer_size = m_init_send_buffer_size;
		createBuffer(m_send_buffer_size,_m_send_buffer,m_send_buffer);
	}
	if(m_init_recv_buffer_size < NETWORK_BUF_SIZE)
	{
		ASSERT(0);
		m_init_recv_buffer_size = NETWORK_BUF_SIZE;
	}
	if(m_recv_buffer_size<m_init_send_buffer_size)
	{
		clearRecvBuffer();
		m_recv_buffer_size = m_init_recv_buffer_size;
		createBuffer(m_recv_buffer_size,_m_recv_buffer,m_recv_buffer);
	}
}

bool Session::parseMessage(Message &msg)
{
	return true;
}

void Session::sendData()
{
	if(m_send_size==0)
	{
		return;
	}

	int len=send(m_socket,(const char *)m_send_buffer,m_send_size,0);
	if(len>0)
	{
		adjustSendBuffer(len);
	}
	else
	{
		if(m_error==0)
		{
			if (len==0)
			{
				setError(ERROR_SENDFAILED);
			}
			else if( len<0 )
			{
				const int errorNo = GET_ERRORNO;
				if(errorNo != ERRORNO_EWOULDBLOCK)
				{
					m_systemError = errorNo;
					setError(ERROR_SENDFAILED2);
				}
			}			
		}
		return;
	}
}

inline bool Session::pushData(const BYTE *data,WORD size)
{
	if(!m_send_buffer)
	{
		return false;
	}
	
	do
	{
		if(m_send_size+size <= m_send_buffer_size)
		{
			memcpy(m_send_buffer+m_send_size,data,size);
			m_send_size+=size;
			return true;
		}
	}
	while(increaseSendBuffer());

	FILE *fp=fopen("full.dat","wb+");
	if(fp!=NULL)
	{
		fwrite(m_send_buffer,m_send_size,1,fp);
		fclose(fp);
	}
	setError(ERROR_SENDBUFFER);
	return false;
}

bool Session::sendMessage(const Message &msg)
{
	return pushData(msg.getData(),msg.getSize());
}

#ifdef HAVE_MINGW32
namespace
{
	class loadWinSock
	{
	public:
		loadWinSock()
		{
			WSADATA w;
			WORD wVersionRequested = MAKEWORD( 1, 1 );
      		if(WSAStartup(wVersionRequested, &w) != 0)
      		{
      			puts("Can't load WinSock DLL.");
      			exit(1);
      		}
		}
		~loadWinSock()
		{
			WSACleanup();
		}
	} load;
}
#endif

void Session::recvData()
{
	int space=m_recv_buffer_size - m_recv_size;
	//ASSERT(space > 0);
	int len = ::recv(m_socket,(char *)m_recv_buffer+m_recv_size,space,0);
	if(len>0)
	{
		m_recv_size+=len;

		m_syspingCount = 0;
		m_lastReceivedTick = g_Clock->getMilliSeconds();

		uppack();
	}
	else
	{
		if(m_error==0)
		{
			if (len==0)
			{
				setError(ERROR_REMOTECLOSED);
			}
			else if( len<0 ) 
			{
				const int errorNo = GET_ERRORNO;
				if(errorNo != ERRORNO_EWOULDBLOCK)
				{
					m_systemError = errorNo;
					setError(ERROR_RECVFAILED);
				}
			}			
		}
		return;
	}
}

void Session::adjustRecvBuffer(DWORD handled_size)
{
	if(handled_size)
	{
		m_recv_size-=handled_size;
		if(m_recv_size)
		{
			memmove(m_recv_buffer,m_recv_buffer+handled_size,m_recv_size);
		}
	}
}

void Session::adjustSendBuffer(DWORD handled_size)
{
	m_send_size-=handled_size;
	if(m_send_size)
	{//���ֻ�����˲�������,��ʣ��Ĳ�����ǰ��
		memmove(m_send_buffer,m_send_buffer+handled_size,m_send_size);
	}
}

bool Session::increaseSendBuffer()
{
	//������̬����
	if(!m_sendbuf_dynamic)
	{
		return false;
	}

	//�Ѵﵽ���ֵ,����������
	if(m_send_buffer_size >= m_max_dynamic_sendbuf_size)
	{
		return false;
	}

	//�����µĻ���
	DWORD buffer_size = m_send_buffer_size*2;
	if(buffer_size > m_max_dynamic_sendbuf_size)
	{
		buffer_size = m_max_dynamic_sendbuf_size;
	}
	char * raw;
	char * data;
	if(!createBuffer(buffer_size,raw,data))
	{
		return false;
	}

	//�ƶ�����
	memmove(data,m_send_buffer,m_send_size);
	m_send_buffer = data;
	if(_m_send_buffer)
	{
		delete [] _m_send_buffer;
	}
	_m_send_buffer = raw;
	m_send_buffer_size = buffer_size;
	return true;
}

void Session::uppack()
{
	char* msg_buf = m_recv_buffer;
	DWORD index=0;	//�Ѵ�������ݰ�
	for(;;)
	{
		if(m_packet_len==0)
		{//��ð�����
			if(index + 2 > m_recv_size)
			{
				break;
			}
			m_packet_len = *(WORD *)msg_buf;
			if(m_packet_len > NETWORK_BUF_SIZE||m_packet_len < 2)
			{
				setError(ERROR_UNPACK);
				return;
			}
		}
		if( index + m_packet_len > m_recv_size)
		{//���������İ�
			break;
		}
		m_receivedMessage = (Message *)(msg_buf-offsetof(Message::Fake,buffer));
		m_receivedMessage->clear();
		if( !parseMessage(*m_receivedMessage))
		{
			if(!m_error)
			{
				setError(ERROR_PARSEMESSAGE);
			}
			return;
		}
		index+=m_packet_len;
		msg_buf+=m_packet_len;
		m_packet_len=0;		
	}
	adjustRecvBuffer(index);
}

bool Session::setSysPingTime(const SysPingTime* sysPingTime)
{
	if(!sysPingTime)
	{
		return false;
	}
	m_syspingTimes=sysPingTime;
	return true;
}
