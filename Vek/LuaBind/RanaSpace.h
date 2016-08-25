#ifndef __RANA_SPACE_H__
#define __RANA_SPACE_H__

#include <map>
#include <stack>
#include <list>
#include <string>
using namespace std;
#include "RanaTriggerBank.h"
#include "RanaEventBank.h"

class RanaContext;
class RanaSpace
{
	friend class RanaSpaceBank;
public:
	typedef RanaCOUNT (*RANA_VALUE_FUNCTION)(RanaState L,void* context);//返回值:实际进行了操作的值的个数
	class RanaValues
	{
	public:
		inline RanaCOUNT pushInputs(RanaState L) const
		{
			return push_datas(L);
		}
		inline RanaCOUNT getOutputs(RanaState L)
		{
			return get_datas(L);
		}
		virtual ~RanaValues(){};
	protected:
		virtual RanaCOUNT push_datas(RanaState L) const
		{
			RANA_ASSERT(0);
			return 0;
		}
		virtual RanaCOUNT get_datas(RanaState L)
		{
			RANA_ASSERT(0);
			return 0;
		}
	};
	enum EXCEPTION_CODE
	{
		EC_SUCCESS,
		EC_ADMINISTRATOR,
		EC_RUNFUCTION,
		EC_RUNBUFFER,
	};
	enum EXCEPTION_METHOD
	{
		EM_DELSPACE,
		EM_CUSTOM,
	};
	struct CFunctionItem
	{
		string name;
		int	(*func)(RanaState);
	};
	class CFunctionItemInit
	{
	public:
		CFunctionItemInit(unsigned char type,const char *name,int	(*func)(RanaState));
	};
	typedef multimap<unsigned char,CFunctionItem> FUNC_MAP;
	static FUNC_MAP & getFunctionMap();
	class CFunctionRegister
	{
	public:
		CFunctionRegister(RanaState _S)
		{
			m_state = _S;
		}

		void registerRanaCallBack(char *funcname, int(*callback)(RanaState _S))
		{
			lua_register(m_state, funcname, callback);
		}

	private:
		RanaState m_state;
	};

	enum ErrorCode
	{
		RESULT_OK=0,
		EMPTYFILENAME,
		ERROR_NO_SUCH_FUNCTION,
		ERROR_WRONG_RETURN_TYPE,
		ERROR_WRONG_PARAM_TYPE,
		ERROR_NOTEXISTFILE,
		ERROR_STATE,
		ERROR_MAINFUNCTION,
	};
	enum Space_Status
	{
		SS_NONE,
		SS_INIT,
		SS_RUNING,
		SS_PENDING,		//Delete or Reload
		SS_TERMINATE,
	};
	class CmpStr
	{
	public:
		bool operator()(const char *a,const char *b) const
		{
			return strcmp(a,b) < 0;
		}
	};
	typedef map<const char*,RanaVariant,CmpStr> VARIANT_MAP;

	RanaSpace();
	~RanaSpace();
	int					initialEmptySpace(RanaSpaceName rsn,unsigned char type = 0,RanaContext * context = NULL);
	int					initial(const char* fname,RanaSpaceName rsn,unsigned char type = 0,RanaContext * context = NULL);
	void				clear();
	bool				checkLuaState(RanaState s);		//用来检查lua_State是否和成员的lua_State相同

