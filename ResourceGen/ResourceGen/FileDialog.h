#ifndef __SEXY_FILEDIALOG_H__
#define __SEXY_FILEDIALOG_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <list>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FileDialog
{
public:
	struct FilterPair
	{
		std::string mFilter;
		std::string mDesc;

		FilterPair() { }
		FilterPair(const std::string &f, const std::string &d) : mFilter(f), mDesc(d) { }
	};
	typedef std::list<FilterPair> FilterList;
	typedef std::list<std::string> FileList;

protected:
	bool mIsSave;
	bool mGetDir;
	bool mAllowMultiple;
	bool mWarnIfExists;
	bool mFileMustExist;
	bool mNoChangeDir;
	FilterList mFilterList;
	FileList mFileList;
	std::string mTitle;
	std::string mOkTitle;
	std::string mInitialFileName;
	std::string mInitialDir;

public:
	FileDialog(bool isSave = false);

	void SetTitle(const std::string &theTitle);
	void SetOkTitle(const std::string &theOkTitle);
	void SetInitialFileName(const std::string &theFileName);
	void SetInitialDir(const std::string &theDir);
	void SetAllowMultiple(bool allowMultiple);
	void SetWarnIfExists(bool warnIfExists);
	void SetFileMustExist(bool mustExist);
	void SetGetDir(bool getDir);
	void SetNoChangeDir(bool noChange);
	void AddFilter(const std::string &theFilter, const std::string &theDescription);

	const FileList& GetFileList();

	const std::string& GetOkTitle() { return mOkTitle; }
	bool IsGetDir() { return mGetDir; }

	std::string GetFile();

	bool DoDialog(HWND theParent = NULL);
};

#endif