#pragma once

#include <Zoic/include/Types.h>
#include <Vek/Base/Singleton.h>
#include <string>
using namespace std;

#include <LuaServer/protocol/RanaProto_Rana.h>
using namespace RanaProto;

class LuaProto_Set:  public Vek::Singleton<LuaProto_Set>
{
	friend class Vek::Singleton<LuaProto_Set>;
public:
	void		Connect();
	void		DisConnect();
	void		GetIpPort_( wstring& locIp, int& locPort, wstring& serverIp, int& serverPort );
	void		UpdateConnectState();
private:
	LuaProto_Set();
	~LuaProto_Set();
public://协议
	struct NetProcess 
	{
		//管理员登录请求 RanaClient->RanaService
		static void TS_Login_Req( const Rana_UserName& name, const Rana_PassWord& password );

		//管理员登录响应 RanaService->RanaClient
		static void OnProcessSerMsg( P_ST_Login_Ack& rec );

		//重新装载脚本请求 RanaClient->RanaService
		static void TS_ReloadSpace_Req( const Rana_SpaceName& name, const Rana_ScriptPath& filepath );

		//重新装载脚本响应 RanaService->RanaClient
		static void OnProcessSerMsg( P_ST_ReloadSpace_Ack& rec );

		//版本号判断
		static void	OnProcessSerMsg(DWORD & rec);
	};
};
extern LuaProto_Set*	GLuaProto;

