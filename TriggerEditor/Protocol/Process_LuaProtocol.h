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
public://Э��
	struct NetProcess 
	{
		//����Ա��¼���� RanaClient->RanaService
		static void TS_Login_Req( const Rana_UserName& name, const Rana_PassWord& password );

		//����Ա��¼��Ӧ RanaService->RanaClient
		static void OnProcessSerMsg( P_ST_Login_Ack& rec );

		//����װ�ؽű����� RanaClient->RanaService
		static void TS_ReloadSpace_Req( const Rana_SpaceName& name, const Rana_ScriptPath& filepath );

		//����װ�ؽű���Ӧ RanaService->RanaClient
		static void OnProcessSerMsg( P_ST_ReloadSpace_Ack& rec );

		//�汾���ж�
		static void	OnProcessSerMsg(DWORD & rec);
	};
};
extern LuaProto_Set*	GLuaProto;

