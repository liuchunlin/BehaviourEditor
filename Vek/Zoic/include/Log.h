/*! @file
	@ingroup buildin
	@brief 定义日志类
*/

#ifndef __ZOIC_LOG_H
#define __ZOIC_LOG_H

#include <stdarg.h>
#include <Vek/Base/Module.h>
#include "Types.h"

class ModuleInit;

namespace Zoic
{
	/*! @brief 日志类
		@ingroup buildin
	*/
	class Log
#ifndef ZOIC_NO_SERVICE
		:public Vek::Module<Log>
#else
		:public NoCopy
#endif
	{
#ifndef ZOIC_NO_SERVICE
		friend class Vek::Singleton<Log>;
		friend class ModuleInit;
#endif
	public :
		//!记录日志到默认文件
		void log_f(
			const char *msg,	//!< 格式控制
			...);


		//!记录日志到特定文件
		void flog_f(
			const char *prefix,	//!< 文件前缀
			const char *suffix,	//!< 文件后缀
			const char *msg,	//!< 格式控制
			...);

		//!等同于同时执行log_f和flog_f
		void flogex_f(
			const char *prefix,	//!< 文件前缀
			const char *suffix,	//!< 文件后缀
			const char *msg,	//!< 格式控制
			...);

		const char* getPrefix() const;
		const char* getSuffix() const;
		//!设置默认日志文件前后缀
		void setLogFix(
			const char *prefix,			//!< 文件前缀
			const char *suffix=".log"	//!< 文件后缀
			);

		//! 记录日志
		void _flog(
			const char *prefix,	//!< 文件前缀
			const char *suffix,	//!< 文件后缀
			const char *msg,	//!< 格式控制
			va_list args,
			bool doput
			);

		//! 记录日志
		void _fplog(
			size_t fileCount,
			FILE ** files,
			const char *msg,	//!< 格式控制
			va_list args,
			bool doput
			);

		FILE* _fopen(
			const char *prefix,	//!< 文件前缀
			const char *suffix	//!< 文件后缀
			);
#ifndef ZOIC_NO_SERVICE
	private:
#else
	public:
#endif
		Log();

		//!更新时间
		void updateTime();
		const char *m_prefix;		//!< 日志前缀
		const char *m_suffix;		//!< 日志后缀
		TIME_SEC	m_lasttime;		//!< 上一次调用flog与log的时间
		char 		m_timebuf[20];	//!< 时间字符串值
		char		m_datebuf[20];	//!< 日期字符串值
	};

#ifdef ZOIC_NO_SERVICE
	extern Log theLog;
#endif
}

namespace Zoic
{
	inline const char* Log::getPrefix() const
	{
		return m_prefix;
	}

	inline const char* Log::getSuffix() const
	{
		return m_suffix;
	}
}

#ifndef ZOIC_NO_SERVICE
	#define log_f Zoic::Log::GetInstance()->log_f
	#define flog_f Zoic::Log::GetInstance()->flog_f
	#define flogex_f Zoic::Log::GetInstance()->flogexf
	#define g_Log Zoic::Log::GetInstance()
#else
	#define log_f (Zoic::theLog.log_f)
	#define flog_f (Zoic::theLog.flog_f)
	#define flogex_f (Zoic::theLog.flogexf)
	#define g_Log (&Zoic::theLog)
#endif

#ifdef _DEBUG
	#define dlog_f log_f
	#define dflog_f flog_f
	#define dflogex_f flogexf
#else
	inline void dlog_f(const char *msg,...){}
	inline void dflog_f(const char *prefix,const char *suffix,const char *msg,...){}
	inline void dflogex_f(const char *ext, const char *msg,...){}
#endif

#endif