	int					runFunctionInNewThread( const char* FnName, RanaMessage* msg );
	//////////////////////////////////////////////////////////////////////////
	int					runFunction(const char* funcName,int argc,RanaVariant *argv);
	int					runFunction(const char* funcName,int argc,RanaVariant *argv,bool* res);
	int					runFunction(const char* funcName,int argc,RanaVariant *argv,double* res);
	int					runFunction(const char* funcName,int argc,RanaVariant *argv,char* res, int len=255);
	int					runFunction(const char* funcName,int argc,RanaVariant *argv,RanaVariant& resv);
	int					runFunction(const char* funcName,int argc,RanaVariant *argv,RanaValues& res);
	int					runFunction(const char* funcName,const RanaValues& args);
	int					runFunction(const char* funcName,const RanaValues& args,bool* res);
	int					runFunction(const char* funcName,const RanaValues& args,double* res);
	int					runFunction(const char* funcName,const RanaValues& args,char* res, int len=255);
	int					runFunction(const char* funcName,const RanaValues& args,RanaValues& res);
	int					runFunction(const char* funcName,const RanaValues& args,RanaVariant& resv);
	int					runFunction(const char* funcName);
	int					runFunction(const char* funcName,bool* res);
	int					runFunction(const char* funcName,double* res);
	int					runFunction(const char* funcName,char* res, int len=255);
	int					runCode(const char* code);
	int					runFile(const char* fname);
	//void				setContext(RanaContext *context);
	//const				RanaContext* getContext();
	void				releaseValueTable(); // 释放Lua上下文环境空间

	bool					getBoolean(const char* key);
	const char*				getString(const char* key);
	int						getInteger(const char* key);
	void*					getUserData(const char* key);
	void					setBoolean(const char *key,bool value);
	void					setNumber(const char *key,int value);
	void					setString(const char *key,const char * value);
	void					setUserData(const char *key,void * value);
	const RanaVariant*		getContext(const char *key);

	const RanaSpaceHandle&	getHandle();
	unsigned long			getSerial();
	RanaSpaceName			getSpaceName();
	const char*				getFileName();
	unsigned char			getSpaceType();
	RanaTriggerBank*		getTriggerBank();
	RanaEventBank*			getEventBank();
	RanaState				GetLuaState();
	int						GetThreadIndex( lua_State* T );
	bool					ThreadBreak( lua_State* T, bool bCheckStatus );
	Space_Status			Status() const 
	{ 
		return m_Status; 
	}
	void					Status(Space_Status val) 
	{ 
		m_Status = val; 
	}

	int						runBuf(const char* script);
	int						runBuf(const char* script, unsigned long nLength);
	int						runBuf(const char* script, char* res, int len);
	int						runBuf(const char* script, bool* res);


	RanaContext*			getContext();
	void					setContext(RanaContext* pContext);
	int						registerEvent(RanaEvent *_event);	// 注册一个事件/触发器
	bool					onMessage(RanaMessage *_msg);		// 触发事件
private:
	int						initialFromFileForEmptySpace(const char* fname);
	void					registerCFunction();
private:
	RanaState					m_state;
	map<lua_State*, int>		m_Threads;

	list<VARIANT_MAP*>			m_unusedVMaps;
	VARIANT_MAP*				m_puttingVariantMap;
	stack<VARIANT_MAP*>			m_VMapStack;

	RanaSpaceName				m_SpaceName;
	unsigned char				m_SpaceType;
	RanaSpaceHandle				m_spaceHandle;
	string						m_FileName;
	RanaContext*				m_context;

	RanaTriggerBank*			m_pTriggerBank;
	RanaEventBank*				m_pEventBank;

	EXCEPTION_CODE				m_errorCode;
	list<RanaSpace*>::iterator m_itErrorList;
	Space_Status				m_Status;

};


inline const RanaSpaceHandle& RanaSpace::getHandle()
{
	return m_spaceHandle;
}

inline RanaSpaceName RanaSpace::getSpaceName()
{
	return m_SpaceName;
}

inline unsigned char RanaSpace::getSpaceType()
{
	return m_SpaceType;
}

inline RanaTriggerBank* RanaSpace::getTriggerBank()
{
	return m_pTriggerBank;
}

inline RanaEventBank* RanaSpace::getEventBank()
{
	return m_pEventBank;
}
inline RanaState	RanaSpace::GetLuaState()
{
	return m_state; 
}

inline RanaSpace::FUNC_MAP & RanaSpace::getFunctionMap()
{
	static FUNC_MAP funcMap;
	return funcMap;
}

