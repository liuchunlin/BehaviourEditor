/*! @file
	@ingroup buildin
	@brief ������־��
*/

#ifndef __ZOIC_LOG_H
#define __ZOIC_LOG_H

#include <stdarg.h>
#include <Vek/Base/Module.h>
#include "Types.h"

class ModuleInit;

namespace Zoic
{
	/*! @brief ��־��
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
		//!��¼��־��Ĭ���ļ�
		void log_f(
			const char *msg,	//!< ��ʽ����
			...);


		//!��¼��־���ض��ļ�
		void flog_f(
			const char *prefix,	//!< �ļ�ǰ׺
			const char *suffix,	//!< �ļ���׺
			const char *msg,	//!< ��ʽ����
			...);

		//!��ͬ��ͬʱִ��log_f��flog_f
		void flogex_f(
			const char *prefix,	//!< �ļ�ǰ׺
			const char *suffix,	//!< �ļ���׺
			const char *msg,	//!< ��ʽ����
			...);

		const char* getPrefix() const;
		const char* getSuffix() const;
		//!����Ĭ����־�ļ�ǰ��׺
		void setLogFix(
			const char *prefix,			//!< �ļ�ǰ׺
			const char *suffix=".log"	//!< �ļ���׺
			);

		//! ��¼��־
		void _flog(
			const char *prefix,	//!< �ļ�ǰ׺
			const char *suffix,	//!< �ļ���׺
			const char *msg,	//!< ��ʽ����
			va_list args,
			bool doput
			);

		//! ��¼��־
		void _fplog(
			size_t fileCount,
			FILE ** files,
			const char *msg,	//!< ��ʽ����
			va_list args,
			bool doput
			);

		FILE* _fopen(
			const char *prefix,	//!< �ļ�ǰ׺
			const char *suffix	//!< �ļ���׺
			);
#ifndef ZOIC_NO_SERVICE
	private:
#else
	public:
#endif
		Log();

		//!����ʱ��
		void updateTime();
		const char *m_prefix;		//!< ��־ǰ׺
		const char *m_suffix;		//!< ��־��׺
		TIME_SEC	m_lasttime;		//!< ��һ�ε���flog��log��ʱ��
		char 		m_timebuf[20];	//!< ʱ���ַ���ֵ
		char		m_datebuf[20];	//!< �����ַ���ֵ
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
