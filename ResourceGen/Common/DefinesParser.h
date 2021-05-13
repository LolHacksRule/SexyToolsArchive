#ifndef __DEFINESPARSER_H__
#define __DEFINESPARSER_H__

#include "XMLParser.h"

namespace Sexy
{

typedef std::map<std::string, std::string> DefinesMap;

class DefinesParser
{
public:
	XMLParser*				mXMLParser;	
	bool					mHasFailed;
	std::string				mError;
	DefinesMap				mDefinesMap;
	
protected:
	void					Fail(const std::string& theErrorText);

	bool					ParseSingleElement(std::string* aString);

	bool					ParseSectionDefinesDefine();
	bool					ParseSectionDefines();
	bool					ParseXML();

public:
	DefinesParser();

	void					Clear();

	bool					ReadFromString(const std::string& theString);

	bool					ReadFromFile(const std::string& theFileName);	
	bool					WriteToFile(const std::string& theFileName);

	std::string				GetValue(const std::string& theName);
	void					SetValue(const std::string& theName, const std::string& theValue);

	std::string				Evaluate(const std::string& theString);

	std::string				GetErrorText();
};

};

#endif //__DEFINESPARSER_H__