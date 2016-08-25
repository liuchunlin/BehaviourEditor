#pragma once

#include <assert.h>
#pragma warning( disable : 4244 )

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;
typedef unsigned long long	QWORD;
typedef long long			TIME_MSEL;
typedef long				TIME_SEC;
typedef const char *		PCSTR;
typedef float				FLOAT;
typedef unsigned int		SOCKET;
typedef unsigned int		UINT;
typedef int					INT;
typedef char				ANSICHAR;

const int NETWORK_BUF_SIZE_		=	1024 * 16;
const int NETWORK_MSG_HEADER_	=   4;

#define SAFE_DELETE(p)		{ delete p; p = NULL; }

#define ARRAY_COUNT( array ) ( sizeof(array) / sizeof((array)[0]) )

template<typename T>
struct Deletor
{
	void operator()( T *p )
	{
		delete( p );
	}
};

#include <vector>
#include <map>
#include <set>
#include <list>
#include <string>
#include <stack>
#include <deque>
using namespace std;