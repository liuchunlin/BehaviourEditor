#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <Vek/Base/Module.h>
#include <Zoic/include/SessionC.h>
#include <Zoic/include/Message.h>
#include <Zoic/include/SessionCommand.h>
#include <vector>
#include <string>
#include <map>
#include <deque>

using namespace Zoic;

typedef unsigned short PORT;

//----------------------------------------------------------------
class LuaClient : public Zoic::SessionC, public Vek::Module<LuaClient>
{
    friend class Vek::Singleton<LuaClient>;

public:

    ON_MESSAGE

    // ����״̬
    enum NetState
    {
		NET_WAIT_FOR_CONNECT = WM_USER,	// �ȴ�����״̬
		NET_CONNECT,					// ��������״̬
		NET_DISCONNECT					// ���ڶϿ�״̬
	};

	//
	bool			ConnectGameServer();		//������Ϸ������
	void			CloseServer();				//�Ͽ�������

	virtual bool	sendMessage( const Zoic::Message &msg );	//!������Ϣ
	NetState		GetNetState() { return m_kNetState; }

protected:
	virtual bool	parseMessage(Zoic::Message &msg);
	virtual bool	open( SOCKET sock, const sockaddr_in &addr );

	void			DoStart();
	void			DoReset();
	bool			DoConnect( const char *ip, PORT port );

	void			OnNetDisconnected();

private:
	//Module
	LuaClient();
	~LuaClient();

	int				Init();
	int				Tick( float appTime );
	int				Flush();

public:
	std::string		m_ServerIP;
	PORT			m_ServerPort;

protected:
	NetState		m_kNetState;
};

extern LuaClient *GLuaClient;
#define SEND_MSG(PRO, v) \
{ \
	Zoic::Message msg; \
	msg.ID(PRO) << v; \
	LuaClient::GetInstance()->sendMessage(msg); \
}

#define SESSION_PACKET(DataSet, ID, Receive) \
	SESSION_COMMAND(LuaClient, ID) \
{ \
	Receive rcv; \
	msg >> rcv; \
	DataSet::NetProcess::OnProcessSerMsg(rcv); \
	return true; \
}

#define SESSION_PACKET2(MsgID, MsgDataType) \
	static void OnProcessSerMsg(const MsgDataType& message); \
	SESSION_COMMAND(LuaClient, MsgID) \
{ \
	MsgDataType rcv; \
	msg >> rcv; \
	OnProcessSerMsg(rcv); \
	return true; \
} 

#endif