#include "ProjectDialog.h"
#include "resource.h"
#include "ResourceManager.h"
#include "ProjectDialog.h"
#include "AttributeDialog.h"
#include "FileDialog.h"

#include "../Common/Common.h"

#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ProjectDialog::ProjectDialog()
{
	mDialogId = IDD_PROJECT_VIEW;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::ReadEntryFile()
{
	mEntryMap.clear();

	FILE *aFile = fopen("projects.txt","r");
	if (aFile==NULL)
		return;

	while (!feof(aFile))
	{
		char aName[512], anXML[512], aCode[512], aPrefix[512], aReturn[512];
		if (fgets(aName, 511, aFile)==NULL)
			break;

		if (fgets(anXML, 511, aFile)==NULL)
			break;

		if (fgets(aCode, 511, aFile)==NULL)
			break;

		if (fgets(aPrefix, 511, aFile)==NULL)
			break;

		fgets(aReturn, 511, aFile);
		if (aReturn[0] != '\n') // invalid file
		{
			mEntryMap.clear(); 
			break;
		}

		char *aPtr;
		aPtr = strchr(aName,'\n'); if (aPtr != NULL) *aPtr = '\0';
		aPtr = strchr(anXML,'\n'); if (aPtr != NULL) *aPtr = '\0';
		aPtr = strchr(aCode,'\n'); if (aPtr != NULL) *aPtr = '\0';
		aPtr = strchr(aPrefix,'\n'); if (aPtr != NULL) *aPtr = '\0';

		Entry &anEntry = mEntryMap[aName];
		anEntry.mXMLPath = anXML;
		anEntry.mCodePath = aCode;
		anEntry.mFunctionPrefix = aPrefix;
	}

	fclose(aFile);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::WriteEntryFile()
{
	FILE *aFile = fopen("projects.txt","w");
	if (aFile==NULL)
		return;

	for (EntryMap::iterator anItr = mEntryMap.begin(); anItr != mEntryMap.end(); ++anItr)
	{	
		Entry &anEntry = anItr->second;
		fprintf(aFile,"%s\n",anItr->first.c_str());
		fprintf(aFile,"%s\n",anEntry.mXMLPath.c_str());
		fprintf(aFile,"%s\n",anEntry.mCodePath.c_str());
		fprintf(aFile,"%s\n\n",anEntry.mFunctionPrefix.c_str());
	}

	fclose(aFile);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::InitListColumns()
{
	LVCOLUMN lvc; 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	lvc.fmt = LVCFMT_LEFT;

	RECT aRect;
	GetClientRect(mProjectList,&aRect);

	lvc.pszText = "Project";
	lvc.cx = 150;
	ListView_InsertColumn(mProjectList, 0, &lvc);

	lvc.cx = (aRect.right - aRect.left - lvc.cx - 50)/2;

	lvc.pszText = "XML";
	ListView_InsertColumn(mProjectList, 1, &lvc);

	lvc.pszText = "Code";
	ListView_InsertColumn(mProjectList, 2, &lvc);

	lvc.pszText = "Prefix";
	lvc.cx = 50;
	ListView_InsertColumn(mProjectList, 3, &lvc);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::PopulateList()
{
	ListView_DeleteAllItems(mProjectList);

	for (EntryMap::iterator anItr = mEntryMap.begin(); anItr != mEntryMap.end(); ++anItr)
	{
		Entry &anEntry = anItr->second;

		int anIndex;
		LVITEM anItem;
		anItem.iItem = 0x7ffffff;
		anItem.iSubItem = 0;
		anItem.mask = LVIF_PARAM;
		anIndex = ListView_InsertItem(mProjectList,(LPARAM)&anItem);

		ListView_SetItemText(mProjectList, anIndex, 0, (char*)anItr->first.c_str());
		ListView_SetItemText(mProjectList, anIndex, 1, (char*)anEntry.mXMLPath.c_str());
		ListView_SetItemText(mProjectList, anIndex, 2, (char*)anEntry.mCodePath.c_str());
		ListView_SetItemText(mProjectList, anIndex, 3, (char*)anEntry.mFunctionPrefix.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnInit()
{
	mProjectList = GetDlgItem(IDC_PROJECT_LIST);
	ListView_SetExtendedListViewStyle(mProjectList, LVS_EX_FULLROWSELECT);

	InitListColumns();
	ReadEntryFile();
	PopulateList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ProjectDialog::DoGenerate(ResourceManager &theMgr, const std::string &thePath, const std::string &theOutputPath, const std::string &thePrefix)
{
	if (!theMgr.ParseResourcesFile(thePath))
		return false;

	if (!theMgr.WriteSourceCode(theOutputPath, thePrefix))
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnNewProject()
{
	AttributeDialog aDialog;
	if (aDialog.DoDialog(mHWND)!=IDOK)
		return;

	Entry &anEntry = mEntryMap[aDialog.GetName()];
	anEntry.mXMLPath = aDialog.GetXMLPath();
	anEntry.mCodePath = aDialog.GetCodePath();
	anEntry.mFunctionPrefix = aDialog.GetFunctionPrefix();

	WriteEntryFile();
	PopulateList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnDeleteProject()
{
	int anIndex = ListView_GetSelectionMark(mProjectList);
	if (anIndex < 0)
		return;

	char aName[512];
	ListView_GetItemText(mProjectList, anIndex, 0, aName, 512);

	EntryMap::iterator anItr = mEntryMap.find(aName);
	if (anItr==mEntryMap.end())
		return;

	mEntryMap.erase(anItr);
	WriteEntryFile();
	PopulateList();


}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnModifyProject()
{
	int anIndex = ListView_GetSelectionMark(mProjectList);
	if (anIndex < 0)
		return;

	AttributeDialog aDialog;
	char aName[512], anXMLPath[512],aCodePath[512],aPrefix[512];
	ListView_GetItemText(mProjectList, anIndex, 0, aName, 512);
	ListView_GetItemText(mProjectList, anIndex, 1, anXMLPath, 512);
	ListView_GetItemText(mProjectList, anIndex, 2, aCodePath, 512);
	ListView_GetItemText(mProjectList, anIndex, 3, aPrefix, 512);

	aDialog.SetName(aName);
	aDialog.SetXMLPath(anXMLPath);
	aDialog.SetCodePath(aCodePath);
	aDialog.SetFunctionPrefix(aPrefix);

	if (aDialog.DoDialog(mHWND)!=IDOK)
		return;

	Entry &anEntry = mEntryMap[aDialog.GetName()];
	anEntry.mXMLPath = aDialog.GetXMLPath();
	anEntry.mCodePath = aDialog.GetCodePath();
	anEntry.mFunctionPrefix = aDialog.GetFunctionPrefix();

	WriteEntryFile();
	PopulateList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnReportUnused()
{
	int anIndex = ListView_GetSelectionMark(mProjectList);
	if (anIndex < 0)
		return;

	char anXMLPath[512], aCodePath[512];
	ListView_GetItemText(mProjectList, anIndex, 1, anXMLPath, 512);
	ListView_GetItemText(mProjectList, anIndex, 2, aCodePath, 512);

	ResourceManager aMgr;
	if (!aMgr.ParseResourcesFile(anXMLPath))
	{
		MessageBox(mHWND,aMgr.GetErrorText().c_str(),"Error",MB_OK);
		return;
	}


	std::string aBatchPath = aCodePath;
	GetFileDir(aBatchPath);
	int aLastSlash = (int)aBatchPath.find_last_of("\\");
	if (aLastSlash != -1)
		aBatchPath = aBatchPath.substr(0, aLastSlash);
	else
		aBatchPath += "\\";		//somehow no backslashes
	aBatchPath += "\\del unused resources.bat";
	FILE *aBatchFile = fopen(aBatchPath.c_str(), "wt");

	std::string aDir = GetFileDir(aCodePath);
	std::string aResult = aMgr.GetUnusedFiles(aDir, aBatchFile);
	
	if (aBatchFile)
		fclose(aBatchFile);

	if (aResult.empty())
		aResult = "All files used.";

	MessageBox(mHWND,aResult.c_str(),"Unused Resources",MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProjectDialog::OnGenerateProject()
{
	int anIndex = ListView_GetSelectionMark(mProjectList);
	if (anIndex < 0)
		return;

	char anXMLPath[512], aCodePath[512], aPrefix[512];
	ListView_GetItemText(mProjectList, anIndex, 1, anXMLPath, 512);
	ListView_GetItemText(mProjectList, anIndex, 2, aCodePath, 512);
	ListView_GetItemText(mProjectList, anIndex, 3, aPrefix, 512);

	ResourceManager aMgr;
	if (DoGenerate(aMgr,anXMLPath,aCodePath,aPrefix))
		MessageBox(mHWND,"Success!","Success",MB_OK);
	else
		MessageBox(mHWND,aMgr.GetErrorText().c_str(),"Error",MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
INT_PTR ProjectDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG: OnInit(); return TRUE;
		case WM_CLOSE: PostQuitMessage(0); return TRUE;

		case WM_COMMAND: 
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_NEW_PROJECT: OnNewProject(); return TRUE;
						case IDC_MODIFY_PROJECT: OnModifyProject(); return TRUE;
						case IDC_GENERATE_PROJECT: OnGenerateProject(); return TRUE;
						case IDC_DELETE_PROJECT: OnDeleteProject(); return TRUE;
						case IDC_REPORT_UNUSED: OnReportUnused(); return TRUE;
					}
					break;
			}
			return FALSE;

		case WM_NOTIFY:
			{
				NMHDR *aNotify = (NMHDR*)lParam;
				if (aNotify->code==LVN_KEYDOWN)
				{
					NMLVKEYDOWN *aKeyDown = (NMLVKEYDOWN*)aNotify;
					if (aKeyDown->wVKey==VK_DELETE)
					{
						OnDeleteProject();
						return TRUE;
					}
				}
			}
			return FALSE;
	}

	return FALSE;
}
