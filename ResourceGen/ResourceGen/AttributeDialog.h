#ifndef __ATTRIBUTEDIALOG_H__
#define __ATTRIBUTEDIALOG_H__
#include "Dialog.h"

#include <string>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AttributeDialog : public Dialog
{
protected:
	std::string mName;
	std::string mXMLPath;
	std::string mCodePath;
	std::string mFunctionPrefix;

	virtual INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void GetDataFromWindows();
	virtual void SetDataToWindows();

	void OnBrowseXML();
	void OnBrowseCode();

public:
	AttributeDialog();

	const std::string& GetName() { return mName; }
	const std::string& GetXMLPath() { return mXMLPath; }
	const std::string& GetCodePath() { return mCodePath; }
	const std::string& GetFunctionPrefix() { return mFunctionPrefix; }

	void SetName(const std::string &theName) { mName = theName; }
	void SetXMLPath(const std::string &thePath) { mXMLPath = thePath; }
	void SetCodePath(const std::string &thePath) { mCodePath = thePath; }
	void SetFunctionPrefix(const std::string &thePath) { mFunctionPrefix = thePath; }
};

#endif