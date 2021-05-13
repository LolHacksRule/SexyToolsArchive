#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "ProjectDialog.h"

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int)
{
	InitCommonControls();

//	DoParseResources("c:\\gamesrc\\cpp\\copy of winsweet\\properties\\resources.xml","c:\\gamesrc\\cpp\\copy of winsweet\\ResExtract");
	ProjectDialog aDialog;
	aDialog.DoDialog();


	return 0;
}