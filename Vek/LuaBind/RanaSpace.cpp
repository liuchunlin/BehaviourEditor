#include "RanaSpace.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "RanaSpaceBank.h"
#include "RanaPush.h"

#ifdef _DEBUG
class _RanaStackCheck
{
public:
	_RanaStackCheck(RanaState L)
		:m_state(L)
	{
		m_orgStack = lua_gettop(m_state);
	}
	~_RanaStackCheck()
	{
		const int currentStack = lua_gettop(m_state);
		for( int i = currentStack;i>m_orgStack;--i)
		{
			ranaError( "lua stack leak:%d(%d)\n", i,lua_type(m_state,i) );
		}
		RANA_ASSERT( currentStack == m_orgStack);
	}
private:
	RanaState	m_state;
	int			m_orgStack;
};
//#define DEBUG_CHECK_STACK(L) _RanaStackCheck _ranaCheckingStack(L);
#define DEBUG_CHECK_STACK(L)
#else
#define DEBUG_CHECK_STACK(L)
#endif

RanaSpace::RanaSpace()
{
	m_state = NULL;
	m_errorCode = EC_SUCCESS;
	m_context = NULL;
	m_pTriggerBank		= new RanaTriggerBank(this);
	m_pEventBank		= new RanaEventBank(this);
	m_puttingVariantMap = new VARIANT_MAP;

	Status( SS_NONE );
}

RanaSpace::~RanaSpace()
{
	while( m_VMapStack.size() )
	{
		delete m_VMapStack.top();
		m_VMapStack.pop();
	}
	if(m_puttingVariantMap)
	{
		delete m_puttingVariantMap;
		m_puttingVariantMap = NULL;
	}
	list<VARIANT_MAP*>::iterator it = m_unusedVMaps.begin();
	for(;it!=m_unusedVMaps.end();++it )
	{
		delete (*it);
	}

	delete m_pEventBank;
	m_pEventBank = NULL;
	delete m_pTriggerBank;
	m_pTriggerBank = NULL;
	delete m_puttingVariantMap;
	m_puttingVariantMap = NULL;
}

int RanaSpace::runFile(const char* fname)
{
	DEBUG_CHECK_STACK(m_state);
	if(!fname || fname[0]=='\0')
	{
		return EMPTYFILENAME;
	}
	char do_buf[2048];
	if( snprintf(do_buf,sizeof(do_buf),"dofile(\"%s\");",fname)>0 )
	{
		int result = luaL_dostring(m_state,do_buf);
		if( result != RANA_OK )
		{
			ranaError( "cannot run file(%s): %s\n", fname, lua_tostring(m_state, -1) );
			lua_pop(m_state, 1);
			return result;
		}
	}
	else
	{
		ranaError("Filename(%s) is too long. It shouldn't lengther than 2000.\n",fname);
		return ERROR_NOTEXISTFILE;
	}
	return RANA_OK;
}

int RanaSpace::initialEmptySpace(RanaSpaceName rsn,unsigned char type,RanaContext * context)
{
	m_SpaceType = type;
	m_SpaceName = rsn;

	m_state = lua_open();   /* opens Lua */

	//将当前RanaSpace的指针注册到相应的ranastate的空间中
	lua_setallocf( m_state,	lua_getallocf(m_state,NULL), this );
	luaL_openlibs( m_state );
	registerCFunction();

	//set space internal name
	m_FileName = "";
	m_context = context;
	m_errorCode = EC_SUCCESS;
	return RESULT_OK;
}