inline RanaSpace::CFunctionItemInit::CFunctionItemInit(unsigned char type,const char *name,int (*func)(RanaState))
{
	CFunctionItem funcItem;
	funcItem.name = name;
	funcItem.func = func;
	getFunctionMap().insert( pair<unsigned char,CFunctionItem>(type,funcItem) );
};

inline RanaContext* RanaSpace::getContext()
{
	return m_context;
}

inline void RanaSpace::setContext(RanaContext* pContext)
{
	m_context = pContext;
}

inline int RanaSpace::registerEvent(RanaEvent *_event)
{
	return getEventBank()->registerEvent(_event);
}

inline int RanaSpace::runFunction(const char* funcName)
{
	return runFunction(funcName,0,static_cast<RanaVariant *>(NULL));
}

inline int RanaSpace::runFunction(const char* funcName,bool* res)
{
	return runFunction(funcName,0,NULL,res);
}

inline int RanaSpace::runFunction(const char* funcName,double* res)
{
	return runFunction(funcName,0,NULL,res);
}

inline int RanaSpace::runFunction(const char* funcName,char* res, int len)
{
	return runFunction(funcName,0,NULL,res,len);
}

inline RanaSpace * GetRanaSpace(RanaState state)
{

	void * space;
	lua_getallocf(state,&space);
#ifdef _DEBUG
	RanaSpace * pRanaSpace = static_cast<RanaSpace *>(space);
	RANA_ASSERT( pRanaSpace != NULL );
#endif
	return static_cast<RanaSpace *>(space);
}

#define REG_RANA_FUNC_NAME(TYPE,NAME,FUNC) \
	extern int l_##FUNC(RanaState); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_##NAME(TYPE,#NAME,l_##FUNC);

#define REG_RANA_FUNC(TYPE,NAME) \
	extern int l_##NAME(RanaState); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_##NAME(TYPE,#NAME,l_##NAME);

#define REG_RANA_TIMER(TYPE) \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_RegisterTimeEvent(TYPE,"RanaRegisterTimeEvent",ranaRegisterTimeEvent); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_CreateTimer(TYPE,"RanaCreateTimer",ranaCreateTimer); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetTimerPeriodTime(TYPE,"RanaSetTimerPeriodTime",ranaSetTimerPeriodTime); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetTimerRepeat(TYPE,"RanaSetTimerRepeat",ranaSetTimerRepeat); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetTimerOnOff(TYPE,"RanaSetTimerOnOff",ranaSetTimerOnOff); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetResetTimer(TYPE,"RanaResetTimer",ranaResetTimer);

#define REG_RANA_EVENT(TYPE) \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetSpaceEventOnOff(TYPE,"RanaSetSpaceEventOnOff",ranaSetSpaceEventOnOff); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetSetTypeEventOnOff(TYPE,"RanaSetTypeEventOnOff",ranaSetTypeEventOnOff); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetTriggerEventOnOff(TYPE,"RanaSetTriggerEventOnOff",ranaSetTriggerEventOnOff); \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetEventOnOff(TYPE,"RanaSetEventOnOff",ranaSetEventOnOff);

#define REG_RANA_TRIGGER(TYPE) \
	static RanaSpace::CFunctionItemInit _init_type##TYPE##_SetTriggerOnOff(TYPE,"RanaSetTriggerOnOff",ranaSetTriggerOnOff);

#define REG_RANA_CONTEXT(TYPE) \
	REG_RANA_FUNC(TYPE,getContextBool) \
	REG_RANA_FUNC(TYPE,getContextString) \
	REG_RANA_FUNC(TYPE,getContextNumber) \
	REG_RANA_FUNC(TYPE,getContext)

#define REG_RANA_MYSQL(TYPE) \
	REG_RANA_FUNC(TYPE,mysql_query)

#endif //__RANA_SPACE_H__

