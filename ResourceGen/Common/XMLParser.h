#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__

#include "Common.h"

#include "PerfTimer.h"

namespace Sexy
{

class XMLParam
{
public:
	std::string				mKey;
	std::string				mValue;
};

typedef std::map<std::string, std::string>	XMLParamMap;
typedef std::list<XMLParamMap::iterator>	XMLParamMapIteratorList;
typedef std::vector<char> CharVector;

class XMLElement
{
public:
	enum
	{
		TYPE_NONE,
		TYPE_START,
		TYPE_END,
		TYPE_ELEMENT,
		TYPE_INSTRUCTION,
		TYPE_COMMENT
	};
public:
	
	int						mType;
	std::string				mSection;
	std::string				mValue;
	std::string				mInstruction;
	XMLParamMap				mAttributes;
	XMLParamMapIteratorList	mAttributeIteratorList; // stores attribute iterators in their original order
};

class XMLParser
{
protected:
	std::string				mFileName;
	std::string				mErrorText;
	int						mLineNum;
	FILE*					mFile;
	bool					mHasFailed;
	bool					mAllowComments;
	CharVector				mBufferedText;

	std::string				mSection;

protected:
	void					Fail(const std::string& theErrorText);
	void					Init();

	bool					AddAttribute(XMLElement* theElement, const std::string& aAttributeKey, const std::string& aAttributeValue);

public:
	XMLParser();
	virtual ~XMLParser();

	bool					OpenFile(const std::string& theFilename);
	void					SetStringSource(const std::string& theString);
	bool					NextElement(XMLElement* theElement);
	std::string				GetErrorText();
	int						GetCurrentLineNum();
	std::string				GetFileName();

	inline void				AllowComments(bool doAllow) { mAllowComments = doAllow; }

	bool					HasFailed();
	bool					EndOfFile();
};

};

#endif //__XMLPARSER_H__
