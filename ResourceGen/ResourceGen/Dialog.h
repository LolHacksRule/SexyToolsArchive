#ifndef __SEXY_DIALOG_H__
#define __SEXY_DIALOG_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Dialog
{
protected:
	HWND mHWND;
	int mResult;
	int mDialogId;

	static INT_PTR WINAPI StaticDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

	HWND GetDlgItem(int theItem);
	std::string GetWindowText(HWND theHWND);
	std::string GetWindowTextItem(int theItem);

	virtual void GetDataFromWindows();
	virtual void SetDataToWindows();

public:
	Dialog();

	void EndDialog(int theResult);
	int DoDialog(HWND theParent = NULL);
};

#endif