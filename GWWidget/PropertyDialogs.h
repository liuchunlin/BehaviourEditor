#pragma once

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wx/propdlg.h"

#include "wx/datetime.h"
#include "wx/image.h"
#include "wx/bookctrl.h"
#include "wx/artprov.h"
#include "wx/imaglist.h"
#include "wx/sysopt.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include "wx/choice.h"
#include "NodeBase.h"

// Property sheet dialog
class TEEPropertyDialogBase: public wxPropertySheetDialog
{
	DECLARE_CLASS(TEEPropertyDialogBase)
public:
	TEEPropertyDialogBase( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	~TEEPropertyDialogBase();

	wxPanel*			CreatePage();
	wxString			GetDisplayName(){ return m_pDisplayName->GetValue(); }
	wxString			GetCodeName(){ return m_pCodeName->GetValue(); }
	wxString			GetTagsName(){ return m_pTags->GetValue(); }
	bool				DoModalBase( wxString& displayName, wxString& codeName, wxString& tags );

	void				OnButton(wxCommandEvent& event);

protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );

	DECLARE_EVENT_TABLE()
protected:
	wxTextCtrl			*m_pDisplayName;
	wxTextCtrl			*m_pCodeName;
	wxTextCtrl			*m_pTags;
	wxButton			*m_pAddTag;
	wxString			m_Title;
	wxString			m_TagRoot;
	TEE::NodeBase		*m_pNode;
	TEE::NODE_TYPE		m_eNodeType;
};

class EventPropertyDialog: public TEEPropertyDialogBase
{
public:
	EventPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeName, wxString& tags );
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
};

class ConditionPropertyDialog: public TEEPropertyDialogBase
{
public:
	ConditionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeName, wxString& tags );
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
};

class FunctionPropertyDialog: public TEEPropertyDialogBase
{
public:
	FunctionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeName, wxString& eReturnType, int& eReturnDetail, wxString& tags, bool& canBeAction);
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
private:
	wxChoice*		m_pReturnTypeChoice;
	wxChoice*		m_pReturnDetailChoice;
	wxCheckBox*		m_canBeActionCheck;
};

class OptionPropertyDialog: public TEEPropertyDialogBase
{
public:
	OptionPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeName, int& eValueType, wxString& tags );
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
private:
	wxChoice		*m_pValueTypeChoice;
};

class RootPropertyDialog: public TEEPropertyDialogBase
{
public:
	RootPropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeName, wxString& tags );
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
private:
};

class SpacePropertyDialog: public TEEPropertyDialogBase
{
public:
	SpacePropertyDialog( wxWindow* parent, const wxString& title, TEE::NodeBase* pNode );
	bool		DoModal( wxString& displayName, wxString& codeNames );
protected:
	virtual		void	DoPage( wxBoxSizer *pUserEdit, wxPanel* pPanel );
	virtual		bool	DoCheck( wxString& errCode );
private:
};