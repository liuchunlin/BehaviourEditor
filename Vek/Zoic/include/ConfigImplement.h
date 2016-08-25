/*! @file
	@ingroup utils
	@brief 替换CONFIG_DECLARE,实现配置文件的自动读取
*/

#ifdef CONFIG_DEFINE
#undef CONFIG_DEFINE
#endif
#define CONFIG_DEFINE(TYPE,NAME) \
	it=m_vars.find(#NAME); \
	if(it!=m_vars.end()) \
	{ \
		Item item(it->second.c_str()); \
		m_##NAME=item; \
		dprintf("%s=%s\n",#NAME,it->second.c_str()); \
	} \
	else \
	{ \
		printf("conf:not find %s=<%s>\n",#NAME,#TYPE); \
		return false; \
	}

/*! @file
	@ingroup utils
	@brief 替换CONFIG_DECLARE,实现配置文件的自动读取
*/

#ifdef CONFIG_DEFINE_EX
#undef CONFIG_DEFINE_EX
#endif
#define CONFIG_DEFINE_EX(TYPE,NAME,VALUE) \
	it=m_vars.find(#NAME); \
	if(it!=m_vars.end()) \
	{ \
		Item item(it->second.c_str()); \
		m_##NAME=item; \
		dprintf("%s=%s\n",#NAME,it->second.c_str()); \
	} \
	else \
	{ \
		Item item(#VALUE); \
		m_##NAME=item; \
		printf("conf:not find %s=<%s>,default value '%s' will be used\n",#NAME,#TYPE,#VALUE); \
	}

#ifndef __CONFIGIMPLEMENT_H
#define __CONFIGIMPLEMENT_H

static std::map<std::string,std::string>::iterator it;

//! @cond 
class Item
{
public:
	Item(const char *str);
	operator const char * (void) const;
	operator char (void) const;
	operator unsigned char (void) const;
	operator short (void) const;
	operator unsigned short (void) const;
	operator int (void) const;
	operator unsigned int (void) const;
	operator long (void) const;
	operator unsigned long (void) const;
	operator long long (void) const;
	operator unsigned long long (void) const;
	operator float (void) const;
	operator double (void) const;
	operator Zoic::DateTime (void) const;
	operator Zoic::Date (void) const;
	operator Zoic::Time (void) const;
private:
	long long getInteger() const;
	unsigned long long getIntegerU() const;
	double getDouble() const;
	const char * m_str;
};

inline Item::Item(const char *str)
{
	m_str=str;
}

inline Item::operator const char * (void) const
{
	return m_str;
}

inline Item::operator char (void) const
{
	return static_cast<char>(getInteger());
}

inline Item::operator unsigned char (void) const
{
	return static_cast<unsigned char>(getIntegerU());
}

inline Item::operator short (void) const
{
	return static_cast<short>(getInteger());
}

inline Item::operator unsigned short (void) const
{
	return static_cast<unsigned short>(getIntegerU());
}

inline Item::operator int (void) const
{
	return static_cast<int>(getInteger());
}

inline Item::operator unsigned int (void) const
{
	return static_cast<unsigned int>(getIntegerU());
}

inline Item::operator long (void) const
{
	return static_cast<long>(getInteger());
}

inline Item::operator unsigned long (void) const
{
	return static_cast<unsigned long>(getIntegerU());
}

inline Item::operator long long (void) const
{
	return getInteger();
}

inline Item::operator unsigned long long (void) const
{
	return getIntegerU();
}

inline Item::operator float (void) const
{
	return static_cast<float>(getDouble());
}

inline Item::operator double (void) const
{
	return getDouble();
}

inline long long Item::getInteger() const
{
	return strtoll(m_str,NULL,10);
}

inline unsigned long long Item::getIntegerU() const
{
	return strtoull(m_str,NULL,10);
}

inline double Item::getDouble() const
{
	return strtod(m_str,NULL);
}

inline Item::operator Zoic::DateTime (void) const
{
	int year=0,month=0,day=0;
	int hour=0,minute=0,second=0;
	sscanf(m_str,"%d%*c%d%*c%d%*c%d%*c%d%*c%d",&year,&month,&day,&hour,&minute,&second);
	return Zoic::DateTime(year,month,day,hour,minute,second);
}

inline Item::operator Zoic::Date (void) const
{
	int year=0,month=0,day=0;
	sscanf(m_str,"%d%*c%d%*c%d",&year,&month,&day);
	return Zoic::Date(year,month,day);
}

inline Item::operator Zoic::Time (void) const
{
	int hour=0,minute=0,second=0;
	sscanf(m_str,"%d%*c%d%*c%d",&hour,&minute,&second);
	return Zoic::Time(hour,minute,second);
}

//! @endcond 
#endif
