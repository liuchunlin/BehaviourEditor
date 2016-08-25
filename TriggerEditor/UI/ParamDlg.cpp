#include "ParamDlg.h"
#include "TgrIDs.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
#include "TgrApp.h"
#include "SParameter.h"
#include "EnumType.h"
#include "ParameterEditor.h"
#include "VarDlg.h"
#include "TgrFrame.h"
#include "OptionEditDlg.h"
#include "SAdjust.h"
#include "SFunction.h"

IMPLEMENT_CLASS(ParamDlg, wxDialog)

BEGIN_EVENT_TABLE(ParamDlg, wxDialog)
EVT_SIZE(ParamDlg::OnSize)
EVT_IDLE(ParamDlg::OnIdle)
EVT_BUTTON(wxID_ANY, ParamDlg::OnButton)
EVT_COMBOBOX(PARAMDLG_COMBO_ADJUST, ParamDlg::OnAdjustComboChanged)
EVT_TEXT(PARAMDLG_COMBO_ADJUST, ParamDlg::OnAdjustComboTextChanged)
EVT_CHOICE(PARAMDLG_CHOICE_CONTEXT_VARIABLE, ParamDlg::OnContextVariableChoice)
EVT_CHOICE(PARAMDLG_CHOICE_LOCAL_VARIABLE, ParamDlg::OnLocalVariableChoice)
EVT_CHOICE(PARAMDLG_CHOICE_GLOBAL_VARIABLE, ParamDlg::OnGlobalVariableChoice)
EVT_CHOICE(PARAMDLG_CHOICE_FUNCTION, ParamDlg::OnFunctionChoice)
EVT_RADIOBUTTON(wxID_ANY, ParamDlg::OnRadioButton)
EVT_HYPERLINK(TGR_MAIN_HYPERLINK, ParamDlg::OnHyperLinkLeftUp)
EVT_TEXT(PARAMDLG_EDIT_VALUE, ParamDlg::OnValueText)
EVT_RADIOBOX(PARAMDLG_BOOL_VALUE, ParamDlg::OnBoolRadio)
END_EVENT_TABLE()

inline static int NodeTypeToKey(Space::SPACE_NODE_TYPE nodeType)
{
	if (nodeType == Space::SNT_SFUNCCALL)
		return Space::SNT_TFUNCCALL;
	else
		return nodeType;
}

ParamDlg::ParamDlg(Space::SNode *pNode, DWORD ePrefType /* = PDT_NORMAL_PARAM */)
	:m_pOrgNode(pNode)
	, m_pTMPOption(NULL)
	, m_pAdjustCombo(NULL)
	, m_pContextVariableChoice(NULL)
	, m_pLocalVariableChoice(NULL)
	, m_pGlobalVariableChoice(NULL)
	, m_pFunctionChoice(NULL)
	, m_pContentBox(NULL)
	, m_pEditValue(NULL)
	, m_bRequireResize(false)
	, m_ePrefType(ePrefType)
	, m_pOk(nullptr)
{
	assert(m_pOrgNode != NULL);
	//初始内容
	InitContents();

	//初始控件
	InitControls();
}

ParamDlg::~ParamDlg()
{
}

void ParamDlg::OnSize(wxSizeEvent& event)
{
	m_bRequireResize = true;
	event.Skip();
}

void ParamDlg::OnIdle(wxIdleEvent& event)
{
	if (m_bRequireResize)
	{
		ResizeControls();
		m_bRequireResize = false;
		Refresh();
	}
	event.Skip();
}

