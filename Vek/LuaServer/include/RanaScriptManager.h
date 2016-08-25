#ifndef __RANASCRIPTMANAGER_H__
#define __RANASCRIPTMANAGER_H__

#include <Vek/Base/Module.h>
#include <Zoic/include/String.h>
#include <map>
#include <set>

class RanaScriptManager: public Vek::Module<RanaScriptManager>
{
	friend class Vek::Singleton<RanaScriptManager>;
public:
	struct RanaScriptType
	{
		BYTE spaceType;
		Zoic::String<100> path;
		Zoic::String<50> scriptName;
	};
	struct CmpPScriptType
	{
		bool operator () (const RanaScriptType* a, const RanaScriptType* b) const;
	};
	typedef std::set<RanaScriptType*,CmpPScriptType> PATH_SET;
	typedef std::set<RanaScriptType*> SCRIPT_SET;
	typedef std::map<BYTE, SCRIPT_SET > TYPE_MAP;

	void setConfigPath(const char* path);
	bool setScriptInfo( const char* path, const char* name, BYTE type);
	bool removeScriptInfo( const char* path );
	const PATH_SET & getPaths();
	const SCRIPT_SET* getPaths(BYTE type);
	const RanaScriptType* findItem(const char* path);
private:
	RanaScriptManager();
	~RanaScriptManager();
	int start();
	int stop();
private:
	void load(const char *file);
	void save(const char *file);

	RanaScriptType* find(const char* path);
	//!载入脚本条目
	void loadScriptItem(
		char *buf			//!<脚本文件定义
		);

	PATH_SET	m_filePaths;
	TYPE_MAP	m_fileTypes;
	bool m_changed;
	Zoic::String<100> m_configPath;
};

inline bool RanaScriptManager::CmpPScriptType::operator () (const RanaScriptType* a, const RanaScriptType* b) const
{
	return strcmp( a->path , b->path ) < 0;
}

inline const RanaScriptManager::PATH_SET & RanaScriptManager::getPaths()
{
	return m_filePaths;
}

#define g_RanaScriptManager RanaScriptManager::GetInstance()

#endif

