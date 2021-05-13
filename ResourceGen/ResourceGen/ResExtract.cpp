#include "SexyAppFramework/SexyAppBase.h"
#include "SexyAppFramework/ResourceManager.h"

using namespace Sexy;

bool LoadInitResources(SexyAppBase *theApp, bool exitOnFailure)
{
	ResourceManager &aMgr = *theApp->mResourceManager;
	try
	{
		IMAGE_CURSOR_DRAGGING = aMgr.GetImageThrow(CURSOR_DRAGGING);
		IMAGE_CURSOR_HAND = aMgr.GetImageThrow(CURSOR_HAND);
		IMAGE_CURSOR_POINTER = aMgr.GetImageThrow(CURSOR_POINTER);
		IMAGE_CURSOR_TEXT = aMgr.GetImageThrow(CURSOR_TEXT);
		IMAGE_TITLE_BAR = aMgr.GetImageThrow(TITLE_BAR);
		IMAGE_TITLE_SCREEN = aMgr.GetImageThrow(TITLE_SCREEN);
	}
	catch(ResourceManagerException &ex)
	{
		if (exitOnFailure)
		{
			MessageBox(NULL,ex.what.c_str(),"Resource Error",MB_OK)
			exit(0);
		}
		return false;
	}
	return true;
}

bool LoadMainResources(SexyAppBase *theApp, bool exitOnFailure)
{
	ResourceManager &aMgr = *theApp->mResourceManager;
	try
	{
		IMAGE_BKG = aMgr.GetImageThrow(BKG);
		IMAGE_BLACK_HOLE = aMgr.GetImageThrow(BLACK_HOLE);
		IMAGE_BLACK_HOLE_COVER = aMgr.GetImageThrow(BLACK_HOLE_COVER);
		IMAGE_BONUS_BAR = aMgr.GetImageThrow(BONUS_BAR);
		IMAGE_CAVITY_LIGHT = aMgr.GetImageThrow(CAVITY_LIGHT);
		IMAGE_CAVITY_NORMAL = aMgr.GetImageThrow(CAVITY_NORMAL);
		IMAGE_CHECKBOX = aMgr.GetImageThrow(CHECKBOX);
		IMAGE_CHECKER_TEXTURES = aMgr.GetImageThrow(CHECKER_TEXTURES);
		IMAGE_DIALOG = aMgr.GetImageThrow(DIALOG);
		IMAGE_DIALOG_BUTTON = aMgr.GetImageThrow(DIALOG_BUTTON);
		IMAGE_EDIT_BOX = aMgr.GetImageThrow(EDIT_BOX);
		IMAGE_ELECT = aMgr.GetImageThrow(ELECT);
		IMAGE_ENDLESS_GAME = aMgr.GetImageThrow(ENDLESS_GAME);
		IMAGE_FOIL = aMgr.GetImageThrow(FOIL);
		IMAGE_FOIL_BOTTOM = aMgr.GetImageThrow(FOIL_BOTTOM);
		IMAGE_FOIL_TOP = aMgr.GetImageThrow(FOIL_TOP);
		IMAGE_GEMS = aMgr.GetImageThrow(GEMS);
		IMAGE_HINT_BUBBLE = aMgr.GetImageThrow(HINT_BUBBLE);
		IMAGE_MAIN_SCREEN = aMgr.GetImageThrow(MAIN_SCREEN);
		IMAGE_OPTIONS_DOWN = aMgr.GetImageThrow(OPTIONS_DOWN);
		IMAGE_OVER_FONT = aMgr.GetImageThrow(OVER_FONT);
		IMAGE_QUIT_DOWN = aMgr.GetImageThrow(QUIT_DOWN);
		IMAGE_QUIT_TO_WINDOWS = aMgr.GetImageThrow(QUIT_TO_WINDOWS);
		IMAGE_SCORES = aMgr.GetImageThrow(SCORES);
		IMAGE_SELECT = aMgr.GetImageThrow(SELECT);
		IMAGE_SLIDER_THUMB = aMgr.GetImageThrow(SLIDER_THUMB);
		IMAGE_SLIDER_TRACK = aMgr.GetImageThrow(SLIDER_TRACK);
		IMAGE_SPARKLE = aMgr.GetImageThrow(SPARKLE);
		IMAGE_SWAPPER_1 = aMgr.GetImageThrow(SWAPPER_1);
		IMAGE_SWAPPER_2 = aMgr.GetImageThrow(SWAPPER_2);
		IMAGE_TIMED_GAME = aMgr.GetImageThrow(TIMED_GAME);
		IMAGE_TOOTH_GUY_ANIM = aMgr.GetImageThrow(TOOTH_GUY_ANIM);
		IMAGE_UNTIMED_GAME = aMgr.GetImageThrow(UNTIMED_GAME);
		IMAGE_WAIT_COMPONENT = aMgr.GetImageThrow(WAIT_COMPONENT);
	}
	catch(ResourceManagerException &ex)
	{
		if (exitOnFailure)
		{
			MessageBox(NULL,ex.what.c_str(),"Resource Error",MB_OK)
			exit(0);
		}
		return false;
	}
	return true;
}