void ParamDlg::InitContents()
{
	if (m_pOrgNode == NULL)	return;

	if (m_ePrefType == PDT_VARIABLE_CHOICE)
	{
		if (m_NewNodes.find(Space::SNT_GLOBAL_VARREF) == m_NewNodes.end())
		{
			if (m_pOrgNode->GetErrorMask() == Space::VS_OK && m_pOrgNode->IsA(Space::SNT_GLOBAL_VARREF))
			{
				m_NewNodes[Space::SNT_GLOBAL_VARREF] = Space::SNode::Clone(m_pOrgNode);
			}
			else
			{
				m_NewNodes[Space::SNT_GLOBAL_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_GLOBAL_VARREF>(NULL);
			}
		}
		if (m_NewNodes.find(Space::SNT_LOCAL_VARREF) == m_NewNodes.end())
		{
			if (m_pOrgNode->GetErrorMask() == Space::VS_OK && m_pOrgNode->IsA(Space::SNT_LOCAL_VARREF))
			{
				m_NewNodes[Space::SNT_LOCAL_VARREF] = Space::SNode::Clone(m_pOrgNode);
			}
			else
			{
				m_NewNodes[Space::SNT_LOCAL_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_LOCAL_VARREF>(NULL);
			}
		}
		if (m_NewNodes.find(Space::SNT_PARAM_VARREF) == m_NewNodes.end())
		{
			if (m_pOrgNode->GetErrorMask() == Space::VS_OK && m_pOrgNode->IsA(Space::SNT_PARAM_VARREF))
			{
				m_NewNodes[Space::SNT_PARAM_VARREF] = Space::SNode::Clone(m_pOrgNode);
			}
			else
			{
				m_NewNodes[Space::SNT_PARAM_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_PARAM_VARREF>(NULL);
			}
		}
		return;
	}

	TEE::NodeBase* pTEETmp = m_pOrgNode->TEETmp();//Adjust,Function...
	wstring optionCodeName;
	if (pTEETmp)
		pTEETmp->GetOptionCode(optionCodeName);
	else if (m_pOrgNode && m_pOrgNode->ClassType() == Space::SNT_ADJUST)
		optionCodeName = dynamic_cast<Space::SAdjust*>(m_pOrgNode)->m_optionName;
	assert(!optionCodeName.empty());
	if (optionCodeName.empty()) return;

	m_pTMPOption = TEE::GTEEMgr->FindOptionFromCode(optionCodeName);
	assert(m_pTMPOption != NULL);
	if (m_pTMPOption == NULL) return;

	m_NewNodes[NodeTypeToKey(m_pOrgNode->ClassType())] = Space::SNode::Clone(m_pOrgNode);

	//调整值
	if (m_NewNodes.find(Space::SNT_ADJUST) == m_NewNodes.end())
	{
		m_NewNodes[Space::SNT_ADJUST] = nullptr;
	}

	//全局变量
	if (m_NewNodes.find(Space::SNT_GLOBAL_VARREF) == m_NewNodes.end())
	{
		m_NewNodes[Space::SNT_GLOBAL_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_GLOBAL_VARREF>(m_pTMPOption);
	}

	//事件环境变量
	if (m_NewNodes.find(Space::SNT_CONTEXT_VARREF) == m_NewNodes.end())
	{
		m_NewNodes[Space::SNT_CONTEXT_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_CONTEXT_VARREF>(m_pTMPOption);
	}

	//局部变量
	if (m_NewNodes.find(Space::SNT_LOCAL_VARREF) == m_NewNodes.end())
	{
		m_NewNodes[Space::SNT_LOCAL_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_LOCAL_VARREF>(m_pTMPOption);
	}

	//参数变量
	if (m_NewNodes.find(Space::SNT_PARAM_VARREF) == m_NewNodes.end())
	{
		m_NewNodes[Space::SNT_PARAM_VARREF] = Space::GSpaceMgr->BuildDefault<Space::SNT_PARAM_VARREF>(m_pTMPOption);
	}

	//处理功能
	{
		//TEE function
		TEE::VerifyFunctionReturnType verifyFunctionReturnType;
		verifyFunctionReturnType.ReturnCode = m_pTMPOption->CodeName();
		vector<TEE::NodeBase*> tfunctions;
		TEE::GTEEMgr->FindNodesOfType(TEE::NT_FUNCTION, verifyFunctionReturnType, tfunctions);
		for (auto iter = tfunctions.begin(); iter != tfunctions.end(); ++iter)
		{
			TEE::NodeBase* pNode = (*iter);
			wstring nodeName = pNode->DisplayName(true);
			m_FunctionContents.insert(make_pair(nodeName, reinterpret_cast<void*>(pNode)));
		}

		//Space function
		vector<Space::SFunction*> sfunctions;
		Vek::ObjLink<Space::SFunction>::VisitAll([&sfunctions](Space::SFunction* pFunction)->void {
			if (Space::IsNodeInSpace(pFunction))
				sfunctions.push_back(pFunction);
		});
		for (size_t index = 0; index < sfunctions.size(); ++index)
		{
			Space::SFunction* func = sfunctions[index];
			if (func->GetReturnTEE() == m_pTMPOption)
				m_FunctionContents.insert(make_pair(SFuncTag + TEXT(" - ") + func->GetReadText(), reinterpret_cast<void*>(func)));
		}

		if (m_NewNodes.find(Space::SNT_TFUNCCALL) == m_NewNodes.end() && !m_FunctionContents.empty())
		{
			Vek::TreeNodeBase* base = static_cast<Vek::TreeNodeBase*>(m_FunctionContents.begin()->second);
			m_NewNodes[Space::SNT_TFUNCCALL] = Space::BuildStatementSOrT(base);
		}
	}

	//处理编辑
	{
		if (TEE::VT_INT == TEE::GetValueTypeEnum(m_pTMPOption->CodeName()) ||
			TEE::VT_REAL == TEE::GetValueTypeEnum(m_pTMPOption->CodeName()) ||
			TEE::VT_STRING == TEE::GetValueTypeEnum(m_pTMPOption->CodeName()))
		{
			if (m_NewNodes.find(Space::SNT_EDIT_VALUE) == m_NewNodes.end())
			{
				m_NewNodes[Space::SNT_EDIT_VALUE] = Space::GSpaceMgr->BuildDefault<Space::SNT_EDIT_VALUE>(m_pTMPOption);
			}
		}
		else if (TEE::VT_BOOLEAN == TEE::GetValueTypeEnum(m_pTMPOption->CodeName()))
		{
			if (m_NewNodes.find(Space::SNT_BOOLEAN) == m_NewNodes.end())
			{
				m_NewNodes[Space::SNT_BOOLEAN] = Space::GSpaceMgr->BuildDefault<Space::SNT_BOOLEAN>(m_pTMPOption);
			}
		}
	}
}

extern int FindChoiceItemByData(wxChoice* pChoice, void* data);

void ParamDlg::InitControls()
{
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP | wxWS_EX_VALIDATE_RECURSIVELY);

	Create((wxWindow*)wxGetApp().GetTgrFrame(), wxID_ANY, m_pTMPOption == NULL ? m_ePrefType == PDT_VARIABLE ? _T("变量") : _T("Unknow Type") : m_pTMPOption->DisplayName(), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER);
	SetAffirmativeId(TGR_PREF_CANCEL);
	SetEscapeId(wxID_ANY);

	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	const int s_iSpace = 4;
	wxSize RadioSize(85, 20);

	//调整值
	if (m_ePrefType & PDT_ADJUST)
	{
		m_pAdjustSizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_ADJUST] = new wxRadioButton(this, Space::SNT_ADJUST, wxT("选项(&O):"), wxDefaultPosition, RadioSize);
		m_pAdjustSizer->Add(m_Radios[Space::SNT_ADJUST], 0, wxALL, s_iSpace);
		m_pAdjustCombo = new wxComboBox(this, PARAMDLG_COMBO_ADJUST, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN);
		m_pAdjustSizer->Add(m_pAdjustCombo, 1, wxALL, s_iSpace);
		wxButton* buttonEditOption = new wxButton(this, TGR_PREFERENCES_EDIT_OPTION_BUTTON, TEXT("编辑选项"), wxDefaultPosition, wxDefaultSize);
		m_pAdjustSizer->Add(buttonEditOption, 0, wxALL, s_iSpace);
		main_sizer->Add(m_pAdjustSizer, 0, wxALL | wxEXPAND, 0);

		UpdateAdjustList();
	}

	//环境变量
	if ((m_ePrefType & PDT_VARIABLE) && m_ePrefType != PDT_VARIABLE_CHOICE)
	{
		m_pContextVariableSizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_CONTEXT_VARREF] = new wxRadioButton(this, Space::SNT_CONTEXT_VARREF, wxT("事件环境变量(&P):"), wxDefaultPosition, RadioSize);
		m_pContextVariableSizer->Add(m_Radios[Space::SNT_CONTEXT_VARREF], 0, wxALL, s_iSpace);
		m_pContextVariableChoice = new wxChoice(this, PARAMDLG_CHOICE_CONTEXT_VARIABLE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
		m_pContextVariableSizer->Add(m_pContextVariableChoice, 1, wxALL | wxALIGN_RIGHT, s_iSpace);
		main_sizer->Add(m_pContextVariableSizer, 0, wxALL | wxEXPAND, 0);

		UpdateContextVariableList();
	}
	//局部变量
	if (m_ePrefType & PDT_VARIABLE)
	{
		m_pLocalVariableSizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_LOCAL_VARREF] = new wxRadioButton(this, Space::SNT_LOCAL_VARREF, wxT("局部变量(&L):"), wxDefaultPosition, RadioSize);
		m_pLocalVariableSizer->Add(m_Radios[Space::SNT_LOCAL_VARREF], 0, wxALL, s_iSpace);
		m_pLocalVariableChoice = new wxChoice(this, PARAMDLG_CHOICE_LOCAL_VARIABLE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
		m_pLocalVariableSizer->Add(m_pLocalVariableChoice, 1, wxALL | wxALIGN_RIGHT, s_iSpace);
		main_sizer->Add(m_pLocalVariableSizer, 0, wxALL | wxEXPAND, 0);

		UpdateLocalVariableList();
	}
	//参数变量
	if (m_ePrefType & PDT_VARIABLE)
	{
		m_pParamVariableSizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_PARAM_VARREF] = new wxRadioButton(this, Space::SNT_PARAM_VARREF, wxT("参数变量(&L):"), wxDefaultPosition, RadioSize);
		m_pParamVariableSizer->Add(m_Radios[Space::SNT_PARAM_VARREF], 0, wxALL, s_iSpace);
		m_pParamVariableChoice = new wxChoice(this, PARAMDLG_CHOICE_PARAM_VARIABLE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
		m_pParamVariableSizer->Add(m_pParamVariableChoice, 1, wxALL | wxALIGN_RIGHT, s_iSpace);
		main_sizer->Add(m_pParamVariableSizer, 0, wxALL | wxEXPAND, 0);

		UpdateParamVariableList();
	}
	//全局变量
	if ((m_ePrefType & PDT_VARIABLE))
	{
		wxBoxSizer *variable_sizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_GLOBAL_VARREF] = new wxRadioButton(this, Space::SNT_GLOBAL_VARREF, wxT("全局变量(&G):"), wxDefaultPosition, RadioSize);
		variable_sizer->Add(m_Radios[Space::SNT_GLOBAL_VARREF], 0, wxALL, s_iSpace);
		m_pGlobalVariableChoice = new wxChoice(this, PARAMDLG_CHOICE_GLOBAL_VARIABLE, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
		variable_sizer->Add(m_pGlobalVariableChoice, 1, wxALL | wxALIGN_RIGHT, s_iSpace);
		wxButton* buttonEditVar = new wxButton(this, TGR_PREFERENCES_ADJUST_VAREDIT, _T("编辑变量"), wxDefaultPosition, wxDefaultSize);
		variable_sizer->Add(buttonEditVar, 0, wxALL, s_iSpace);
		main_sizer->Add(variable_sizer, 0, wxALL | wxEXPAND, 0);

		UpdateGlobalVariableList();
	}
	//功能
	if (!m_FunctionContents.empty() && (m_ePrefType & PDT_FUNCTION))
	{
		wxBoxSizer *function_sizer = new wxBoxSizer(wxHORIZONTAL);
		m_Radios[Space::SNT_TFUNCCALL] = new wxRadioButton(this, Space::SNT_TFUNCCALL, wxT("功能(&F):"), wxDefaultPosition, RadioSize);
		function_sizer->Add(m_Radios[Space::SNT_TFUNCCALL], 0, wxALL, s_iSpace);
		m_pFunctionChoice = new wxChoice(this, PARAMDLG_CHOICE_FUNCTION, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);
		function_sizer->Add(m_pFunctionChoice, 1, wxALL, s_iSpace);
		main_sizer->Add(function_sizer, 0, wxALL | wxEXPAND, 0);

		map<wstring, void*>::iterator iter(m_FunctionContents.begin());
		for (; iter != m_FunctionContents.end(); ++iter)
		{
			m_pFunctionChoice->Append((*iter).first, (*iter).second);
		}
		m_pFunctionChoice->Select(0);

		//功能描述及选择
		if (1)
		{
			wxBoxSizer *content_sizer = new wxBoxSizer(wxHORIZONTAL);
			content_sizer->Add(80 + s_iSpace, 0);
			m_pContentBox = new wxStaticBox(this, -1, wxEmptyString, wxDefaultPosition, wxSize(350, 150));
			content_sizer->Add(m_pContentBox, 2, wxEXPAND | wxLEFT | wxRIGHT, s_iSpace);

			main_sizer->Add(content_sizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 0);
		}
	}

	//编辑值
	if (m_pTMPOption)
	{
		switch (TEE::GetValueTypeEnum(m_pTMPOption->CodeName()))
		{
			//数值型值
		case TEE::VT_INT:
		case TEE::VT_REAL:
		{
			wxBoxSizer *value_sizer = new wxBoxSizer(wxHORIZONTAL);
			m_Radios[Space::SNT_EDIT_VALUE] = new wxRadioButton(this, Space::SNT_EDIT_VALUE, wxT("值(&A):"), wxDefaultPosition, RadioSize);
			value_sizer->Add(m_Radios[Space::SNT_EDIT_VALUE], 0, wxALL, s_iSpace);
			m_pEditValue = new GWTextCtrl(this, PARAMDLG_EDIT_VALUE, _T(""), wxDefaultPosition, wxSize(120, 20), 0, wxTextValidator(wxFILTER_NUMERIC));
			value_sizer->Add(m_pEditValue, 0, wxALL, s_iSpace);

			main_sizer->Add(value_sizer, 0, wxALL | wxEXPAND, 0);

			break;
		}
		case TEE::VT_STRING:
		{
			wxBoxSizer *value_sizer = new wxBoxSizer(wxHORIZONTAL);
			m_Radios[Space::SNT_EDIT_VALUE] = new wxRadioButton(this, Space::SNT_EDIT_VALUE, wxT("值(&A):"), wxDefaultPosition, RadioSize);
			value_sizer->Add(m_Radios[Space::SNT_EDIT_VALUE], 0, wxALL, s_iSpace);
			m_pEditValue = new GWTextCtrl(this, PARAMDLG_EDIT_VALUE, _T(""), wxDefaultPosition, wxSize(380, 20));
			value_sizer->Add(m_pEditValue, 1, wxALL, s_iSpace);

			main_sizer->Add(value_sizer, 0, wxALL | wxEXPAND, 0);
			break;
		}
		case TEE::VT_BOOLEAN:
		{
			wxBoxSizer *value_sizer = new wxBoxSizer(wxHORIZONTAL);
			m_Radios[Space::SNT_BOOLEAN] = new wxRadioButton(this, Space::SNT_BOOLEAN, wxT("值(&A):"), wxDefaultPosition, RadioSize);
			value_sizer->Add(m_Radios[Space::SNT_BOOLEAN], 0, wxALL | wxEXPAND, s_iSpace);
			wxString choices[] = { _T("True"), _T("False"), };
			m_pBoolRadio = new wxRadioBox(this, PARAMDLG_BOOL_VALUE, _T(""), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choices), choices, 1, wxRA_SPECIFY_ROWS);
			value_sizer->Add(m_pBoolRadio, 0, wxALL | wxEXPAND, s_iSpace);

			main_sizer->Add(value_sizer, 0, wxALL | wxEXPAND, 0);

			break;
		}
		default:
			break;
		}
		if (m_pEditValue != NULL)
		{
			m_pEditValue->SetClientData(reinterpret_cast<void*>(m_pTMPOption));
		}
	}

	{
		wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
		button_sizer->Add(10, 10, 1);
		m_pOk = new wxButton(this, TGR_PREF_OK, _T("确定(&E)"));
		m_pOk->SetDefault();
		button_sizer->Add(m_pOk, 0, wxCENTER | wxLEFT, s_iSpace);
		button_sizer->Add(10, 0); // spacer
		button_sizer->Add(new wxButton(this, TGR_PREF_CANCEL, _T("取消(&C)")), 0, wxCENTER | wxLEFT, s_iSpace);

		main_sizer->Add(button_sizer, 0, wxALL | wxEXPAND, 2);
	}
	SetSizer(main_sizer);

	GetSizer()->SetSizeHints(this);
	GetSizer()->Fit(this);

	//选中org
	if (m_pOrgNode != NULL)
	{
		switch (m_pOrgNode->ClassType())
		{
		case Space::SNT_PARAMETER:
		{
			break;
		}
		case Space::SNT_TFUNCCALL:
		{
			if (TEE::NodeBase *pTmpNode = m_pOrgNode->TEETmp())
			{
				int iSelEvent = FindChoiceItemByData(m_pFunctionChoice, pTmpNode);
				m_pFunctionChoice->Select(iSelEvent);
				m_pFunctionChoice->SetFocus();
			}
			break;
		}
		case Space::SNT_SFUNCCALL:
		{
			if (Space::SNode* pfunc = m_pOrgNode->Definition())
			{
				int iSelEvent = FindChoiceItemByData(m_pFunctionChoice, pfunc);
				m_pFunctionChoice->Select(iSelEvent);
				m_pFunctionChoice->SetFocus();
			}
			break;
		}
		case Space::SNT_ADJUST:
		{
			TEE::NodeBase *pTmpNode = m_pOrgNode->TEETmp();
			assert(pTmpNode != NULL);
			if (pTmpNode != NULL)
			{
				assert(pTmpNode->ClassType() == TEE::NT_ADJUST);
				if (pTmpNode->ClassType() == TEE::NT_ADJUST)
				{
					int iSelEvent = m_pAdjustCombo->FindString(pTmpNode->DisplayName());
					m_pAdjustCombo->Select(iSelEvent);
					m_pAdjustCombo->SetFocus();
				}
			}
			break;
		}
		case Space::SNT_EDIT_VALUE:
		{
			assert(m_pEditValue != NULL);
			if (m_pEditValue != NULL)
			{
				m_pEditValue->SetValue(m_pOrgNode->Name().c_str());
				m_pEditValue->SelectAll();
				m_pEditValue->SetFocus();
			}
			break;
		}
		case Space::SNT_BOOLEAN:
		{
			assert(m_pBoolRadio != NULL);
			if (m_pBoolRadio != NULL)
			{
				if (m_pOrgNode->Name() == _T("0"))
				{
					m_pBoolRadio->Select(1);
				}
				else
				{
					m_pBoolRadio->Select(0);
				}
				m_pBoolRadio->SetFocus();
			}
			break;
		}
		case Space::SNT_GLOBAL_VARREF:
		{
			assert(m_pGlobalVariableChoice != NULL);
			if (m_pGlobalVariableChoice != NULL)
			{
				m_pGlobalVariableChoice->SetStringSelection(m_pOrgNode->GetReadText());
				m_pGlobalVariableChoice->SetFocus();
			}
			break;
		}
		case Space::SNT_CONTEXT_VARREF:
		{
			assert(m_pContextVariableChoice != NULL);
			if (m_pContextVariableChoice != NULL)
			{
				m_pContextVariableChoice->SetStringSelection(m_pOrgNode->GetReadText());
				m_pContextVariableChoice->SetFocus();
			}
			break;
		}
		case Space::SNT_LOCAL_VARREF:
		{
			assert(m_pLocalVariableChoice != NULL);
			if (m_pLocalVariableChoice != NULL)
			{
				m_pLocalVariableChoice->SetStringSelection(m_pOrgNode->GetReadText());
				m_pLocalVariableChoice->SetFocus();
			}
			break;
		}
		case Space::SNT_PARAM_VARREF:
		{
			assert(m_pParamVariableChoice != NULL);
			if (m_pParamVariableChoice != NULL)
			{
				m_pParamVariableChoice->SetStringSelection(m_pOrgNode->GetReadText());
				m_pParamVariableChoice->SetFocus();
			}
			break;
		}
		}

		if (Space::SNT_PARAMETER != m_pOrgNode->ClassType())
		{
			CheckRadio(NodeTypeToKey(m_pOrgNode->ClassType()));
		}
		UpdateLayout(MASK_EVERYTHING);
	}
}

void ParamDlg::OnButton(wxCommandEvent& event)
{
	if (event.GetId() == TGR_PREFERENCES_ADJUST_VAREDIT)
	{
		VarListDlg dlg(this);
		dlg.ShowModal();
		if (!CheckRadio(Space::SNT_GLOBAL_VARREF))
			return;

		UpdateGlobalVariableList();
		UpdateLayout(MASK_OK_BUTTON);

		if (m_pGlobalVariableChoice != NULL)
		{
			int i = m_pGlobalVariableChoice->GetSelection();
			if (i != -1)
			{
				Space::SNode* pVarItem = static_cast<Space::SNode*>(m_pGlobalVariableChoice->GetClientData(i));
				//assert( pVarItem != NULL );
				if (pVarItem != NULL)
				{
					m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(pVarItem->Name());
					m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(pVarItem);
					m_NewNodes[Space::SNT_GLOBAL_VARREF]->IsArray(pVarItem->IsArray());
				}
			}
		}
	}
	else if (event.GetId() == TGR_PREFERENCES_EDIT_OPTION_BUTTON)
	{
		OptionEditDlg optionEditDlg(this, m_pTMPOption);
		if (optionEditDlg.DoShowModal())
		{
			UpdateAdjustList();
		}
	}
	else
	{
		this->EndModal(event.GetId());
	}
	event.Skip();
}
void ParamDlg::OnRadioButton(wxCommandEvent &event)
{
	switch (event.GetId())
	{
	case Space::SNT_ADJUST:
		if (m_pAdjustCombo != NULL)
		{
			m_pAdjustCombo->SetFocus();
		}
		break;
	case Space::SNT_CONTEXT_VARREF:
		if (m_pContextVariableChoice != NULL)
		{
			m_pContextVariableChoice->SetFocus();
		}
		break;
	case Space::SNT_LOCAL_VARREF:
		if (m_pLocalVariableChoice != NULL)
		{
			m_pLocalVariableChoice->SetFocus();
		}
		break;
	case Space::SNT_PARAM_VARREF:
		if (m_pParamVariableChoice != NULL)
		{
			m_pParamVariableChoice->SetFocus();
		}
		break;
	case Space::SNT_GLOBAL_VARREF:
		if (m_pGlobalVariableChoice != NULL)
		{
			m_pGlobalVariableChoice->SetFocus();
		}
		break;
	case Space::SNT_TFUNCCALL:
		if (m_pFunctionChoice != NULL)
		{
			m_pFunctionChoice->SetFocus();
		}
		break;
	case Space::SNT_EDIT_VALUE:
		if (m_pEditValue != NULL)
		{
			m_pEditValue->SetFocus();
		}
		break;
	case Space::SNT_BOOLEAN:
		if (m_pBoolRadio != NULL)
		{
			m_pBoolRadio->SetFocus();
		}
		break;
	default:
		assert(false);
		break;
	}
	UpdateLayout(MASK_OK_BUTTON);
	event.Skip();
}

bool ParamDlg::CheckRadio(int eType)
{
	if (m_Radios.find(eType) == m_Radios.end())
	{
		assert(false);
		return false;
	}
	wxRadioButton* pRadio = m_Radios[eType];
	pRadio->SetValue(true);

	return true;
}

void ParamDlg::OnAdjustComboChanged(wxCommandEvent& event)
{
	if (!CheckRadio(Space::SNT_ADJUST))
		return;

	TEE::NodeBase* pTmpNode = static_cast<TEE::NodeBase*>(m_pAdjustCombo->GetClientData(m_pAdjustCombo->GetSelection()));
	if (pTmpNode == nullptr)
		return;
	if (m_NewNodes[Space::SNT_ADJUST] == nullptr)
	{
		m_NewNodes[Space::SNT_ADJUST] = Space::GSpaceMgr->BuildDefault<Space::SNT_ADJUST>(pTmpNode);
	}
	else
	{
		m_NewNodes[Space::SNT_ADJUST]->TEETmp(pTmpNode);
		m_NewNodes[Space::SNT_ADJUST]->Name(pTmpNode->DisplayName());
	}
	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnAdjustComboTextChanged(wxCommandEvent& event)
{
	if (!CheckRadio(Space::SNT_ADJUST))
		return;

	if (m_pAdjustCombo)
	{
		int MatchIndex = m_pAdjustCombo->FindString(event.GetString());
		if (MatchIndex != wxNOT_FOUND)
		{
			m_pAdjustCombo->SetSelection(MatchIndex);

			wxCommandEvent eventSelChange(wxEVT_COMMAND_COMBOBOX_SELECTED, m_pAdjustCombo->GetId());
			event.SetInt(MatchIndex);
			event.SetString(event.GetString());
			OnAdjustComboChanged(eventSelChange);
		}
	}
}
void ParamDlg::OnContextVariableChoice(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_CONTEXT_VARREF))
		return;

	TEE::NodeBase* pEventParam = static_cast<TEE::NodeBase*>(m_pContextVariableChoice->GetClientData(m_pContextVariableChoice->GetSelection()));
	assert(pEventParam != NULL);
	m_NewNodes[Space::SNT_CONTEXT_VARREF]->UserData(Space::SNode::EventParamName, reinterpret_cast<void*>(pEventParam));
	m_NewNodes[Space::SNT_CONTEXT_VARREF]->Name(pEventParam->CodeName());

	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnLocalVariableChoice(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_LOCAL_VARREF))
		return;

	Space::SNode* pVarItem = static_cast<Space::SNode*>(m_pLocalVariableChoice->GetClientData(m_pLocalVariableChoice->GetSelection()));
	assert(pVarItem != NULL);
	Space::GSpaceMgr->ReBuildVariableIndex(m_NewNodes[Space::SNT_GLOBAL_VARREF]);
	m_NewNodes[Space::SNT_LOCAL_VARREF]->TEETmp(pVarItem->TEETmp());
	m_NewNodes[Space::SNT_LOCAL_VARREF]->Definition(pVarItem);
	m_NewNodes[Space::SNT_LOCAL_VARREF]->Name(pVarItem->Name());
	m_NewNodes[Space::SNT_LOCAL_VARREF]->IsArray(pVarItem->IsArray());
	m_NewNodes[Space::SNT_LOCAL_VARREF]->UserData("bTgr", reinterpret_cast<void*>(pVarItem->ClassType() == Space::SNT_VARIABLE ? false : true));

	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnParamVariableChoice(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_PARAM_VARREF))
		return;

	Space::SNode* pVarItem = static_cast<Space::SNode*>(m_pParamVariableChoice->GetClientData(m_pParamVariableChoice->GetSelection()));
	assert(pVarItem != NULL);
	Space::GSpaceMgr->ReBuildVariableIndex(m_NewNodes[Space::SNT_GLOBAL_VARREF]);
	m_NewNodes[Space::SNT_PARAM_VARREF]->TEETmp(pVarItem->TEETmp());
	m_NewNodes[Space::SNT_PARAM_VARREF]->Definition(pVarItem);
	m_NewNodes[Space::SNT_PARAM_VARREF]->Name(pVarItem->Name());
	m_NewNodes[Space::SNT_PARAM_VARREF]->IsArray(pVarItem->IsArray());
	m_NewNodes[Space::SNT_PARAM_VARREF]->UserData("bTgr", reinterpret_cast<void*>(pVarItem->ClassType() == Space::SNT_VARIABLE ? false : true));

	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnGlobalVariableChoice(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_GLOBAL_VARREF))
		return;

	if (m_ePrefType == PDT_TRIGGER_CHOICE)
	{
		Space::SNode* pTrigger = static_cast<Space::SNode*>(m_pGlobalVariableChoice->GetClientData(m_pGlobalVariableChoice->GetSelection()));
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(pTrigger);
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(pTrigger->Name());
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->IsArray(pTrigger->IsArray());
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->UserData("bTgr", reinterpret_cast<void*>(pTrigger->ClassType() == Space::SNT_VARIABLE ? false : true));
	}
	else
	{
		Space::SNode* pVarItem = static_cast<Space::SNode*>(m_pGlobalVariableChoice->GetClientData(m_pGlobalVariableChoice->GetSelection()));
		assert(pVarItem != NULL);
		{
			//选取后，更新孩子
			Space::GSpaceMgr->ReBuildVariableIndex(m_NewNodes[Space::SNT_GLOBAL_VARREF]);
		}
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(pVarItem->Name());
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->TEETmp(pVarItem->TEETmp());
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(pVarItem);
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->IsArray(pVarItem->IsArray());
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->UserData("bTgr", reinterpret_cast<void*>(false));
	}

	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnFunctionChoice(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_TFUNCCALL))
		return;

	if (m_NewNodes[Space::SNT_TFUNCCALL] != m_pOrgNode)
	{
		SAFE_DELETE(m_NewNodes[Space::SNT_TFUNCCALL]);
	}

	Vek::TreeNodeBase* base = static_cast<Vek::TreeNodeBase*>(m_pFunctionChoice->GetClientData(m_pFunctionChoice->GetSelection()));
	m_NewNodes[Space::SNT_TFUNCCALL] = Space::BuildStatementSOrT(base);
	UpdateLayout(MASK_EVERYTHING);
}
void ParamDlg::OnValueText(wxCommandEvent& event)
{
	if (!CheckRadio(Space::SNT_EDIT_VALUE))
		return;

	m_NewNodes[Space::SNT_EDIT_VALUE]->Name(event.GetString().wc_str());
	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnBoolRadio(wxCommandEvent &event)
{
	if (!CheckRadio(Space::SNT_BOOLEAN))
		return;

	wxString value = event.GetString();
	if (value == _T("True"))
	{
		m_NewNodes[Space::SNT_BOOLEAN]->Name(_T("1"));
	}
	else if (value == _T("False"))
	{
		m_NewNodes[Space::SNT_BOOLEAN]->Name(_T("0"));
	}
	UpdateLayout(MASK_OK_BUTTON);
}
void ParamDlg::OnHyperLinkLeftUp(GWHyperlinkEvent& event)
{
	Space::SNode *pOrgNode = static_cast<Space::SNode*>(event.GetClientData());
	if (pOrgNode == NULL)
		return;
	CheckRadio(Space::SNT_TFUNCCALL);
	ParamDlg dlg(pOrgNode);
	dlg.PrefShowModal();
	UpdateLayout(MASK_EVERYTHING);
}
void ParamDlg::ResizeControls()
{
	if (m_pContentBox != NULL)
	{
		wxPoint beginPos = m_pContentBox->GetPosition();
		beginPos += wxPoint(8, 16);
		wxSize allSize = m_pContentBox->GetSize();

		ParamEditor::Resize(beginPos, allSize, m_ContextCtrls);
	}
}
Space::SNode* ParamDlg::PrefShowModal()
{
	Space::SNode* pNewWorkNode = NULL;
	if (ShowModal() == TGR_PREF_OK)
	{
		pNewWorkNode = GetNewWorkNode();
		if (pNewWorkNode != NULL)
		{
			Space::SNode::Swap(m_pOrgNode, pNewWorkNode);
			m_pOrgNode->Remove();

			pNewWorkNode->DominoOffect();
		}
	}

	map<int, Space::SNode*>::iterator iter(m_NewNodes.begin());
	for (; iter != m_NewNodes.end(); ++iter)
	{
		if ((*iter).second != pNewWorkNode)
		{
			SAFE_DELETE((*iter).second);
		}
	}
	m_NewNodes.clear();
	return pNewWorkNode;
}
void ParamDlg::UpdateLayout(int iMask /* = MASK_EVERYTHING */)
{
	Space::SNode* pNewWorkNode = GetNewWorkNode();

	if (iMask & MASK_OK_BUTTON)
	{
		Space::ErrorMask nValid = Space::SNode::CheckErrorTree(pNewWorkNode);
		if (m_pOk) m_pOk->Enable(nValid == Space::VS_OK);
	}

	if (iMask & MASK_PREFERENCES)
	{
		if (m_NewNodes.find(Space::SNT_TFUNCCALL) != m_NewNodes.end() && m_pContentBox != NULL)
		{
			Space::SNode* pFunctionNode = m_NewNodes[Space::SNT_TFUNCCALL];
			wxPoint beginPos = m_pContentBox->GetPosition();
			beginPos += wxPoint(8, 16);
			wxSize allSize = m_pContentBox->GetSize();
			ParamEditor::Build(this, beginPos, allSize, pFunctionNode, m_ContextCtrls);
		}
	}
}

void ParamDlg::UpdateAdjustList()
{
	if (m_pAdjustCombo == nullptr) return;
	m_pAdjustCombo->Clear();

	vector<TEE::NodeBase*> adjusts;
	m_pTMPOption->FindChildren(TEE::VerifyAlwaysOk(), adjusts);
	for (auto iter = adjusts.begin(); iter != adjusts.end(); ++iter)
	{
		TEE::NodeBase* pNode = (*iter);
		assert(pNode->ClassType() == TEE::NT_ADJUST);
		const wstring& KeyName = pNode->DisplayName();
		m_pAdjustCombo->Append(KeyName, reinterpret_cast<void*>(pNode));
	}

	if (adjusts.size() == 0)
	{
		if (m_NewNodes[Space::SNT_ADJUST])
		{
			m_NewNodes[Space::SNT_ADJUST]->TEETmp(nullptr);
		}
	}
	else if (m_NewNodes[Space::SNT_ADJUST] != nullptr &&
		m_pAdjustCombo->SetStringSelection(m_NewNodes[Space::SNT_ADJUST]->Name()))
	{
		OnAdjustComboChanged(wxCommandEvent());
	}
	else
	{
		m_pAdjustCombo->SetSelection(0);
		OnAdjustComboChanged(wxCommandEvent());
	}
	m_pAdjustCombo->Enable(adjusts.size() > 0);
}

wstring GetGlobalVariableText(Space::SNode* pVarImpl)
{
	return wxString::Format(_T("%s <%s>"), pVarImpl->Name().c_str(), pVarImpl->TEETmp() ? pVarImpl->TEETmp()->DisplayName().c_str() : _T("")).wc_str();
}

void ParamDlg::UpdateGlobalVariableList()
{
	if (m_pGlobalVariableChoice == NULL)
		return;
	m_pGlobalVariableChoice->Clear();

	Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot(Space::SNT_VARSET, NULL);
	assert(pVarSet != NULL);
	if (pVarSet == NULL)
		return;

	vector<Space::SNode*>	vars;
	if (m_ePrefType == PDT_VARIABLE_CHOICE)
	{
		pVarSet->FindChildren(Space::SVerifyAlwaysOk(), vars);
	}
	else
	{
		if (m_NewNodes.find(Space::SNT_GLOBAL_VARREF) == m_NewNodes.end())
			return;

		if (m_pTMPOption != NULL)
		{
			Space::SVerifyTEEType verify;
			verify.code = m_pTMPOption->CodeName();
			pVarSet->FindChildren(verify, vars);
		}
	}

	//插入普通变量
	if (!vars.empty())
	{
		for (vector<Space::SNode*>::iterator iter(vars.begin()); iter != vars.end(); ++iter)
		{
			Space::SNode* pVar = (*iter);
			m_pGlobalVariableChoice->Append(GetGlobalVariableText(pVar), reinterpret_cast<void*>(pVar));
		}
		//选中
		Space::SNode* pVarItem = m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition();
		if (pVarItem == NULL || !m_pGlobalVariableChoice->SetStringSelection(GetGlobalVariableText(pVarItem)))
		{
			pVarItem = vars.front();
			if (pVarItem->ClassType() != Space::SNT_TRIGGER)
			{
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(pVarItem->Name());
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->TEETmp(pVarItem->TEETmp());
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(pVarItem);
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->IsArray(pVarItem->IsArray());
				m_pGlobalVariableChoice->SetStringSelection(GetGlobalVariableText(pVarItem));
				m_pGlobalVariableChoice->Enable(true);
			}
		}
	}

	//插入触发器名字
	if (m_ePrefType == PDT_TRIGGER_CHOICE)
	{
		vector<Space::SNode*> triggers;
		Space::GSpaceMgr->FindTriggers(triggers);
		for (vector<Space::SNode*>::iterator iter(triggers.begin()); iter != triggers.end(); ++iter)
		{
			Space::SNode* pTgr = (*iter);
			m_pGlobalVariableChoice->Append(pTgr->Name() + _T(" <空间>"), reinterpret_cast<void*>(pTgr));
		}

		if (!triggers.empty())
		{
			int iSelect = m_pGlobalVariableChoice->GetSelection();
			if (iSelect == -1)
			{
				Space::SNode* pTrigger = m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition();
				if (pTrigger == NULL)
				{
					pTrigger = triggers[0];
				}
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(pTrigger);
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(pTrigger->Name());
				m_NewNodes[Space::SNT_GLOBAL_VARREF]->UserData("bTgr", false);
				m_pGlobalVariableChoice->SetStringSelection(pTrigger->Name() + _T(" <空间>"));
			}
			m_pGlobalVariableChoice->Enable(true);
		}
	}

	if (m_pGlobalVariableChoice->GetCount() <= 0)
	{
		m_pGlobalVariableChoice->Append(_T("没有符合的变量类型"), reinterpret_cast<void*>(NULL));
		m_pGlobalVariableChoice->Select(0);
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Name(_T(""));
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->TEETmp(NULL);
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->Definition(NULL);
		m_NewNodes[Space::SNT_GLOBAL_VARREF]->IsArray(false);
		m_pGlobalVariableChoice->Enable(false);
	}
}

static wstring GetParamVariableText(TEE::NodeBase* pParam, TEE::NodeBase* pType)
{
	return pParam->CodeName() + wxString::Format(_T(" <%s>"), pType->DisplayName().c_str()).wc_str();
}

void ParamDlg::UpdateContextVariableList()
{
	if (m_pContextVariableChoice == NULL || m_pTMPOption == NULL)
	{
		return;
	}
	m_pContextVariableChoice->Clear();

	vector<TEE::NodeBase*> eventParams;
	Space::SNode* pTriggerNode = wxGetApp().GetTgrFrame()->GetEditingNode();
	if (pTriggerNode && pTriggerNode->IsA(Space::SNT_TRIGGER))
	{
		Space::SNode* pEventRoot = pTriggerNode->FindFirstChild(Space::SVerifyNodeType(Space::SNT_EVENTROOT));
		if (pEventRoot != NULL)
		{
			Space::SNode* pEvent = pEventRoot->FindFirstChild(Space::SVerifyNodeType(Space::SNT_EVENT));
			if (pEvent != NULL)
			{
				TEE::NodeBase* pTEEEvent = TEE::GTEEMgr->FindEventFromCode(pEvent->Name());
				if (pTEEEvent != NULL)
				{
					pTEEEvent->FindChildren(TEE::NodeTypeAndDisplayNamePredicate(TEE::VerifyNodeType(TEE::NT_CONTEXTPARAM), TEE::VerifyDisplayName(m_pTMPOption->CodeName())), eventParams, true);
				}
			}
		}
	}

	//插入参数变量
	if (eventParams.size() > 0)
	{
		for (vector<TEE::NodeBase*>::iterator iter(eventParams.begin()); iter != eventParams.end(); ++iter)
		{
			TEE::NodeBase* pVar = (*iter);
			m_pContextVariableChoice->Append(GetParamVariableText(pVar, m_pTMPOption), reinterpret_cast<void*>(pVar));
		}
		//选中
		TEE::NodeBase* pEventParam = reinterpret_cast<TEE::NodeBase*>(m_NewNodes[Space::SNT_CONTEXT_VARREF]->UserData(Space::SNode::EventParamName));
		if (pEventParam == NULL || !m_pContextVariableChoice->SetStringSelection(GetParamVariableText(pEventParam, m_pTMPOption).c_str()))
		{
			pEventParam = eventParams.front();
			m_NewNodes[Space::SNT_CONTEXT_VARREF]->UserData(Space::SNode::EventParamName, reinterpret_cast<void*>(pEventParam));
			m_NewNodes[Space::SNT_CONTEXT_VARREF]->Name(pEventParam->CodeName());
			m_pContextVariableChoice->SetStringSelection(GetParamVariableText(pEventParam, m_pTMPOption).c_str());
		}
	}

	if (m_pContextVariableChoice->GetCount() == 0)
	{
		m_NewNodes[Space::SNT_CONTEXT_VARREF]->Name(_T(""));
		m_NewNodes[Space::SNT_CONTEXT_VARREF]->UserData(Space::SNode::EventParamName, NULL);
	}
	m_pContextVariableSizer->ShowItems(m_pContextVariableChoice->GetCount() != 0);
}

wstring GetLocalVariableText(Space::SNode* pVarItem)
{
	return wxString::Format(TEXT("%s <%s>"), pVarItem->Name().c_str(), pVarItem->TEETmp() ? pVarItem->TEETmp()->DisplayName().c_str() : TEXT("???")).wc_str();
}

void ParamDlg::UpdateLocalVariableList()
{
	if (m_pLocalVariableChoice == NULL)
	{
		return;
	}
	m_pLocalVariableChoice->Clear();

	vector<Space::SNode*> LocalVariables;
	Space::SNode* pTriggerNode = wxGetApp().GetTgrFrame()->GetEditingNode();
	if (pTriggerNode)
	{
		Space::SNode* pActionRoot = pTriggerNode->FindFirstChild(Space::SVerifyNodeType(Space::SNT_ACTIONROOT));
		if (pActionRoot != NULL)
		{
			if (m_ePrefType == PDT_VARIABLE_CHOICE)
			{
				pActionRoot->FindChildren(Space::SVerifyNodeType(Space::SNT_VARIABLE), LocalVariables, false);
			}
			else if (m_pTMPOption)
			{
				pActionRoot->FindChildren(Space::NodeTypeAndTEETypePredicate(Space::SVerifyNodeType(Space::SNT_VARIABLE), Space::SVerifyTEEType(m_pTMPOption->CodeName())), LocalVariables, false);
			}
		}
	}

	if (LocalVariables.size() > 0)
	{
		//插入
		for (vector<Space::SNode*>::iterator iter(LocalVariables.begin()); iter != LocalVariables.end(); ++iter)
		{
			Space::SNode* pVarItem = (*iter);
			m_pLocalVariableChoice->Append(GetLocalVariableText(pVarItem), reinterpret_cast<void*>(pVarItem));
		}
		//选中
		Space::SNode* pVarItem = m_NewNodes[Space::SNT_LOCAL_VARREF]->Definition();
		if (pVarItem == NULL || !m_pLocalVariableChoice->SetStringSelection(GetLocalVariableText(pVarItem)))
		{
			pVarItem = LocalVariables.front();
			if (pVarItem->ClassType() != Space::SNT_TRIGGER)
			{
				m_NewNodes[Space::SNT_LOCAL_VARREF]->Name(pVarItem->Name());
				m_NewNodes[Space::SNT_LOCAL_VARREF]->TEETmp(pVarItem->TEETmp());
				m_NewNodes[Space::SNT_LOCAL_VARREF]->Definition(pVarItem);
				m_NewNodes[Space::SNT_LOCAL_VARREF]->IsArray(pVarItem->IsArray());
				m_pLocalVariableChoice->SetStringSelection(GetLocalVariableText(pVarItem));
				m_pLocalVariableChoice->Enable(true);
			}
		}
	}

	if (m_pLocalVariableChoice->GetCount() <= 0)
	{
		m_NewNodes[Space::SNT_LOCAL_VARREF]->Name(_T(""));
		m_NewNodes[Space::SNT_LOCAL_VARREF]->TEETmp(NULL);
		m_NewNodes[Space::SNT_LOCAL_VARREF]->Definition(NULL);
		m_NewNodes[Space::SNT_LOCAL_VARREF]->IsArray(false);
	}
	m_pLocalVariableSizer->ShowItems(m_pLocalVariableChoice->GetCount() != 0);
}

void ParamDlg::UpdateParamVariableList()
{
	if (m_pParamVariableChoice == NULL)
	{
		return;
	}
	m_pParamVariableChoice->Clear();

	vector<Space::SNode*> ParamVariables;
	Space::SNode* pFunctionNode = wxGetApp().GetTgrFrame()->GetEditingNode();
	if (pFunctionNode)
	{
		Space::SNode* pParamRoot = pFunctionNode->FindFirstChild(Space::SVerifyNodeType(Space::SNT_PARAMROOT));
		if (pParamRoot != NULL)
		{
			if (m_ePrefType == PDT_VARIABLE_CHOICE)
			{
				pParamRoot->FindChildren(Space::SVerifyNodeType(Space::SNT_VARIABLE), ParamVariables, false);
			}
			else if (m_pTMPOption)
			{
				pParamRoot->FindChildren(Space::NodeTypeAndTEETypePredicate(Space::SVerifyNodeType(Space::SNT_VARIABLE), Space::SVerifyTEEType(m_pTMPOption->CodeName())), ParamVariables, false);
			}
		}
	}

	if (ParamVariables.size() > 0)
	{
		//插入
		for (vector<Space::SNode*>::iterator iter(ParamVariables.begin()); iter != ParamVariables.end(); ++iter)
		{
			Space::SNode* pVarItem = (*iter);
			m_pParamVariableChoice->Append(GetLocalVariableText(pVarItem), reinterpret_cast<void*>(pVarItem));
		}
		//选中
		Space::SNode* pVarItem = m_NewNodes[Space::SNT_PARAM_VARREF]->Definition();
		if (pVarItem == NULL || !m_pParamVariableChoice->SetStringSelection(GetLocalVariableText(pVarItem)))
		{
			pVarItem = ParamVariables.front();
			if (pVarItem->ClassType() != Space::SNT_TRIGGER)
			{
				m_NewNodes[Space::SNT_PARAM_VARREF]->Name(pVarItem->Name());
				m_NewNodes[Space::SNT_PARAM_VARREF]->TEETmp(pVarItem->TEETmp());
				m_NewNodes[Space::SNT_PARAM_VARREF]->Definition(pVarItem);
				m_NewNodes[Space::SNT_PARAM_VARREF]->IsArray(pVarItem->IsArray());
				m_pParamVariableChoice->SetStringSelection(GetLocalVariableText(pVarItem));
				m_pParamVariableChoice->Enable(true);
			}
		}
	}

	if (m_pParamVariableChoice->GetCount() <= 0)
	{
		m_NewNodes[Space::SNT_PARAM_VARREF]->Name(_T(""));
		m_NewNodes[Space::SNT_PARAM_VARREF]->TEETmp(NULL);
		m_NewNodes[Space::SNT_PARAM_VARREF]->Definition(NULL);
		m_NewNodes[Space::SNT_PARAM_VARREF]->IsArray(false);
	}
	m_pParamVariableSizer->ShowItems(m_pParamVariableChoice->GetCount() != 0);
}

Space::SNode* ParamDlg::GetNewWorkNode()
{
	map<int, wxRadioButton*>::iterator	iter(m_Radios.begin());
	for (; iter != m_Radios.end(); ++iter)
	{
		wxRadioButton* pRadio = (*iter).second;
		if (pRadio->GetValue())
		{
			int iTypeIndex = (*iter).first;
			assert(m_NewNodes.find(iTypeIndex) != m_NewNodes.end());
			if (m_NewNodes.find(iTypeIndex) != m_NewNodes.end())
			{
				return m_NewNodes[iTypeIndex];
			}
			break;
		}
	}
	//assert( false );
	return NULL;
}
