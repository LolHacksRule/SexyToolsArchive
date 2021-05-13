#include "PropertiesParser.h"
#include "XMLParser.h"
#include <assert.h>

using namespace Sexy;

PropertiesParser::PropertiesParser()
{	
	mHasFailed = false;
	mXMLParser = NULL;
}

void PropertiesParser::Fail(const std::string& theErrorText)
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
}


PropertiesParser::~PropertiesParser()
{
}

bool PropertiesParser::ParseSingleElement(std::string* aString)
{
	*aString = "";

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			Fail("Unexpected Section: '" + aXMLElement.mValue + "'");
			return false;			
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			*aString = aXMLElement.mValue;
		}		
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool PropertiesParser::ParseStringArray(StringVector* theStringVector)
{
	theStringVector->clear();

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				std::string aString;

				if (!ParseSingleElement(&aString))
					return false;

				theStringVector->push_back(aString);
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

bool PropertiesParser::ParseProperties()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				std::string aDef;

				if (!ParseSingleElement(&aDef))
					return false;

				std::string anId = aXMLElement.mAttributes["id"];

				mStringProperties.insert(StringStringMap::value_type(anId, aDef));
			}
			else if (aXMLElement.mValue == "StringArray")
			{
				StringVector aDef;

				if (!ParseStringArray(&aDef))
					return false;

				std::string anId = aXMLElement.mAttributes["id"];

				mStringVectorProperties.insert(StringStringVectorMap::value_type(anId, aDef));
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

bool PropertiesParser::ParsePropertiesFile(const std::string& theFilename)
{
	mXMLParser = new XMLParser();

	if (mXMLParser->OpenFile(theFilename))
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == "Properties")
				{
					if (!ParseProperties())
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

std::string PropertiesParser::GetErrorText()
{
	return mError;
}

std::string PropertiesParser::GetString(const std::string& theId)
{
	StringStringMap::iterator anItr = mStringProperties.find(theId);
	assert(anItr != mStringProperties.end());
	
	if (anItr != mStringProperties.end())	
		return anItr->second;
	else
		return "";	
}

StringVector PropertiesParser::GetStringVector(const std::string& theId)
{
	StringStringVectorMap::iterator anItr = mStringVectorProperties.find(theId);
	assert(anItr != mStringVectorProperties.end());
	
	if (anItr != mStringVectorProperties.end())	
		return anItr->second;
	else
		return StringVector();
}