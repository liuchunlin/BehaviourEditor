// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/treectrl.h"
#include "wx/richtext/richtextctrl.h"

#include <map>
#include <string>
using namespace std;

class TgrEditFrame;
class TgrEditApp : public wxApp
{
public:
	// override base class virtuals
	// ----------------------------

	// this one is called on application startup and is a good place for the app
	// initialization (doing it here and not in the ctor allows to have an error
	// return: if OnInit() returns false, the application terminates)
	virtual bool		OnInit();
	virtual int			OnExit();
	void				CreateStyles();
	TgrEditFrame*		GetTgrFrame(){ return m_TgrFrame; }
private:
	TgrEditFrame		*m_TgrFrame;
};

DECLARE_APP(TgrEditApp)


extern wchar_t GConfigFileName[MAX_PATH];