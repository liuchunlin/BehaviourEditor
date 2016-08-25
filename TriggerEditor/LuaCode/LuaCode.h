#pragma once

#include "Vek/Base/Singleton.h"
#include "Vek/Base/Types.h"
#include <tchar.h>
#include <string>
#include <set>
using namespace std;
namespace Space { class SNode; }

class GenCode: public Vek::Singleton<GenCode>
{
	friend class Vek::Singleton<GenCode>;
public:
	bool		Gen();
	void		SetGenerateLuaPath( const TCHAR* path );
	bool		CompileLua();
private:
	GenCode();
	bool		GenLuaCode( );
	void		GenLuaCodeRecursively(Space::SNode* pRoot, string& eventCode, string& triggerCode, string& functionCode, set<DWORD>& triggerIds, set<wstring>& functionNames);

	string		m_GenLuaPath;
};

extern GenCode*	GGenCode;