#ifndef __PROJECTDIALOG_H__
#define __PROJECTDIALOG_H__
#include "Dialog.h"

#include <string>
#include <map>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Sexy
{
class ResourceManager;

class ProjectDialog : public Dialog
{
protected:
	HWND mProjectList;

	struct Entry
	{
		std::string mXMLPath;
		std::string mCodePath;
		std::string mFunctionPrefix;
	};

	typedef std::map<std::string,Entry> EntryMap;
	EntryMap mEntryMap;

	bool DoGenerate(ResourceManager &theMgr, const std::string &thePath, const std::string &theOutputPath, const std::string &thePrefix);

	void InitListColumns();
	void PopulateList();
	void ReadEntryFile();
	void WriteEntryFile();

	void OnInit();
	void OnNewProject();
	void OnModifyProject();
	void OnGenerateProject();
	void OnDeleteProject();
	void OnReportUnused();

	virtual INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	ProjectDialog();
};

} // namespace Sexy;

#endif