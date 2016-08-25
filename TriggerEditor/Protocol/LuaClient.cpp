#include "LuaClient.h"
#include <MMSystem.h>
#include <Zoic/include/Clock.h>
#include <Zoic/include/Selector.h>
#include <Zoic/include/SessionCommand.h>
#include <Zoic/include/ProtocolVersion.h>
#include <Zoic/include/UnhandledMessageException.h>
#include <Zoic/include/Netlog.h>
#include "Process_LuaProtocol.h"

LuaClient *GLuaClient = LuaClient::GetInstance();

LuaClient::LuaClient()
{
	m_ServerIP = "127.0.0.1";
	m_ServerPort = 9000;

	m_kNetState = NET_WAIT_FOR_CONNECT;
	m_init_recv_buffer_size = NETWORK_BUF_SIZE * 512;
} 

LuaClient::~LuaClient()
{
}
//连接游戏服务器
bool	LuaClient::ConnectGameServer()
{
	if( m_kNetState == NET_CONNECT || m_kNetState == NET_WAIT_FOR_CONNECT )
		CloseServer();

	DoConnect( m_ServerIP.c_str(), m_ServerPort );

	return ( m_kNetState == NET_CONNECT );
}

//断开服务器
void	LuaClient::CloseServer()
{
	SessionC::close();
	if(m_kNetState == NET_CONNECT)
	{
		m_kNetState = NET_DISCONNECT;
	}
	GLuaProto->UpdateConnectState();
}

bool LuaClient::DoConnect( const char *ip, PORT port )
{
	setConnectAddress(ip, port);
	int iResult = Zoic::SessionC::start();

	iResult = Zoic::SessionC::run();
	if(iResult != 0)
	{
		//MessageBox(NULL, TEXT("Can't connect"), TEXT("error"), 0);
		m_kNetState = NET_WAIT_FOR_CONNECT;
	}
	else
	{
		m_kNetState = NET_CONNECT;
	}
	GLuaProto->UpdateConnectState();
	return m_kNetState == NET_CONNECT;
}

bool LuaClient::open(SOCKET sock, const sockaddr_in &addr)
{
	bool ret= SessionC::open(sock, addr);
	if(ret)
	{
		printf("CommandClient connect to %s:%d\n", getIP(), getPort());
	}

	return ret;
}

void LuaClient::DoStart()
{
	assert( false );

	//int iResult = SessionC::start();
	//if(iResult != 0)
	//	m_kNetState = NET_WAIT_FOR_CONNECT;
	//else
	//	m_kNetState = NET_CONNECT;

	//GLuaProto->UpdateConnectState();
}

void LuaClient::DoReset()
{
	m_kNetState = NET_WAIT_FOR_CONNECT;
	GLuaProto->UpdateConnectState();
}

bool LuaClient::sendMessage(const Zoic::Message &msg)
{
	return Zoic::SessionC::sendMessage(msg);
}

int	LuaClient::Init()
{
	ConnectGameServer();
	LuaProto_Set::NetProcess::TS_Login_Req("","");

	return 0;
}

int	LuaClient::Flush()
{
	if(m_kNetState == NET_CONNECT)
	{
		Zoic::SessionC::close();
	}
	return 0;
}

int	LuaClient::Tick( float appTime )
{
	int iResult = 0;
	if(m_kNetState == NET_CONNECT)
	{
		iResult = Zoic::SessionC::run();

		if(iResult != 0)
		{
			Zoic::SessionC::close();
			m_kNetState = NET_DISCONNECT;
			OnNetDisconnected();
		}
	}
	else
	{
		static float s_lastRetryTime = 0;
		static float s_waitTime = 3.0f;
		s_waitTime = m_kNetState == NET_DISCONNECT ? 3.0f : min(s_waitTime * 1.5f, 180.0f);
		if (appTime - s_lastRetryTime > s_waitTime)
		{
			s_lastRetryTime = appTime;
			ConnectGameServer();
			LuaProto_Set::NetProcess::TS_Login_Req("", "");
		}
	}

	return iResult;
}

void LuaClient::OnNetDisconnected()
{
	GLuaProto->UpdateConnectState();
	//MessageBox(NULL, TEXT("Net disconnected !"), TEXT("error"), 0);
}

bool LuaClient::parseMessage(Zoic::Message &msg)
{
	try
	{
		return SessionC::parseMessage(msg);
	}
	catch(Zoic::UnhandledMessageException * e )
	{
		delete e;
		e = NULL;
		return true;
	}
	catch(std::exception * e)
	{
		delete e;
		e = NULL;
		return false;
	}
}

