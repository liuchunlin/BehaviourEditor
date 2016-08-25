#pragma once

#include <assert.h>

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned long		DWORD;
typedef unsigned long long	QWORD;
typedef long long			TIME_MSEL;
typedef long				TIME_SEC;
typedef const char *		PCSTR;
typedef float				FLOAT;
typedef unsigned int		SOCKET;


const int NETWORK_BUF_SIZE		=	1024 * 16;
const int NETWORK_MSG_HEADER	=   4;
