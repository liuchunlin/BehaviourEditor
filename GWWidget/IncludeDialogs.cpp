#include "IncludeDialogs.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if wxUSE_SPINCTRL
#include "wx/spinctrl.h"
#endif
#include "GWIDs.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "Label.h"
#include <windows.h>

IMPLEMENT_CLASS(IncludeDialog, wxDialog)

BEGIN_EVENT_TABLE(IncludeDialog, wxDialog)
EVT_BUTTON(wxID_ANY, IncludeDialog::OnButton)
EVT_CHECKLISTBOX(CTRL_TEE_INCLUDE_CHECKBOX, IncludeDialog::OnCheckboxToggle)
END_EVENT_TABLE()

extern wchar_t GConfigFileName[MAX_PATH];

IncludeDialog::IncludeDialog( const set<wstring>& Includes )
:m_Includes(Includes)
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

	Create( NULL, wxID_ANY, _("Include"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE );

	wxBoxSizer *pTopSizer = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer *pBox1 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *pBox2 = new wxBoxSizer( wxVERTICAL );

	m_pTagListBox = new wxCheckListBox(	this, CTRL_TEE_INCLUDE_CHECKBOX,	wxPoint(10, 10), wxSize(200, 150) );

	pBox1->Add( m_pTagListBox, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	pTopSizer->Add( pBox1, 0, wxGROW|wxALL, 0 );

	wxButton *btn = new wxButton(this, CTRL_TEE_INCLUDE_OK, _T("确定"));
	pBox2->Add( btn, 0, wxALL|wxALIGN_CENTER, 5 );

	pTopSizer->Add( pBox2, 0, wxGROW|wxALL, 0 );

	SetSizer(pTopSizer);
	pTopSizer->Fit(this);

	Init();
}

extern std::wstring MakeValidPathName(const wchar_t* pPathName);

bool FindTEELibs(vector<wstring>& fileNames, vector<WIN32_FIND_DATA>* pFileAttrs)
{
	TCHAR strTEELibPath[256];
	DWORD nBuffer = GetPrivateProfileString(_T("TEELibPath"), _T("path"), _T(""), strTEELibPath, sizeof(strTEELibPath) / sizeof(TCHAR), GConfigFileName);
	if (nBuffer == 0)
	{
		wxMessageDialog dialog(NULL, _T("请先检查配置文件:Profile.ini"), _T("错误"), wxOK | wxICON_ERROR);
		dialog.ShowModal();
		return false;
	}

	wstring szFind;
	szFind = MakeValidPathName(strTEELibPath) + _T("*.TEE");

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(szFind.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE) return false;
	do
	{
		//过滤本级目录和父目录
		if (findData.cFileName[0] == '.')
			continue;
		//如果找到的是目录，跳出
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		//找到的是文件
		fileNames.push_back(findData.cFileName);
		if (pFileAttrs)
			pFileAttrs->push_back(findData);
	} while (FindNextFile(hFind, &findData));
	FindClose(hFind);
	return true;
}

void	IncludeDialog::Init()
{
	vector<wstring> TEEFiles;
	FindTEELibs(TEEFiles, nullptr);

	auto iter = TEEFiles.begin();
	for( int iIndex = 0; iter != TEEFiles.end(); ++iter, ++iIndex )
	{
		const wstring& teeFile = (*iter);
		m_pTagListBox->Append( (teeFile.c_str()) );
		if( m_Includes.find(teeFile) != m_Includes.end() )
		{
			m_pTagListBox->Check( iIndex );
		}
	}
}

IncludeDialog::~IncludeDialog()
{
}
void	IncludeDialog::OnButton(wxCommandEvent& event)
{
	if( event.GetId() == CTRL_TEE_INCLUDE_OK )
	{
		m_Includes.clear();
		unsigned int iCount = m_pTagListBox->GetCount();
		for( unsigned int k = 0; k < iCount; ++k )
		{
			if( m_pTagListBox->IsChecked( k ) )
			{
				m_Includes.insert( m_pTagListBox->GetString( k ).wc_str() );
			}
		}

		this->EndModal( CTRL_TEE_INCLUDE_OK );
	}
	event.Skip();
}
void	IncludeDialog::OnCheckboxToggle(wxCommandEvent& event)
{

}
