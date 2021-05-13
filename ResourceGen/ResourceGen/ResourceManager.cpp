#include "ResourceManager.h"
#include "../Common/XMLParser.h"
#include "../Common/Common.h"

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::ResourceManager() 
{
	mHasFailed = false;
	mXMLParser = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::~ResourceManager()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void GetStandardFileName(std::string &theName)
{
	for (int i=0; i<(int)theName.length(); i++)
	{
		char aChar = tolower(theName[i]);
		if (aChar=='\\')
			aChar = '/';

		theName[i] = aChar;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void AddTrailingSlash(std::string &thePath)
{
	if (thePath.empty())
		return;

	if (thePath[thePath.length()-1]=='\\')
		thePath[thePath.length()-1]='/';
	else
		thePath += '/';	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void RemoveExtensionAndUnderscores(std::string &thePath)
{
	if (thePath.empty())
		return;

	GetStandardFileName(thePath);
	int aSlashPos = ((int)thePath.find_last_of('/'))+1;

	if (thePath[aSlashPos]=='_')
		thePath = thePath.substr(0,aSlashPos) + thePath.substr(aSlashPos+1);

	if (thePath.empty())
		return;

	int aDotPos = (int)thePath.find_last_of('.');
	if (aDotPos != std::string::npos)
		thePath = thePath.substr(0,aDotPos);

	if (thePath[thePath.length()-1]=='_')
		thePath = thePath.substr(0,thePath.length()-1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string ResourceManager::GetErrorText()
{
	return mError;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::HadError()
{
	return !mError.empty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string	ResourceManager::GetUnusedFilesForDir(const std::string &theBasePath, const std::string &theDir, FILE *theBatchFile /* = NULL */)
{
	std::string aReport;
	std::string aPath = theBasePath;
	AddTrailingSlash(aPath);
	aPath += theDir;
	AddTrailingSlash(aPath);
	GetStandardFileName(aPath);
	aPath += "*.*";

	std::string aDir = theDir;
	AddTrailingSlash(aDir);
	GetStandardFileName(aDir);

	std::string aResult;

	WIN32_FIND_DATA aFindData;
	HANDLE aFindHandle = FindFirstFile(aPath.c_str(),&aFindData); 
	if (aFindHandle == INVALID_HANDLE_VALUE)
		return aResult;

	do 
	{
		if (aFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			continue;

		std::string aName = aFindData.cFileName;
		GetStandardFileName(aName);
		if (strstr(aName.c_str(),"cached_")==aName.c_str())
			continue;

		RemoveExtensionAndUnderscores(aName);
		aName = aDir + aName;
		if (mFileSet.find(aName)==mFileSet.end())
		{
			if (!aResult.empty())
				aResult += ", ";

			aResult += aFindData.cFileName;

			if (theBatchFile)
			{
				std::string aDelPath = theBasePath;
				aDelPath += "\\";
				aDelPath += theDir;
				aDelPath += "\\";
				if (strcmp(aFindData.cFileName, ".cvsignore") != 0)
				{
					aDelPath += aFindData.cFileName;
					fprintf(theBatchFile, "del %s\n", aDelPath.c_str());
				}
					
			}
		}
	} while (FindNextFile(aFindHandle,&aFindData));

	FindClose(aFindHandle);
	return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string	ResourceManager::GetUnusedFiles(const std::string &theBasePath, FILE *theBatchFile /* = NULL */)
{
	std::string aResult;
	for (StringSet::iterator anItr = mDirSet.begin(); anItr != mDirSet.end(); ++anItr)
	{
		std::string &aDir = *anItr;
		std::string aDirResult = GetUnusedFilesForDir(theBasePath,aDir,theBatchFile);
		if (theBatchFile)
			fprintf(theBatchFile, "\n\n");

		if (!aDirResult.empty())
			aResult += aDir + ": " + aDirResult + "\r\n";
	}
	return aResult;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::Fail(const std::string& theErrorText)
{
	if (!mHasFailed)
	{
		mHasFailed = true;
		int aLineNum = mXMLParser->GetCurrentLineNum();

		char aLineNumStr[16];
		sprintf(aLineNumStr, "%d", aLineNum);	

		mError = theErrorText;

		if (aLineNum > 0)
			mError += std::string(" on Line ") + aLineNumStr;

		if (mXMLParser->GetFileName().length() > 0)
			mError += " in File '" + mXMLParser->GetFileName() + "'";
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::AddFile(const std::string &thePath)
{
	if (thePath.empty())
		return;

	std::string aPath = mDirPrefix + thePath;
	GetStandardFileName(aPath);
	mDirSet.insert(GetFileDir(aPath));

	RemoveExtensionAndUnderscores(aPath);
	if (!aPath.empty() && aPath[0]!='!') // program supplied resource
		mFileSet.insert(aPath);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseCommonResource(XMLElement &theElement, ResType theType)
{
	std::string aPath = theElement.mAttributes["path"];
	if (aPath.empty())
		return Fail("No path specified.");
	
	std::string anId;
	XMLParamMap::iterator anItr = theElement.mAttributes.find("id");
	if (anItr == theElement.mAttributes.end())
		anId = mDefaultIdPrefix + GetFileName(aPath,true);
	else
		anId = mDefaultIdPrefix + anItr->second;

	AddFile(aPath);
	if (theType==ResType_Image)
	{
		anItr = theElement.mAttributes.find("alphaimage");
		if (anItr != theElement.mAttributes.end())
			AddFile(anItr->second);

		anItr = theElement.mAttributes.find("alphagrid");
		if (anItr != theElement.mAttributes.end())
			AddFile(anItr->second);
	}



	BaseRes aRes;
	aRes.mId = anId;
	aRes.mType = theType;

	anItr = theElement.mAttributes.find("alias");
	if (anItr != theElement.mAttributes.end())
		aRes.mAlias[0] = mDefaultIdPrefix + anItr->second;

	for (int i=0; i<MAX_ALIASES; i++)
	{
		anItr = theElement.mAttributes.find(StrFormat("alias%d",i+1));
		if (anItr != theElement.mAttributes.end())
			aRes.mAlias[i] = mDefaultIdPrefix + anItr->second;
	}


	std::pair<ResMap::iterator,bool> aRet = mCurResMap->insert(ResMap::value_type(anId,aRes));
	if (!aRet.second)
		return Fail("Resource already defined.");

	mVariableList.push_back(aRes);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSetDefaults(XMLElement &theElement)
{
	XMLParamMap::iterator anItr;

	anItr = theElement.mAttributes.find("idprefix");
	if (anItr != theElement.mAttributes.end())
		mDefaultIdPrefix = RemoveTrailingSlash(anItr->second);	

	std::string aPath = theElement.mAttributes["path"];
	GetStandardFileName(aPath);
	mDirPrefix = aPath;
	AddTrailingSlash(mDirPrefix);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResources()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "Image")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Image))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "Sound")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Sound))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "Font")
			{
				if (!ParseCommonResource(aXMLElement,ResType_Font))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == "SetDefaults")
			{
				if (!ParseSetDefaults(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");		
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}		
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoParseResources()
{
	if (!mXMLParser->HasFailed())
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == "Resources")
				{
					std::string aResGroup = aXMLElement.mAttributes["id"];
					if (aResGroup.empty())
						return Fail("No id specified.");

					mCurResMap = &mResGroupMap[aResGroup];

					if (!ParseResources())
						break;
				}
				else 
				{
					Fail("Invalid Section '" + aXMLElement.mValue + "'");
					break;
				}
			}
			else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
			{
				Fail("Element Not Expected '" + aXMLElement.mValue + "'");
				break;
			}
		}
	}

	if (mXMLParser->HasFailed())
		Fail(mXMLParser->GetErrorText());	

	delete mXMLParser;
	mXMLParser = NULL;

	return !mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResourcesFile(const std::string& theFilename)
{
	mXMLParser = new XMLParser();
	mXMLParser->OpenFile(theFilename);

	XMLElement aXMLElement;
	while (true)
	{
		if (!mXMLParser->NextElement(&aXMLElement))
			Fail(mXMLParser->GetErrorText());

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue != "ResourceManifest")
				break;
			else
				return DoParseResources();
		}
	}
		
	Fail("Expecting ResourceManifest tag");

	return DoParseResources();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string ResourceManager::GetResName(const BaseRes &theRes)
{
	return theRes.mId;
/*	switch(theRes.mType)
	{
		case ResType_Image: return "IMAGE_" + theRes.mId;
		case ResType_Sound: return "SOUND_" + theRes.mId;
		case ResType_Font: return "FONT_" + theRes.mId;
	}

	return "";*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string	ResourceManager::GetTypeName(const BaseRes &theRes)
{
	switch(theRes.mType)
	{
		case ResType_Image: return "Image*"; 
		case ResType_Sound: return "int"; 
		case ResType_Font: return "Font*"; 
	}

	return "";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteSourceFileGroup(FILE *theFile, const std::string &theResGroup, ResMap &theMap)
{
	fprintf(theFile,"bool Sexy::%sExtract%sResources(ResourceManager *theManager)\n",mFunctionPrefix.c_str(),theResGroup.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\tgNeedRecalcVariableToIdMap = true;\n\n");
	fprintf(theFile,"\tResourceManager &aMgr = *theManager;\n");
	fprintf(theFile,"\ttry\n");
	fprintf(theFile,"\t{\n");

	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		const std::string &anId = anItr->first;
		BaseRes &aRes = anItr->second;

		std::string aVarName = GetResName(aRes);
		std::string aMethodName;
		switch(aRes.mType)
		{
			case ResType_Image: aMethodName = "GetImageThrow"; break;
			case ResType_Sound: aMethodName = "GetSoundThrow"; break;
			case ResType_Font: aMethodName = "GetFontThrow"; break;
		}

		fprintf(theFile,"\t\t%s = aMgr.%s(\"%s\");\n",aVarName.c_str(),aMethodName.c_str(),anId.c_str());
	}

	fprintf(theFile,"\t}\n");
	fprintf(theFile,"\tcatch(ResourceManagerException&)\n");
	fprintf(theFile,"\t{\n");
//	fprintf(theFile,"\t\tif (exitOnFailure)\n\t\t{\n");
//	fprintf(theFile,"\t\t\tMessageBox(NULL,ex.what.c_str(),\"Resource Error\",MB_OK);\n");
//	fprintf(theFile,"\t\t\texit(0);\n");
//	fprintf(theFile,"\t\t}\n");
	fprintf(theFile,"\t\treturn false;\n");
	fprintf(theFile,"\t}\n");
	fprintf(theFile,"\treturn true;\n");
	fprintf(theFile,"}\n\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteSourceFileVariables(FILE *theFile, const std::string &theResGroup, ResMap &theMap)
{
	fprintf(theFile,"// %s Resources\n",theResGroup.c_str());
	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		BaseRes &aRes = anItr->second;
		std::string aVarName = GetResName(aRes);
		std::string aTypeName = GetTypeName(aRes);

		fprintf(theFile,"%s Sexy::%s;\n",aTypeName.c_str(), aVarName.c_str());
	}
	fprintf(theFile,"\n");
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteHeaderFileGroup(FILE *theFile, const std::string &theResGroup, ResMap &theMap)
{
	fprintf(theFile,"\t// %s Resources\n",theResGroup.c_str());
	fprintf(theFile,"\tbool %sExtract%sResources(ResourceManager *theMgr);\n",mFunctionPrefix.c_str(),theResGroup.c_str());

	StringSet aSet;

	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		BaseRes &aRes = anItr->second;
		std::string aVarName = GetResName(aRes);
		std::string aTypeName = GetTypeName(aRes);

		aSet.insert(StrFormat("\textern %s %s;\n",aTypeName.c_str(), aVarName.c_str()));
	}

	for (StringSet::iterator aStrItr = aSet.begin(); aStrItr!= aSet.end(); ++aStrItr)
		fprintf(theFile,"%s",aStrItr->c_str());

	fprintf(theFile,"\n");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteSourceFile(FILE *theFile, const std::string &theFileName)
{
	fprintf(theFile,"#include \"%s.h\"\n",theFileName.c_str());
	fprintf(theFile,"#include \"SexyAppFramework/ResourceManager.h\"\n");
	fprintf(theFile,"\nusing namespace Sexy;\n\n");

	fprintf(theFile,"#pragma warning(disable:4311 4312)\n\n");
	fprintf(theFile,"static bool gNeedRecalcVariableToIdMap = false;\n\n");

	// ExtractResourcesByName
	fprintf(theFile,"bool Sexy::%sExtractResourcesByName(ResourceManager *theManager, const char *theName)\n{\n",mFunctionPrefix.c_str());
	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
		fprintf(theFile,"\tif (strcmp(theName,\"%s\")==0) return %sExtract%sResources(theManager);\n",anItr->first.c_str(),mFunctionPrefix.c_str(),anItr->first.c_str());
	fprintf(theFile,"\treturn false;\n}\n\n");

	// GetIdByStringId
	fprintf(theFile,"Sexy::%sResourceId Sexy::%sGetIdByStringId(const char *theStringId)\n{\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"\ttypedef std::map<std::string,int> MyMap;\n");
	fprintf(theFile,"\tstatic MyMap aMap;\n");
	fprintf(theFile,"\tif(aMap.empty())\n\t{\n");
	fprintf(theFile,"\t\tfor(int i=0; i<%sRESOURCE_ID_MAX; i++)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\t\t\taMap[%sGetStringIdById(i)] = i;\n\t}\n\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tMyMap::iterator anItr = aMap.find(theStringId);\n");
	fprintf(theFile,"\tif (anItr == aMap.end())\n");
	fprintf(theFile,"\t\treturn %sRESOURCE_ID_MAX;\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\telse\n");
	fprintf(theFile,"\t\treturn (%sResourceId) anItr->second;\n",mFunctionPrefix.c_str());
	fprintf(theFile,"}\n\n");
	

	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
	{
		WriteSourceFileVariables(theFile,anItr->first,anItr->second);
		WriteSourceFileGroup(theFile,anItr->first,anItr->second);
	}

	fprintf(theFile,"static void* gResources[] =\n");
	fprintf(theFile,"{\n");

	ResList::iterator aVarItr;
	for (aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
		fprintf(theFile,"\t&%s,\n",aVarItr->mId.c_str());

	fprintf(theFile,"\tNULL\n");
	fprintf(theFile,"};\n\n");

	// LoadImageById
	fprintf(theFile,"Image* Sexy::%sLoadImageById(ResourceManager *theManager, int theId)\n{\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\treturn (*((Image**)gResources[theId]) = theManager->LoadImage(%sGetStringIdById(theId)));\n}\n\n", mFunctionPrefix.c_str());

	// ReplaceImageById
	fprintf(theFile,"void Sexy::%sReplaceImageById(ResourceManager *theManager, int theId, Image *theImage)\n{\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\ttheManager->ReplaceImage(%sGetStringIdById(theId),theImage);\n", mFunctionPrefix.c_str());
	fprintf(theFile,"\t*(Image**)gResources[theId] = theImage;\n}\n\n", mFunctionPrefix.c_str());

	// GetImageById
	fprintf(theFile,"Image* Sexy::%sGetImageById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(Image**)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// GetFontById
	fprintf(theFile,"Font* Sexy::%sGetFontById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(Font**)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// GetSoundById
	fprintf(theFile,"int Sexy::%sGetSoundById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(int*)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// GetImageRefById
	fprintf(theFile,"Image*& Sexy::%sGetImageRefById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(Image**)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// SetFontRefById
	fprintf(theFile,"Font*& Sexy::%sGetFontRefById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(Font**)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// SetSoundRefById
	fprintf(theFile,"int& Sexy::%sGetSoundRefById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn *(int*)gResources[theId];\n");
	fprintf(theFile,"}\n\n");

	// GetIdByVariable
	fprintf(theFile,"static Sexy::%sResourceId %sGetIdByVariable(const void *theVariable)\n{\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"\ttypedef std::map<int,int> MyMap;\n");
	fprintf(theFile,"\tstatic MyMap aMap;\n");
	fprintf(theFile,"\tif(gNeedRecalcVariableToIdMap)\n\t{\n");
	fprintf(theFile,"\t\tgNeedRecalcVariableToIdMap = false;\n");
	fprintf(theFile,"\t\taMap.clear();\n");
	fprintf(theFile,"\t\tfor(int i=0; i<%sRESOURCE_ID_MAX; i++)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\t\t\taMap[*(int*)gResources[i]] = i;\n\t}\n\n");
	fprintf(theFile,"\tMyMap::iterator anItr = aMap.find((int)theVariable);\n");
	fprintf(theFile,"\tif (anItr == aMap.end())\n");
	fprintf(theFile,"\t\treturn %sRESOURCE_ID_MAX;\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\telse\n");
	fprintf(theFile,"\t\treturn (%sResourceId) anItr->second;\n",mFunctionPrefix.c_str());
	fprintf(theFile,"}\n\n");

	// GetIdByImage
	fprintf(theFile,"Sexy::%sResourceId Sexy::%sGetIdByImage(Image *theImage)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn %sGetIdByVariable(theImage);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"}\n\n");

	// GetIdByFont
	fprintf(theFile,"Sexy::%sResourceId Sexy::%sGetIdByFont(Font *theFont)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn %sGetIdByVariable(theFont);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"}\n\n");

	// GetIdBySound
	fprintf(theFile,"Sexy::%sResourceId Sexy::%sGetIdBySound(int theSound)\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\treturn %sGetIdByVariable((void*)theSound);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"}\n\n");

	// GetStringIdById
	fprintf(theFile,"const char* Sexy::%sGetStringIdById(int theId)\n",mFunctionPrefix.c_str());
	fprintf(theFile,"{\n");
	fprintf(theFile,"\tswitch(theId)\n");
	fprintf(theFile,"\t{\n");
	for (aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
		fprintf(theFile,"\t\tcase %s_ID: return \"%s\";\n",aVarItr->mId.c_str(),aVarItr->mId.c_str());
	fprintf(theFile,"\t\tdefault: return \"\";\n");
	fprintf(theFile,"\t}\n");
	fprintf(theFile,"}\n");
	fprintf(theFile,"\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::WriteHeaderFile(FILE *theFile, const std::string &theName)
{
	fprintf(theFile,"#ifndef __%s_H__\n",theName.c_str());
	fprintf(theFile,"#define __%s_H__\n",theName.c_str());
	fprintf(theFile,"\n");
	fprintf(theFile,"namespace Sexy\n");
	fprintf(theFile,"{\n");
	fprintf(theFile,"\tclass ResourceManager;\n");
	fprintf(theFile,"\tclass Image;\n");
	fprintf(theFile,"\tclass Font;\n\n");

	fprintf(theFile,"\tImage* %sLoadImageById(ResourceManager *theManager, int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tvoid %sReplaceImageById(ResourceManager *theManager, int theId, Image *theImage);\n",mFunctionPrefix.c_str());

	fprintf(theFile,"\tbool %sExtractResourcesByName(ResourceManager *theManager, const char *theName);\n\n",mFunctionPrefix.c_str());

	for (ResGroupMap::iterator anItr = mResGroupMap.begin(); anItr != mResGroupMap.end(); ++anItr)
		WriteHeaderFileGroup(theFile,anItr->first,anItr->second);

	fprintf(theFile,"\tenum %sResourceId\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\t{\n");
	for (ResList::iterator aVarItr = mVariableList.begin(); aVarItr != mVariableList.end(); ++aVarItr)
	{
		fprintf(theFile,"\t\t%s_ID,\n",aVarItr->mId.c_str());
		for (int i=0; i<MAX_ALIASES; i++)
		{
			if (!aVarItr->mAlias[i].empty())
				fprintf(theFile,"\t\t%s_ID = %s_ID,\n",aVarItr->mAlias[i].c_str(),aVarItr->mId.c_str());
		}
	}

	fprintf(theFile,"\t\t%sRESOURCE_ID_MAX\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\t};\n\n");
	fprintf(theFile,"\tImage* %sGetImageById(int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tFont* %sGetFontById(int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tint %sGetSoundById(int theId);\n\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tImage*& %sGetImageRefById(int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tFont*& %sGetFontRefById(int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\tint& %sGetSoundRefById(int theId);\n\n",mFunctionPrefix.c_str());

	fprintf(theFile,"\t%sResourceId %sGetIdByImage(Image *theImage);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"\t%sResourceId %sGetIdByFont(Font *theFont);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());
	fprintf(theFile,"\t%sResourceId %sGetIdBySound(int theSound);\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());

	fprintf(theFile,"\tconst char* %sGetStringIdById(int theId);\n",mFunctionPrefix.c_str());
	fprintf(theFile,"\t%sResourceId %sGetIdByStringId(const char *theStringId);\n\n",mFunctionPrefix.c_str(),mFunctionPrefix.c_str());

	fprintf(theFile,"} // namespace Sexy\n");
	fprintf(theFile,"\n\n#endif\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::WriteSourceCode(const std::string &theFilename, const std::string &thePrefix)
{
	mFunctionPrefix = thePrefix;

	std::string aName = GetFileName(theFilename,true);
	std::string aPath = GetFileDir(theFilename);
	std::string aSourceName = aName + ".cpp";
	std::string aHeaderName = aName + ".h";

	FILE *aSource = fopen((aPath + "/" + aSourceName).c_str(),"w");
	if (aSource==NULL)
	{
		mError = "Failed to open source file.";
		return false;
	}

	FILE *aHeader = fopen((aPath + "/" + aHeaderName).c_str(),"w");
	if (aHeader==NULL)
	{
		mError = "Failed to open header file.";
		fclose(aSource);
		return false;
	}

	WriteHeaderFile(aHeader,aName);
	WriteSourceFile(aSource,aName);
	


	fclose(aSource);
	fclose(aHeader);

	return true;

}
