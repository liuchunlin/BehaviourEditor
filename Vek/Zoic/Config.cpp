#include "include/Config.h"

using namespace Zoic;

static char *trim(char *src)
{
	char *ret;
	if(!src) return NULL;
	ret=src;
	while(*ret==' '||*ret=='\r'||*ret=='\n'||*ret=='\t') ret++;
	char *tmp=ret+strlen(ret)-1;

	while((*tmp==' '||*tmp=='\r'||*tmp=='\n'||*tmp=='\t')&&tmp>ret) tmp--;
	*(tmp+1)=0;
	return ret;
}

Config::Config()
	:m_loaded(false)
{
}

void Config::load(const char *file)
{
	FILE *fp = fopen(file, "r");
	if(fp == NULL)
	{
		printf("Cannot open file %s\n", file);
		exit(0);
	}
	
	char buf[1024];
	char *key=buf;
	char *val;
	while(fgets(buf,1024,fp))
	{
		if(*buf!='#')
		{
			val=strstr(buf,"=");
			if(val) *val++=0;
			char *t_key=trim(key);
			char *t_val=trim(val);
			if(t_key&&t_val) 
			{
				m_vars[t_key]=t_val;
			}
		}
	}
	fclose(fp);
	m_loaded = true;
}