int RanaSpace::initialFromFileForEmptySpace(const char* fname)
{
	if(!m_state)
	{
		return ERROR_STATE;
	}
	if(!fname || fname[0] == '\0' )
	{
		return EMPTYFILENAME;
	}
#if 1
	{
		int error = luaL_loadfile( m_state, fname );
		if( error != 0 )
		{
			ranaError( "load script [%s] failed\n", lua_tostring(m_state, -1) );
			return ERROR_NOTEXISTFILE;
		}
		lua_resume( m_state, 0 );
	}
#else
	char do_buf[2048];
	if(snprintf(do_buf,sizeof(do_buf),"dofile(\"%s\");",fname)>0)
	{
		if(luaL_dostring(m_state,do_buf) != 0)	//if (luaL_loadfile(m_state, fname) || lua_pcall(m_state, 0, 0, 0))
		{
			ranaError("cannot run configuration file: %s\n",lua_tostring(m_state, -1));
			lua_pop(m_state,1);
			return ERROR_NOTEXISTFILE;
		}
	}
	else
	{
		ranaError("Filename(%s) is too long. It shouldn't lengther than 2000.\n",fname);
		return ERROR_NOTEXISTFILE;
	}
#endif

	lua_getglobal(m_state, "main");
	if (lua_pcall(m_state, 0, 0, 0) != 0)
	{
		ranaError("error running function 'main': %s\n",lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_MAINFUNCTION;
	}

	//set space internal name
	m_FileName = fname;
	return RESULT_OK;
}


int RanaSpace::initial(const char* fname,RanaSpaceName rsn,unsigned char type,RanaContext * context)
{
	Status( SS_INIT );
	int retCode = initialEmptySpace(rsn,type,context);
	if( retCode != RESULT_OK )
	{
		return retCode;
	}
	retCode = initialFromFileForEmptySpace(fname);
	if( retCode != RESULT_OK )
	{
		clear();
		return retCode;
	}
	Status( SS_RUNING );
	return RESULT_OK;
}

void RanaSpace::clear()
{
	if(m_state)
	{
		lua_close(m_state);
		m_state = NULL;
	}
	Status( SS_TERMINATE );
}


void RanaSpace::registerCFunction()
{
	FUNC_MAP &funcMap = getFunctionMap();
	pair<FUNC_MAP::iterator, FUNC_MAP::iterator> funcPair = funcMap.equal_range(m_SpaceType);
	FUNC_MAP::iterator it;
	for(it = funcPair.first; it!=funcPair.second;it++ )
	{
		CFunctionItem *pCfuncItem = &(it->second);
		lua_register(m_state, pCfuncItem->name.c_str(), pCfuncItem->func );
	}
/*	// 向Rana脚本中注册C API
	lua_register(m_state, "RanaRegisterTimeEvent", ranaRegisterTimeEvent);
	lua_register(m_state, "RanaCreateTimer", ranaCreateTimer);
	lua_register(m_state, "RanaSetTimerPeriodTime", ranaSetTimerPeriodTime);
	lua_register(m_state, "RanaSetTimerRepeat", ranaSetTimerRepeat);
	lua_register(m_state, "RanaSetTimerOnOff", ranaSetTimerOnOff);
	lua_register(m_state, "RanaResetTimer", ranaResetTimer);
	lua_register(m_state, "RanaSetSpaceEventOnOff", ranaSetSpaceEventOnOff);
	lua_register(m_state, "RanaSetTypeEventOnOff", ranaSetTypeEventOnOff);
	lua_register(m_state, "RanaSetTriggerEventOnOff", ranaSetTriggerEventOnOff);
*/
}

bool RanaSpace::checkLuaState(RanaState _S)
{
	return (m_state == _S);
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 0, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(), EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n", funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}

	return RESULT_OK;
}

int	RanaSpace::GetThreadIndex( lua_State* T )
{
	if( T == NULL )
		return 0;

	for( int i = 1; i <= lua_gettop(m_state); ++i )
	{
		int type = lua_type(m_state,i);
		switch(type)
		{
		case LUA_TTHREAD:
			{
				lua_State *L = (lua_State *)( lua_topointer( m_state, i ) );
				if( T == L )
					return i;
			}
			break;
		}
	}
	return 0;
}
bool	RanaSpace::ThreadBreak( lua_State* T, bool bCheckStatus )
{
	if( NULL == T )
		return false;
	if( NULL == m_state )
		return false;
	if( !bCheckStatus || lua_status( T ) != LUA_YIELD )
	{
		int iThreadIndex = GetThreadIndex(T);
		if( iThreadIndex != 0 )
		{
			lua_remove( m_state, iThreadIndex ); 
		}
	}
	return true;
}

int	RanaSpace::runFunctionInNewThread( const char* FnName, RanaMessage* msg )
{
	lua_State *T = lua_newthread(m_state);
	int Index_T = lua_gettop(m_state);
	lua_setallocf( T, lua_getallocf(T,NULL), this );
	lua_resume( T, 0 );

	lua_getglobal( T, FnName );
	int nargs = (msg ? (int)msg->GetContexts().size() : 0);
	if (nargs > 0)
	{
		const std::vector<RanaVariant>& contextParams = msg->GetContexts();
		for(int index = 0; index < nargs; ++index)
		{
			rana_push(T, contextParams[index]);
		}
	}
	if (lua_pcall( T, nargs, 0, 0) != 0)
	{
		ranaError("error running function '%s': %s\n", FnName, lua_tostring( T, -1));
		lua_pop( T, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}

	ThreadBreak( T, true );
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv, bool* res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(), EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isboolean(m_state, -1))
	{
        ranaError("function %s must return a boolean\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	*res = lua_toboolean(m_state, -1)==0?false:true;
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv, double* res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isnumber(m_state, -1))
	{
		ranaError("function %s must return a number\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	*res = lua_tonumber(m_state, -1);
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv, char* res, int len)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isstring(m_state, -1))
	{
		ranaError("function %s must return a string\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	strncpy(res, lua_tostring(m_state, -1),len);
	res[len-1]=0;
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv,RanaVariant& resv)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	switch( lua_type(m_state,-1) )
	{
	case LUA_TNIL:
		resv.clear();
		break;
	case LUA_TNUMBER:
		resv = static_cast<int>( lua_tonumber(m_state, -1) );
		break;
	case LUA_TSTRING:
		resv = lua_tostring(m_state, -1);
		break;
	case LUA_TBOOLEAN:
		resv = lua_toboolean(m_state, -1);
		break;
	default:
		ranaError("function %s must return simple type\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,int argc,RanaVariant *argv,RanaValues& res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	for(int i=0;i<argc;i++)
	{
		rana_push(m_state,*argv++);
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	argc = res.getOutputs(m_state);
	if(argc < 0)
	{
		ranaError("function %s return data failed");
		return ERROR_WRONG_RETURN_TYPE;
	}
	for(;argc>0;--argc)
	{
		lua_pop(m_state, 1);
	}
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaSpace::RanaValues& args)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 0, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaSpace::RanaValues& args,bool* res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isboolean(m_state, -1))
	{
		ranaError("function %s must return a bool\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	*res = lua_toboolean(m_state, -1)==0?false:true;
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaSpace::RanaValues& args,double* res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isnumber(m_state, -1))
	{
		ranaError("function %s must return a number\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	*res = lua_tonumber(m_state, -1);
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaSpace::RanaValues& args,char* res, int len)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	if (!lua_isstring(m_state, -1))
	{
		ranaError("function %s must return a string\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	strncpy(res, lua_tostring(m_state, -1),len);
	res[len-1]=0;
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaValues& args,RanaValues& res)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	argc = res.getOutputs(m_state);
	if(argc < 0)
	{
		ranaError("function %s return data failed");
		return ERROR_WRONG_RETURN_TYPE;
	}
	for(;argc>0;--argc)
	{
		lua_pop(m_state, 1);
	}
	return RESULT_OK;
}

int RanaSpace::runFunction(const char* funcName,const RanaValues& args,RanaVariant& resv)
{
	DEBUG_CHECK_STACK(m_state);
	lua_getglobal(m_state, funcName);
	int argc = args.pushInputs(m_state);
	if(argc < 0)
	{
		return ERROR_WRONG_PARAM_TYPE;
	}
	if (lua_pcall(m_state, argc, 1, 0) != 0)
	{
		RanaSpaceBank::getInstance()->setSpaceExit(getHandle(),EC_RUNFUCTION);
		ranaError("error running function '%s': %s\n",funcName,lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
		return ERROR_NO_SUCH_FUNCTION;
	}
	switch( lua_type(m_state,-1) )
	{
	case LUA_TNIL:
		resv.clear();
		break;
	case LUA_TNUMBER:
		resv = static_cast<int>( lua_tonumber(m_state, -1) );
		break;
	case LUA_TSTRING:
		resv = lua_tostring(m_state, -1);
		break;
	case LUA_TBOOLEAN:
		resv = lua_toboolean(m_state, -1);
		break;
	default:
		ranaError("function %s must return simple type\n",funcName);
		return ERROR_WRONG_RETURN_TYPE;
	}
	lua_pop(m_state, 1);  /* pop returned value */
	return RESULT_OK;
}

void RanaSpace::releaseValueTable()
{
}
void RanaSpace::setBoolean(const char* key ,bool value)
{
	(*m_puttingVariantMap)[key] = value;
}

void RanaSpace::setString(const char* key ,const char* value)
{
	(*m_puttingVariantMap)[key] = value;
}

void RanaSpace::setNumber(const char* key ,int value)
{
	(*m_puttingVariantMap)[key] = value;
}

void RanaSpace::setUserData(const char* key ,void * value)
{
	(*m_puttingVariantMap)[key] = value;
}

bool RanaSpace::getBoolean(const char* key)
{
	VARIANT_MAP& vMap = *( m_VMapStack.top() );
	VARIANT_MAP::iterator it = vMap.find(key);
	if(it != vMap.end())
	{
		if( it->second.getType() == RanaVariant::VT_BOOL )
		{
			return it->second;
		}
	}
	return false;
}

const char* RanaSpace::getString(const char* key)
{
	VARIANT_MAP& vMap = *( m_VMapStack.top() );
	VARIANT_MAP::iterator it = vMap.find(key);
	if(it != vMap.end())
	{
		if( it->second.getType() == RanaVariant::VT_STRING )
		{
			return it->second;
		}
	}
	return NULL;
}

int RanaSpace::getInteger (const char* key)
{
	VARIANT_MAP& vMap = *( m_VMapStack.top() );
	VARIANT_MAP::iterator it = vMap.find(key);
	if(it != vMap.end())
	{
		if( it->second.getType() == RanaVariant::VT_INTEGER )
		{
			return int(it->second);
		}
	}
	return 0;
}

void * RanaSpace::getUserData(const char* key)
{
	VARIANT_MAP& vMap = *( m_VMapStack.top() );
	VARIANT_MAP::iterator it = vMap.find(key);
	if(it != vMap.end())
	{
		if( it->second.getType() == RanaVariant::VT_USERDATA)
		{
			return it->second.body.vVal;
		}
	}
	return NULL;
}

const RanaVariant * RanaSpace::getContext(const char* key)
{
	VARIANT_MAP& vMap = *( m_VMapStack.top() );
	VARIANT_MAP::iterator it = vMap.find(key);
	if(it != vMap.end())
	{
		return &it->second;
	}
	return NULL;
}

const char* RanaSpace::getFileName()
{
	return m_FileName.c_str();
}
int RanaSpace::runBuf(const char* script)
{
	DEBUG_CHECK_STACK(m_state);
	int retVal = luaL_dobuffer(m_state,script,strlen(script),m_SpaceName);
	if ( retVal != RANA_OK )
	{
		ranaError("running script error: %s\n:\nScript content: %s\n",lua_tostring(m_state, -1),script);
		lua_pop(m_state, 1);
	}
	return retVal;
}

int RanaSpace::runBuf(const char* script, unsigned long nLength)
{
	DEBUG_CHECK_STACK(m_state);
	int retVal = luaL_dobuffer(m_state,script,nLength,m_SpaceName);
	if ( retVal != RANA_OK )
	{
		char* pBuf = new char[nLength+1];
		memcpy(pBuf,script,sizeof(char)*nLength);
		pBuf[nLength] = 0;
		ranaError("running script error: %s\n:\nScript content: %s\n",lua_tostring(m_state, -1),pBuf);
		lua_pop(m_state, 1);
		delete pBuf;
	}
	return retVal;
}

int RanaSpace::runBuf(const char* script, char* res, int len)
{
	DEBUG_CHECK_STACK(m_state);
	int retVal = luaL_dobuffer(m_state,script,strlen(script),m_SpaceName);
	if ( retVal != RANA_OK )
	{
		ranaError("running script error: %s\n:\nScript content: %s\n",lua_tostring(m_state, -1),script);
		lua_pop(m_state, 1);
		return retVal;
	}
	if (!lua_isstring(m_state, -1))
	{
		ranaError("script\n< %s >\n must return a string\n",script);
		return ERROR_WRONG_RETURN_TYPE;
	}
	strncpy(res, lua_tostring(m_state, -1),len);
	res[len-1]=0;
	lua_pop(m_state, 1);  /* pop returned value */
	return retVal;
}

int RanaSpace::runBuf(const char* script, bool* res)
{
	DEBUG_CHECK_STACK(m_state);
	int retVal = luaL_dobuffer(m_state,script,strlen(script),m_SpaceName);
	if ( retVal != RANA_OK )
	{
		ranaError("running script error: %s\n:\nScript content: %s\n",lua_tostring(m_state, -1),script);
		lua_pop(m_state, 1);
		return retVal;
	}
	if (!lua_isboolean(m_state, -1))
	{
		ranaError("script\n< %s >\n must return a bool\n",script);
		return ERROR_WRONG_RETURN_TYPE;
	}
	*res = lua_toboolean(m_state, -1)==0?false:true;
	lua_pop(m_state, 1);  /* pop returned value */
	return retVal;
}

bool RanaSpace::onMessage(RanaMessage *_msg)
{
	m_VMapStack.push(m_puttingVariantMap);
	if( !m_unusedVMaps.empty() )
	{
		m_puttingVariantMap = m_unusedVMaps.front();
		m_unusedVMaps.pop_front();
	}
	else
	{
		m_puttingVariantMap = new VARIANT_MAP;
	}
	bool bSuccess = getEventBank()->onMessage(_msg);
	m_puttingVariantMap->clear();
	m_unusedVMaps.push_back(m_puttingVariantMap);
	m_puttingVariantMap = m_VMapStack.top();
	m_VMapStack.pop();
	m_puttingVariantMap->clear();

	return bSuccess;
}
