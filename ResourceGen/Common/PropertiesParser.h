#ifndef __PROPERTIESPARSER_H__
#define __PROPERTIESPARSER_H__

#include "Common.h"

namespace Sexy
{

typedef std::vector<std::string> StringVector;

typedef std::map<std::string, std::string> StringStringMap;
typedef std::map<std::string, StringVector> StringStringVectorMap;

class XMLParser;

class PropertiesParser
{
public:	
	XMLParser*				mXMLParser;
	std::string				mError;
	bool					mHasFailed;

	StringStringMap			mStringProperties;
	StringStringVectorMap	mStringVectorProperties;

protected:
	void					Fail(const std::string& theErrorText);

	bool					ParseSingleElement(std::string* theString);
	bool					ParseStringArray(StringVector* theStringVector);
	bool					ParseProperties();

public:
	PropertiesParser();
	virtual ~PropertiesParser();

	bool					ParsePropertiesFile(const std::string& theFilename);
	std::string				GetErrorText();

	std::string				GetString(const std::string& theId);
	StringVector			GetStringVector(const std::string& theId);
};

}

#endif //__PROPERTIESPARSER_H__


