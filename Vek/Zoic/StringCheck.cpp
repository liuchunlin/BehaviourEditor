#include <string>
#include "include/StringCheck.h"
#include "include/Types.h"

using namespace Zoic;

bool StringCheck::isGB2312NoNumberSpaceCode(const char* pStr)
{
	union{
		char str[2];
		char chr;
	} searchStr;
	searchStr.str[1] = '\0';
	const char* temp = NULL; 
	unsigned char test;
	const char normalSignList[] = "\\~`!#$^*()-_+=|{}[];:'/?.";

	for( temp=pStr; *temp != NULL; ++temp )
	{
		test = static_cast<unsigned char>(*temp);
		if( test >= 'a' && test <='z' || test >= 'A' && test <='Z' )
		{
			continue;
		}
		searchStr.chr = *temp;
		if( strstr(normalSignList,searchStr.str) != NULL )
		{
			continue;
		}
		if( test<176 || test > 247 )//test<129 || test > 254
		{
			return false;
		}
		++temp;
		test = static_cast<unsigned char>(*temp);
		if(test<160 || test >254)//test<64 || test > 254
		{//后半个字符在64-254
			return false;
		}
	}
	return true;
}

bool StringCheck::hasForbiddenString( const char* pStr,const std::vector<std::string>& forbiddenList)
{
	size_t nLength = strlen(pStr);
	if(nLength==0)
	{
		return true;
	}
	BYTE* pNoChecks = new BYTE[nLength];
	memset(pNoChecks,0,sizeof(BYTE)*nLength);
	for(size_t k = 0; k < nLength-1; ++k)
	{//如果是gbk后半个字符，记录为不检查标记
		const char* checkStr = pStr + k;
		unsigned char test = static_cast<unsigned char>(*checkStr);
		if(test & 0x80)
		{
			pNoChecks[k+1]=1;
			++k;
		}
	}
	size_t i=0;
	for(i=0;i<forbiddenList.size();++i)
	{	
		const char* tempStr = pStr;
		const char* result;
		while(true)
		{	
			result=strstr( tempStr,forbiddenList[i].c_str() );
			if(result==NULL)
			{//不存在屏蔽字符串
				break;
			}
			size_t position = ( result - pStr ) /sizeof(char);
			//如果存在，找到该字符串位置并替换
			if(pNoChecks[position])
			{//不检查
				tempStr = result+1;
				continue;
			}
			delete [] pNoChecks;
			return true;
		}
	}
	delete [] pNoChecks;
	return false;
}

StringCheck::CHECK_RESULT StringCheck::replaceForbiddenString(char* pStr,const std::vector<std::string>& forbiddenList)
{
	size_t nLength = strlen(pStr);
	if(nLength<=0)
	{
		return CR_NORMAL;
	}
	BYTE* pNoChecks = new BYTE[nLength];
	memset(pNoChecks,0,sizeof(BYTE)*nLength);
	for(size_t k = 0; k < nLength-1; ++k)
	{//如果是gbk后半个字符，记录为不检查标记
		char* checkStr = pStr + k;
		unsigned char test = static_cast<unsigned char>(*checkStr);
		if(test & 0x80)
		{
			pNoChecks[k+1]=1;
			++k;
		}
	}
	BYTE* pMarks = new BYTE[nLength];
	memset(pMarks,0,sizeof(BYTE)*nLength);
	bool haveBadChar = false;//是否存在屏蔽字符串
	size_t i=0;
	for(i=0;i<forbiddenList.size();++i)
	{	
		char* tempStr = pStr;
		char* result;
		while(true)
		{	
			result=strstr( tempStr,forbiddenList[i].c_str() );
			if(result==NULL)
			{//不存在屏蔽字符串
				break;
			}
			size_t position = ( result - pStr ) /sizeof(char);
			//如果存在，找到该字符串位置并替换
			if(pNoChecks[position])
			{//不检查
				tempStr = result+1;
				continue;
			}
			haveBadChar = true;
			size_t size = strlen(forbiddenList[i].c_str());
			memset(&pMarks[position],1,sizeof(BYTE)*size);
			if(size+position>=nLength)//如果有有剩余字符串,继续标记
			{
				break;
			}
			else
			{
				tempStr = result+1;
			}
			
		}
	}
	if(haveBadChar)
	{
		for(i=0;i<nLength;++i)
		{
			if(pMarks[i])
			{
				pStr[i] = '*';
			}
		}
	}
	delete [] pMarks;
	delete [] pNoChecks;
	return haveBadChar ? CR_FORBIDDEN:CR_NORMAL;
}

