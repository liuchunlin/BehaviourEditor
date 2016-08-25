#pragma once

#include "Vek/Base/Singleton.h"
#include <tchar.h>
#include <string>
using namespace std;

class GenCode: public Vek::Singleton<GenCode>
{
	friend class Vek::Singleton<GenCode>;
public:
	bool		Gen();
	void		SetGenerateCPPPath( const TCHAR* path );
private:
	bool		RanaEventImpl( );
	bool		RanaEventType( );
	bool		RanaMsgImpl( );
	bool		RanaRegisterEvent( );
	bool		RanaSpaceType( );
	bool		RanaType( );
	bool		RanaTask();

	bool		RanaEventImplCS( );
	bool		RanaEventTypeCS( );
	bool		RanaMsgImplCS( );
	bool		RanaRegisterEventCS( );
	bool		RanaSpaceTypeCS( );
	bool		RanaTaskCS();

	GenCode();

	string		m_GenCPPPath;
};

extern GenCode*	GGenCode;