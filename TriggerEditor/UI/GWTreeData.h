#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"

class GWTreeData : public wxTreeItemData
{
public:
	GWTreeData( Space::SNode* pNode ):m_pUserData(pNode){ }
	~GWTreeData(){}

	Space::SNode*		UserData(){ return m_pUserData; }
private:
	Space::SNode*		m_pUserData;

};
