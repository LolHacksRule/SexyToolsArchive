#include "FileDialog.h"
#include "../Common/Common.h"

using namespace Sexy;

#include <commdlg.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FileDialog::FileDialog(bool isSave)
{
	mIsSave = isSave;
	mAllowMultiple = false;
	mWarnIfExists = false;
	mGetDir = false;
	mNoChangeDir = false;

	mFileMustExist = !isSave;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static FileDialog *gFileDialog = NULL;
static std::string gLastPath;
static bool gGotDir;

static UINT CALLBACK MyOFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if(gFileDialog==NULL)
		return 0;

	if (uiMsg == WM_NOTIFY && gFileDialog->IsGetDir())
    {
     	LPOFNOTIFY lpofn = (LPOFNOTIFY)lParam;
        if(lpofn->hdr.code==CDN_FOLDERCHANGE)
		{
			std::string aNewPath;
			char aBuf[MAX_PATH+1];
			SendMessage(::GetParent(hdlg),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)aBuf);
			aNewPath = aBuf;
			if(aNewPath==gLastPath) // user hit ok button
			{
				PostMessage(::GetParent(hdlg),WM_COMMAND,MAKELONG( IDCANCEL, 0 ) ,NULL);
				gGotDir = true;
	       		return 1;
			}
			else
				gLastPath = aNewPath;
		}
    } 
	else if(uiMsg==CDN_INCLUDEITEM && gFileDialog->IsGetDir())
	{
		OFNOTIFYEX *anInfo = (OFNOTIFYEX*)lParam;

	}
	else if(uiMsg==WM_INITDIALOG)
	{

		if(!gFileDialog->GetOkTitle().empty())
		{
			std::string aStr = gFileDialog->GetOkTitle();
			SendMessage(::GetParent(hdlg),CDM_SETCONTROLTEXT,IDOK,(LPARAM)aStr.c_str()); 
		}
	}
	

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetGetDir(bool getDir)
{
	mGetDir = getDir;

	mFilterList.clear();
	AddFilter("*.zzz","Directories");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetTitle(const std::string &theTitle)
{
	mTitle = theTitle;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetOkTitle(const std::string &theOkTitle)
{
	mOkTitle = theOkTitle;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetInitialFileName(const std::string &theFileName)
{
	mInitialFileName = theFileName;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetInitialDir(const std::string &theDir)
{
	mInitialDir = theDir;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetAllowMultiple(bool allowMultiple)
{
	if(!mIsSave)
		mAllowMultiple = allowMultiple;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetFileMustExist(bool mustExist)
{
	mFileMustExist = mustExist;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetWarnIfExists(bool warnIfExists)
{
	if(mIsSave)
		mWarnIfExists = warnIfExists;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::SetNoChangeDir(bool noChange)
{
	mNoChangeDir = noChange;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileDialog::AddFilter(const std::string &theFilter, const std::string &theDescription)
{
	mFilterList.push_back(FilterPair(theFilter,theDescription));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const FileDialog::FileList& FileDialog::GetFileList()
{
	return mFileList;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string FileDialog::GetFile()
{
	if(mFileList.empty())
		return "";
	else
		return mFileList.front();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FileDialog::DoDialog(HWND theParent)
{
	mFileList.clear();

	const MAX_FILE_NAME = 8192;
	char aFileName[MAX_FILE_NAME];
	char aInitialDir[MAX_FILE_NAME];
	OPENFILENAME aStruct;
	memset(&aStruct,0,sizeof(aStruct));
	aStruct.lStructSize = sizeof(aStruct);
	aStruct.hwndOwner = theParent;

	std::string aTitle = mTitle;
	aStruct.lpstrTitle = aTitle.c_str();
	aStruct.lpfnHook = MyOFNHookProc;
	gFileDialog = this;
	gGotDir = false;
	gLastPath.erase();

	std::string aFilterStr;
	for(FilterList::iterator anItr = mFilterList.begin(); anItr!=mFilterList.end(); ++anItr)
	{
		const FilterPair &aPair = *anItr;
		aFilterStr.append(aPair.mDesc);
		aFilterStr.append(1,'\0');
		aFilterStr.append(aPair.mFilter);
		aFilterStr.append(1,'\0');
	}
	aFilterStr.append(2,'\0');
	aStruct.lpstrFilter = aFilterStr.c_str();

	aFileName[0] = '\0';
	std::string strFileName = mInitialFileName;
	strcpy(aFileName, strFileName.c_str());
	aStruct.lpstrFile = aFileName;
	aStruct.nMaxFile = MAX_FILE_NAME-1;

	if(mInitialDir.length())
	{
		std::string strInitialDir = mInitialDir;
		strcpy(aInitialDir, strInitialDir.c_str());
		aStruct.lpstrInitialDir = aInitialDir;
	}

	aStruct.Flags = OFN_EXPLORER;
	if(mAllowMultiple)
		aStruct.Flags |= OFN_ALLOWMULTISELECT;
	if(mWarnIfExists)
		aStruct.Flags |= OFN_OVERWRITEPROMPT;
	if(mFileMustExist)
		aStruct.Flags |= OFN_FILEMUSTEXIST;
	if(mNoChangeDir)
		aStruct.Flags |= OFN_NOCHANGEDIR;
	if(mGetDir || !mOkTitle.empty())
		aStruct.Flags |= OFN_ENABLEHOOK;

	BOOL aResult = FALSE;
	if(mIsSave)
		aResult = GetSaveFileName(&aStruct);
	else
		aResult = GetOpenFileName(&aStruct);

	gFileDialog = NULL;
	if(gGotDir) // user just selected a directory
		strcpy(aFileName,gLastPath.c_str());
	else if(!aResult)
		return false;
	else if(mGetDir) // strip filename off the folder
	{
		std::string aFileNameStr = GetFileDir(aFileName);
		strcpy(aFileName,aFileNameStr.c_str());
	}

	if(mAllowMultiple)
	{
		const char *aPtr = aFileName;
		std::string aDirectory = aPtr;
		aPtr+=aDirectory.length()+1;
		while(true)
		{
			std::string aStr = aPtr;
			if(aStr.empty())
			{
				if(mFileList.empty()) // only one file
					mFileList.push_back(aDirectory);
			
				break;
			}
			else
			{
				std::string aFullPath = aDirectory;
				aFullPath += '/';
				aFullPath += aStr;
				mFileList.push_back(aFullPath);
			}

			aPtr += aStr.length() + 1;
		}
	}
	else
		mFileList.push_back(aFileName);

	return true;
}
