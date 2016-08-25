#include "LuaClient.h"
#include "TgrApp.h"
#include "TgrFrame.h"
#include "TgrIDs.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------
const int	GFrameDefaultWidth = 1204;
const int	GFrameDefaultHeight = 768;

IMPLEMENT_APP(TgrEditApp)

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------
bool TgrEditApp::OnInit()
{
	//Parse command line.
	for (int Index = 0; Index < argc; ++Index)
	{
		wxString strArg = argv[Index];

		const wxChar IPOption[] = TEXT("-ip");
		const wxChar PortOption[] = TEXT("-port");
		if (strArg.CompareTo(IPOption, wxString::ignoreCase) == 0 && Index + 1 < argc)
		{
			wxString strValue = argv[Index+1];
			GLuaClient->m_ServerIP = strValue.ToAscii();
			Index++;
		}
		else if (strArg.CompareTo(PortOption, wxString::ignoreCase) == 0 && Index + 1 < argc)
		{
			wxString strValue = argv[Index+1];
			long Port = 0;
			if (strValue.ToLong(&Port))
			{
				GLuaClient->m_ServerPort = Port;
			}
			Index++;
		}
	}

	CreateStyles();

	// create the main application window
	m_TgrFrame = new TgrEditFrame( _T("ÊÂ¼þ±à¼­Æ÷"), TGR_FRAME_ID, wxDefaultPosition, wxSize(GFrameDefaultWidth, GFrameDefaultHeight), wxDEFAULT_FRAME_STYLE );
	m_TgrFrame->Show( true ) ;


	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

int TgrEditApp::OnExit()
{
	return 0;
}

void TgrEditApp::CreateStyles()
{
}
