#include "include/Log.h"
#include "include/Clock.h"
#include <stdarg.h>

using namespace Zoic;

#ifdef ZOIC_NO_SERVICE
	Log Zoic::theLog;
#endif

#undef log_f
#undef flog_f
#undef flogex_f
static const int TIMEBUF_LENGTH=12;
static const int LOGBUF_MAXLEN=1024*1024;
static char LOGBUF[LOGBUF_MAXLEN+1] = { 0 };

#ifdef _DEBUG	
const static bool s_doput = true;
#else
const static bool s_doput = false;
#endif


Log::Log():
	m_prefix("Server"),
	m_suffix(".log"),
	m_lasttime( 0 )
{
}

static inline FILE * open(const char *prefix,const char *suffix,const char *date)
{
	char fname[640];
	snprintf(fname,sizeof(fname),"%s%s%s",prefix,date,suffix);
	fname[sizeof(fname)-1]='\0';
	return fopen(fname, "a");
}

void Log::_flog(const char *prefix, const char *suffix, const char *msg, va_list args,bool doput)
{
	updateTime();
	FILE *fp = open(prefix, suffix, m_datebuf);
	if( fp )
	{
		_fplog(1,&fp,msg,args,doput);
		fclose(fp);
	}
	else
	{
		_fplog(0,NULL,msg,args,doput);
	}
}


void Log::_fplog(size_t fileCount, FILE ** files, const char *msg, va_list args,bool doput)
{
	updateTime();

	LOGBUF[TIMEBUF_LENGTH] = 0;
	strncpy(LOGBUF, m_timebuf, TIMEBUF_LENGTH);
	int len = vsnprintf(LOGBUF+TIMEBUF_LENGTH, LOGBUF_MAXLEN-TIMEBUF_LENGTH, msg, args);

	if(doput)
	{
		puts(LOGBUF);
	}

	if(!fileCount)
	{
		return;
	}
	char *tail = LOGBUF+TIMEBUF_LENGTH+len;
	*tail ++= '\n';
	*tail = 0;
	for(size_t i = 0; i< fileCount; ++i)
	{
		fputs(LOGBUF, files[i]);
	}
}


void Log::log_f(const char *msg,...)
{
	va_list args;
	va_start(args, msg);
	_flog(m_prefix, m_suffix, msg, args, s_doput);
	va_end(args);
}

void Log::flog_f(const char *prefix,const char *suffix,const char *msg,...)
{
	va_list args;
	va_start(args, msg);
	_flog(prefix, suffix, msg, args, s_doput);
	va_end(args);
}

void Log::flogex_f(const char *prefix,const char *suffix,const char *msg,...)
{
	updateTime();

	va_list args;
	va_start(args, msg);
	FILE* pfs[2];
	size_t index = 0;
	pfs[index] = open(m_prefix, m_suffix, m_datebuf);
	if(pfs[index])
	{
		++index;
	}
	pfs[index] = open(prefix, suffix, m_datebuf);
	if(pfs[index])
	{
		++index;
	}
	_fplog(index,pfs,msg,args,s_doput);
	for(size_t i = 0; i <index; ++i )
	{
		fclose(pfs[i]);
	}
}

void Log::setLogFix(const char *prefix,const char *suffix)
{
	m_prefix=prefix;
	m_suffix=suffix;
}

void Log::updateTime()
{
	TIME_SEC now=g_Clock->getTime();
	if(m_lasttime!=now)
	{
		time_t timeValue = static_cast<time_t>( now );
		struct tm * tm_now=localtime(&timeValue);
		strftime( m_timebuf, sizeof(m_timebuf) ,"[%H:%M:%S]: ", tm_now );
		strftime( m_datebuf, sizeof(m_datebuf) ,"%Y-%m-%d", tm_now);
		m_lasttime=now;
	}
}
