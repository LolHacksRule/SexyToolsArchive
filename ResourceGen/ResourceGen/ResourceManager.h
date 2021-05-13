#ifndef __SEXY_RESOURCEMANAGER_H__
#define __SEXY_RESOURCEMANAGER_H__
#include <string>
#include <map>
#include <set>
#include <list>
#include <stdio.h>

namespace Sexy
{

class XMLParser;
class XMLElement;

const int MAX_ALIASES=10;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ResourceManager
{
protected:
	enum ResType
	{
		ResType_Image,
		ResType_Sound,
		ResType_Font
	};

	struct BaseRes
	{
		ResType mType;
		std::string mId;
		std::string mAlias[MAX_ALIASES];
	};

	typedef std::map<std::string,BaseRes> ResMap;
	typedef std::map<std::string,ResMap> ResGroupMap;
	typedef std::set<std::string> StringSet;
	typedef std::set<std::string> StringList;
	typedef std::list<BaseRes> ResList;


	ResMap					*mCurResMap;
	ResGroupMap				mResGroupMap;
	std::string				mDefaultIdPrefix;
	ResList					mVariableList;
	std::string				mFunctionPrefix;
	std::string				mDirPrefix;
	StringSet				mDirSet;
	StringSet				mFileSet;

	XMLParser*				mXMLParser;
	std::string				mError;
	bool					mHasFailed;

	bool					Fail(const std::string& theErrorText);

	void					AddFile(const std::string &thePath);
	bool					ParseCommonResource(XMLElement &theElement, ResType theType);
	bool					ParseSetDefaults(XMLElement &theElement);
	bool					ParseResources();
	bool					DoParseResources();

	std::string				GetResName(const BaseRes &theRes);
	std::string				GetTypeName(const BaseRes &theRes);
	void					WriteSourceFileVariables(FILE *theFile, const std::string &theResGroup, ResMap &theMap);
	void					WriteSourceFileGroup(FILE *theFile, const std::string &theResGroup, ResMap &theMap);
	void					WriteHeaderFileGroup(FILE *theFile, const std::string &theResGroup, ResMap &theMap);
	void					WriteSourceFile(FILE *theFile, const std::string &theFileName);
	void					WriteHeaderFile(FILE *theFile, const std::string &theFileName);

public:
	ResourceManager();
	virtual ~ResourceManager();

	bool					ParseResourcesFile(const std::string& theFilename);
	bool					WriteSourceCode(const std::string &theFilename, const std::string &thePrefix);
	std::string				GetErrorText();
	bool					HadError();
	
	std::string				GetUnusedFilesForDir(const std::string &theBasePath, const std::string &theDir, FILE *theBatchFile = NULL);
	std::string				GetUnusedFiles(const std::string &theBasePath, FILE *theBatchFile = NULL);
};

}

#endif //__PROPERTIESPARSER_H__


