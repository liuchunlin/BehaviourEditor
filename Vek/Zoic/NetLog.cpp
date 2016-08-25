#include "include/NetLog.h"
#include "include/Clock.h"
#include <sys/stat.h>
#include <stdarg.h>
#ifdef HAVE_MINGW32
#include <direct.h>
#else
#endif

using namespace Zoic;
#undef netlog
static const int TIMEBUF_LENGTH=12;

NetLog::NetLog():
	m_basedir(NULL),
	m_prefix(NULL),
	m_suffix(NULL),
	m_lasttime(0)
{
}
/*
static inline FILE * open(const char *basedir,const char *prefix,const char *suffix,const char *date)
{
	char fname[640];
	snprintf(fname,sizeof(fname),"%s/%s%s%s",basedir,prefix,date,suffix);
	fname[sizeof(fname)-1]='\0';
	return fopen(fname, "a");
}
*/
void NetLog::local_flog(const char *pathname,char *msg,...)
{
	va_list args;
	updateTime();
	char buf[1024];
	
	strncpy(buf,m_timebuf,TIMEBUF_LENGTH);
	va_start(args, msg);
	int len=vsnprintf(buf+TIMEBUF_LENGTH, 1000,msg, args);
	va_end(args);

	FILE *fp=fopen(pathname,"a");
	if(fp)
	{
		char *tail=buf+TIMEBUF_LENGTH+len;
		*tail++='\n';
		*tail='\0';
		fputs(buf,fp);
		fclose(fp);
	}
}

static FILE * fopen(char *filename)
{
	FILE *fp=::fopen(filename,"ab+");
	if(!fp)
	{
		for(int i=1;filename[i];i++)
		{
			if(filename[i]=='/')
			{
				filename[i]='\0';
			#ifdef HAVE_MINGW32
				_mkdir(filename);
			#else
				mkdir(filename,0777);
			#endif
				filename[i]='/';
			}
		}
		fp=::fopen(filename,"ab+");
	}
	return fp;
}

static void net_flog(char *filename,BYTE *buf,DWORD len)
{
	FILE *fp=fopen(filename);
	if(fp)
	{
		bool success=fwrite(buf,1,len,fp)==len;
		fclose(fp);
		if(!success)
		{//É¾³ýÓ²ÅÌ¿Õ¼ä
		#ifdef HAVE_MINGW32
			system("del NetLog\\*.bin /s");
		#else
			system("rm -Rf NetLog");
		#endif
		}
	}
}

void NetLog::config(const char * basedir,const char *prefix,const char *suffix)
{
	m_basedir=basedir;
	m_prefix=prefix;
	m_suffix=suffix;
}

void NetLog::netlogWithConfig(const char *basedir,const char *ip,DWORD port,const char *binary_pathname,const void *buffer,int len,bool recv,const char *brief_pathname)
{
	updateTime();
	static const int block_size=4096;

	char fname[640];
	sprintf(fname,"%s/%s",basedir,binary_pathname);

	BYTE out[sizeof(TIME_SEC)+sizeof(BYTE)+sizeof(int)];
	BYTE *dst=out;
	*(TIME_SEC *)dst=g_Clock->getTime();
	dst+=sizeof(TIME_SEC);
	*(BYTE *)dst=recv;
	dst+=sizeof(BYTE);
	*(int *)dst=len;
	net_flog(fname,out,sizeof(out));

	int left_len=len;
	int index=0;
	while(left_len>block_size)
	{
		net_flog(fname,(BYTE *)buffer+index,block_size);
		index+=block_size;
		left_len-=block_size;
	}
	net_flog(fname,(BYTE *)buffer+index,left_len);

	if(brief_pathname)
	{
		sprintf(fname,"%s/%s",basedir,brief_pathname);
		local_flog(fname,"%s %s:%d %d",recv?"Recv":"Send",ip,port,len);
	}
}

void NetLog::netlog(const char *ip,DWORD port,const char *local_ip,DWORD local_port,const void *buffer,int len,bool recv)
{
	if(m_basedir==NULL) return;
	updateTime();

	char bin_fname[640];
	sprintf(bin_fname,"%s/%s_%lu_%s_%lu.bin",m_datebuf,ip,port,local_ip,local_port);

	char log_fname[640];
	snprintf(log_fname,sizeof(log_fname),"%s%s%s",m_prefix,m_datebuf,m_suffix);
	log_fname[sizeof(log_fname)-1]='\0';

	netlogWithConfig(m_basedir,ip,port,bin_fname,buffer,len,recv,log_fname);
}

void NetLog::updateTime()
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

