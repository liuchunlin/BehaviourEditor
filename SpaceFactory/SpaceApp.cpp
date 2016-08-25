
#include "SpaceApp.h"
#if wxUSE_HELP
#include "wx/cshelp.h"
#endif
#include "wx/richtext/richtextbuffer.h"
#include "wx/richtext/richtextxml.h"
#include "wx/richtext/richtexthtml.h"
#include "wx/fs_mem.h"
#include "SpaceFrame.h"

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(SpaceApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool SpaceApp::OnInit()
{
#if wxUSE_HELP
	wxHelpProvider::Set(new wxSimpleHelpProvider);
#endif

	// Add extra handlers (plain text is automatically added)
	wxRichTextBuffer::AddHandler(new wxRichTextXMLHandler);
	wxRichTextBuffer::AddHandler(new wxRichTextHTMLHandler);

	// Add image handlers
#if wxUSE_LIBPNG
	wxImage::AddHandler( new wxPNGHandler );
#endif

#if wxUSE_LIBJPEG
	wxImage::AddHandler( new wxJPEGHandler );
#endif

#if wxUSE_GIF
	wxImage::AddHandler( new wxGIFHandler );
#endif

#if wxUSE_FILESYSTEM
	wxFileSystem::AddHandler( new wxMemoryFSHandler );
#endif

	// create the main application window
	SpaceFrame *frame = new SpaceFrame(_T("Trigger Edit Editor"), wxID_ANY, wxDefaultPosition, wxSize(700, 600));

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(true);

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

int SpaceApp::OnExit()
{
	return 0;
}
