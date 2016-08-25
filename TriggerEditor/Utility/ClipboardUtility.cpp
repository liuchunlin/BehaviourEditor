#include <Windows.h>
#include <string>
#include <assert.h>
using std::string;

// Optimization macros (uses __pragma to enable inside a #define).
#define PRAGMA_DISABLE_OPTIMIZATION __pragma(optimize("",off))
#ifdef _DEBUG
#define PRAGMA_ENABLE_OPTIMIZATION  __pragma(optimize("",off))
#else
#define PRAGMA_ENABLE_OPTIMIZATION  __pragma(optimize("",on))
#endif

// Disabling optimizations helps to reduce the frequency of OpenClipboard failing with error code 0. It still happens
// though only with really large text buffers and we worked around this by changing the editor to use an intermediate
// text buffer for internal operations.
PRAGMA_DISABLE_OPTIMIZATION 

//
// Copy text to clipboard.
//
void appClipboardCopy( const char* Str )
{
	if( OpenClipboard(GetActiveWindow()) )
	{
		EmptyClipboard();
		HGLOBAL GlobalMem;
		INT StrLen = strlen(Str);
		GlobalMem = GlobalAlloc( GMEM_MOVEABLE, sizeof(char)*(StrLen+1) );
		assert(GlobalMem);
		char* Data = (char*) GlobalLock( GlobalMem );
		strcpy_s( Data, (StrLen+1), Str );
		GlobalUnlock( GlobalMem );
		if( SetClipboardData( CF_TEXT, GlobalMem ) == NULL )
		{
			assert(FALSE && "SetClipboardData failed.");
		}
		CloseClipboard();
	}
}

//
// Paste text from clipboard.
//
string appClipboardPaste()
{
	string Result;
	if( OpenClipboard(GetActiveWindow()) )
	{
		HGLOBAL GlobalMem = NULL;
		GlobalMem = GetClipboardData( CF_TEXT );
		if( !GlobalMem )
		{
			Result = "";
		}
		else
		{
			void* Data = GlobalLock( GlobalMem );
			assert( Data );	
			Result = (char*)Data;
			GlobalUnlock( GlobalMem );
		}
		CloseClipboard();
	}
	else 
	{
		Result="";
	}
	return Result;
}

PRAGMA_ENABLE_OPTIMIZATION 

