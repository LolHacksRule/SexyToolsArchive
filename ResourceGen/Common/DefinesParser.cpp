#include "DefinesParser.h"
#include <fstream.h>

using namespace Sexy;

DefinesParser::DefinesParser()
{
	mHasFailed = false;
}

void DefinesParser::Clear()
{
	mDefinesMap.clear();
}

bool DefinesParser::ParseSingleElement(std::string* aString)
{
	*aString = "";

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			Fail("Unexpected Section: " + aXMLElement.mValue);
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

bool DefinesParser::ParseSectionDefinesDefine()
{
	std::string aName;
	std::string aValue;

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "Name")
			{
				if (!ParseSingleElement(&aName))
					return false;
			}
			else if (aXMLElement.mValue == "Value")
			{
				if (!ParseSingleElement(&aValue))
					return false;
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
			if (aName.length() > 0)
				SetValue(aName, aValue);

			return true;
		}
	}	
}

bool DefinesParser::ParseSectionDefines()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "Define")
			{
				if (!ParseSectionDefinesDefine())
					return false;
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

void DefinesParser::Fail(const std::string& theErrorText)
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

bool DefinesParser::ParseXML()
{	
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			break;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "Defines")
			{
				if (!ParseSectionDefines())
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

	if (mXMLParser->HasFailed())	
		Fail(mXMLParser->GetErrorText());		

	delete mXMLParser;
	mXMLParser = NULL;

	return !mHasFailed;
}

bool DefinesParser::ReadFromString(const std::string& theString)
{
	mXMLParser = new XMLParser();
	mXMLParser->SetStringSource(theString);
	return ParseXML();
}

bool DefinesParser::ReadFromFile(const std::string& theFileName)
{
	mXMLParser = new XMLParser();
	mXMLParser->OpenFile(theFileName);
	return ParseXML();	
}

bool DefinesParser::WriteToFile(const std::string& theFileName)
{
	fstream aXMLStream(theFileName.c_str(), ios::out);

	if (!aXMLStream.is_open())
	{
		Fail("Unable to create file");
		return false;
	}

	aXMLStream << "<Defines>" << endl;

	DefinesMap::iterator anItr = mDefinesMap.begin();
	while (anItr != mDefinesMap.end())
	{
		aXMLStream << "\t<Define>" << endl;
		aXMLStream << "\t\t<Name>" << XMLEncodeString(anItr->first).c_str() << "</Name>" << endl;
		aXMLStream << "\t\t<Value>" << XMLEncodeString(anItr->second).c_str() << "</Value>" << endl;
		aXMLStream << "\t</Define>" << endl;
		++anItr;
	}

	aXMLStream << "</Defines>" << endl;

	return true;
}

std::string DefinesParser::GetValue(const std::string& theName)
{
	DefinesMap::iterator anItr = mDefinesMap.find(theName);
	
	if (anItr == mDefinesMap.end())
		return "";
	else
		return anItr->second;
}

void DefinesParser::SetValue(const std::string& theName, const std::string& theValue)
{
	mDefinesMap[theName] = theValue;	
}

std::string DefinesParser::Evaluate(const std::string& theString)
{
	std::string anEvaluatedString = theString;

	for (;;)
	{
		int aPercentPos = anEvaluatedString.find('%');

		if (aPercentPos == -1)
			break;
		
		int aSecondPercentPos = anEvaluatedString.find('%', aPercentPos + 1);
		if (aSecondPercentPos == -1)
			break;

		std::string aName = anEvaluatedString.substr(aPercentPos + 1, aSecondPercentPos - aPercentPos - 1);
		std::string aValue = GetValue(aName);
		
		//if (aValue.length() == 0)
		//	aValue = "(null)";

		anEvaluatedString.erase(anEvaluatedString.begin() + aPercentPos, anEvaluatedString.begin() + aSecondPercentPos + 1);
		anEvaluatedString.insert(anEvaluatedString.begin() + aPercentPos, aValue.begin(), aValue.begin() + aValue.length());
	}

	return anEvaluatedString;
}

std::string DefinesParser::GetErrorText()
{
	return mError;
}