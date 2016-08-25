#include "Process_LuaProtocol.h"

#include "LuaClient.h"
#include "TgrApp.h"
#include "TgrFrame.h"

#include "UnStringConv.h"

LuaProto_Set*	GLuaProto = LuaProto_Set::GetInstance();

LuaProto_Set::LuaProto_Set()
{

}
LuaProto_Set::~LuaProto_Set()
{

}
void	LuaProto_Set::Connect()
{
	if( GLuaClient->GetNetState() != LuaClient::NET_CONNECT )
	{
		GLuaClient->CloseServer();
		GLuaClient->ConnectGameServer();
		NetProcess::TS_Login_Req("","");
	}
}
void	LuaProto_Set::DisConnect()
{
	GLuaClient->CloseServer();
}
void	LuaProto_Set::GetIpPort_( wstring& locIp, int& locPort, wstring& serverIp, int& serverPort )
{
	locIp = FANSIToTCHAR( GLuaClient->getLocalIP() );
	locPort = GLuaClient->getLocalPort();
	serverIp = FANSIToTCHAR( GLuaClient->getIP() );
	serverPort = GLuaClient->getPort();
}
void	LuaProto_Set::UpdateConnectState()
{
	LuaClient::NetState eNowState = GLuaClient->GetNetState();
	wxGetApp().GetTgrFrame()->SetConnectState( eNowState == LuaClient::NET_CONNECT );
}
//////////////////////////////////////////////////////////////////////////
//版本号判断
SESSION_PACKET(LuaProto_Set, PROTOCOL_VERSION_NTF,DWORD);
void	LuaProto_Set::NetProcess::OnProcessSerMsg(DWORD & rec)
{
	DWORD nVersion = rec;
	nVersion = rec;
}
//重新装载脚本请求 RanaClient->RanaService
void LuaProto_Set::NetProcess::TS_ReloadSpace_Req( const Rana_SpaceName& name, const Rana_ScriptPath& filepath )
{	
	RanaProto::P_TS_ReloadSpace_Req request;
	request.name = name;
	request.filepath = filepath;
	SEND_MSG( RanaProto::TS_RELOADSPACE_REQ, request );
}
//重新装载脚本响应 RanaService->RanaClient
SESSION_PACKET(LuaProto_Set, ST_RELOADSPACE_ACK,P_ST_ReloadSpace_Ack);
void	LuaProto_Set::NetProcess::OnProcessSerMsg( P_ST_ReloadSpace_Ack& rec )
{
	if( rec.code == ERROR_OK )
	{
	}
	else
	{
		//ERROR_RANA_ACCESS
		//ERROR_RANA_SPACE
	}
}
//管理员登录请求 RanaClient->RanaService
void LuaProto_Set::NetProcess::TS_Login_Req( const Rana_UserName& name, const Rana_PassWord& password )
{
	P_TS_Login_Req request;
	request.username = name;
	request.password = password;
	SEND_MSG( RanaProto::TS_LOGIN_REQ, request );
}

//管理员登录响应 RanaService->RanaClient
SESSION_PACKET(LuaProto_Set, ST_LOGIN_ACK,P_ST_Login_Ack);
void	LuaProto_Set::NetProcess::OnProcessSerMsg( P_ST_Login_Ack& rec )
{
	if( rec.code == ERROR_OK )
	{
	}
	else
	{
	}
}


