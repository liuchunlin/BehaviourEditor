#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using namespace std;
#include "wx/treectrl.h"

class GalleryTree;
namespace Space
{
	class ICommand
	{
	public:
		ICommand();
		virtual ~ICommand(){}
		virtual bool		Execute() = 0;
		virtual bool		UnExecute() = 0;
	protected:
	};

}