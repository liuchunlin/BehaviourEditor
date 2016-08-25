/*
** $Id: luac.c,v 1.54 2006/06/02 17:37:11 lhf Exp $
** Lua compiler (saves bytecodes to files; also list bytecodes)
** See Copyright Notice in lua.h
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

#define luac_c
#define LUA_CORE

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}

#define PROGNAME	"luac"		/* default program name */


static void printerror(const char* message)
{
	fprintf(stderr,"%s\n",message);
}

#define toproto(L,i) (clvalue(L->top+(i))->l.p)

static const Proto* combine(lua_State* L, int n)
{
	if (n==1)
		return toproto(L,-1);
	else
	{
		int i,pc;
		Proto* f=luaF_newproto(L);
		setptvalue2s(L,L->top,f); incr_top(L);
		f->source=luaS_newliteral(L,"=(" PROGNAME ")");
		f->maxstacksize=1;
		pc=2*n+1;
		f->code=luaM_newvector(L,pc,Instruction);
		f->sizecode=pc;
		f->p=luaM_newvector(L,n,Proto*);
		f->sizep=n;
		pc=0;
		for (i=0; i<n; i++)
		{
			f->p[i]=toproto(L,i-n-1);
			f->code[pc++]=CREATE_ABx(OP_CLOSURE,0,i);
			f->code[pc++]=CREATE_ABC(OP_CALL,0,1,1);
		}
		f->code[pc++]=CREATE_ABC(OP_RETURN,0,1,0);
		return f;
	}
}

static int writer(lua_State* L, const void* p, size_t size, void* u)
{
 UNUSED(L);
 return (fwrite(p,size,1,(FILE*)u)!=1) && (size!=0);
}

struct Smain
{
	vector<string>	inputfiles;
	string			outfile;
	bool			bstrip_debug_info;
};

static int pmain(lua_State* L)
{
	struct Smain* s = (struct Smain*)lua_touserdata(L, 1);

	if (!lua_checkstack(L,s->inputfiles.size())) return 1;
	for (unsigned int i=0; i<s->inputfiles.size(); i++)
	{
		const char* filename=s->inputfiles[i].c_str();
		if (luaL_loadfile(L,filename)!=0)
		{
			printerror(lua_tostring(L,-1));
			return 1;
		}
	}

	const Proto* f=combine(L,s->inputfiles.size());

	FILE* D= fopen(s->outfile.c_str(),"wb");
	if (D==NULL) return 1;

	lua_lock(L);
	luaU_dump(L,f,writer,D,s->bstrip_debug_info);
	lua_unlock(L);

	if (ferror(D)) return 1;
	if (fclose(D)) return 1;

	return 0;
}

bool LuaCMain(vector<string> inputfiles, string outfile, bool bstrip_debug_info)
{
	if (inputfiles.size()<=0) return false;

	lua_State* L=lua_open();
	if (L==NULL) return false;

	Smain s;
	s.inputfiles = inputfiles;
	s.outfile = outfile;
	s.bstrip_debug_info = bstrip_debug_info;
	bool bresult = lua_cpcall(L,pmain,&s)==0;
	lua_close(L);
	return bresult;
}
