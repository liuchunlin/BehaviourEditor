#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <vector>
using namespace std;
namespace Space
{
	class SNode;
}

class ParamEditor
{
public:
	static	void	Build( wxWindow* pParent, wxPoint beginPos, wxSize allSize, Space::SNode *pSNode, vector<wxControl*>& ctrls );
	static	void	Resize( wxPoint beginPos, wxSize allSize, const vector<wxControl*>& ctrls );
};