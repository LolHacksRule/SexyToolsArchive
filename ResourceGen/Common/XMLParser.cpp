#include "XMLParser.h"

using namespace Sexy;

XMLParser::XMLParser()
{
	mFile = NULL;
	mLineNum = 0;
	mAllowComments = false;
}

XMLParser::~XMLParser()
{
	if (mFile != NULL)
		fclose(mFile);
}

void XMLParser::Fail(const std::string& theErrorText)
{
	mHasFailed = true;
	mErrorText = theErrorText;
}

void XMLParser::Init()
{
	mSection = "";
	mLineNum = 1;
	mHasFailed = false;
	mErrorText = "";	
}

bool XMLParser::AddAttribute(XMLElement* theElement, const std::string& theAttributeKey, const std::string& theAttributeValue)
{
	std::pair<XMLParamMap::iterator,bool> aRet;

	aRet = theElement->mAttributes.insert(XMLParamMap::value_type(theAttributeKey, theAttributeValue));
	if (!aRet.second)
		aRet.first->second = theAttributeValue;

	if (theAttributeKey != "/")
		theElement->mAttributeIteratorList.push_back(aRet.first);

	return aRet.second;
}

bool XMLParser::OpenFile(const std::string& theFileName)
{		
	mFile = fopen(theFileName.c_str(), "r");

	if (mFile == NULL)
	{
		mLineNum = 0;
		Fail("Unable to open file " + theFileName);		
		return false;
	}

	mFileName = theFileName.c_str();
	Init();
	return true;
}

void XMLParser::SetStringSource(const std::string& theString)
{
	Init();

	int aSize = theString.size();

	mBufferedText.resize(aSize);	
	for (int i = 0; i < aSize; i++)
		mBufferedText[i] = theString[aSize - i - 1];	
}

bool XMLParser::NextElement(XMLElement* theElement)
{
	for (;;)
	{		
		theElement->mType = XMLElement::TYPE_NONE;
		theElement->mSection = mSection;
		theElement->mValue = "";
		theElement->mAttributes.clear();			
		theElement->mInstruction.erase();

		bool hasSpace = false;	
		bool inQuote = false;
		bool gotEndQuote = false;

		bool doingAttribute = false;
		bool AttributeVal = false;
		std::string aAttributeKey;
		std::string aAttributeValue;

		std::string aLastAttributeKey;
		
		for (;;)
		{
			// Process character by character

			char c;
			int aVal;
			
			if (mBufferedText.size() > 0)
			{								
				c = mBufferedText[mBufferedText.size()-1];
				mBufferedText.pop_back();				

				aVal = 1;
			}
			else
			{
				if (mFile != NULL)
					aVal = fread(&c, 1, 1, mFile);
				else
					aVal = 0;
			}
			
			if (aVal == 1)
			{
				bool processChar = false;

				if (c == '\n')
				{
					mLineNum++;
				}

				if (theElement->mType == XMLElement::TYPE_COMMENT)
				{
					// Just add text to theElement->mInstruction until we find -->

					std::string* aStrPtr = &theElement->mInstruction;
					
					*aStrPtr += c;					

					int aLen = aStrPtr->length();

					if ((c == '>') && (aLen >= 3) && ((*aStrPtr)[aLen - 2] == '-') && ((*aStrPtr)[aLen - 3] == '-'))
					{
						*aStrPtr = aStrPtr->substr(0, aLen - 3);
						break;
					}
				}
				else if (theElement->mType == XMLElement::TYPE_INSTRUCTION)
				{
					// Just add text to theElement->mInstruction until we find ?>

					std::string* aStrPtr = &theElement->mValue;

					if ((theElement->mInstruction.length() != 0) || (::isspace(c)))
						aStrPtr = &theElement->mInstruction;
					
					*aStrPtr += c;					

					int aLen = aStrPtr->length();

					if ((c == '>') && (aLen >= 2) && ((*aStrPtr)[aLen - 2] == '?'))
					{
						*aStrPtr = aStrPtr->substr(0, aLen - 2);
						break;
					}
				}
				else
				{
					if (c == '"')
					{
						inQuote = !inQuote;
						if (theElement->mType==XMLElement::TYPE_NONE || theElement->mType==XMLElement::TYPE_ELEMENT)
							processChar = true;

						if (!inQuote)
							gotEndQuote = true;
					}
					else if (!inQuote)
					{
						if (c == '<')
						{
							if (theElement->mType == XMLElement::TYPE_ELEMENT)
							{
								//TODO: Fix buffered text.  Not sure what I meant by that.

								//OLD: mBufferedText = c + mBufferedText;

								mBufferedText.push_back(c);								
								break;
							}

							if (theElement->mType == XMLElement::TYPE_NONE)
							{
								theElement->mType = XMLElement::TYPE_START;
							}
							else
							{
								Fail("Unexpected '<'");
								return false;
							}
						}
						else if (c == '>')
						{
							if (theElement->mType == XMLElement::TYPE_START)
							{	
								bool insertEnd = false;

								if (aAttributeKey == "/")
								{
									// We will get this if we have a space before the />, so we can ignore it
									//  and go about our business now
									insertEnd = true;
								}
								else
								{
									// Probably isn't committed yet
									if (aAttributeKey.length() > 0)
									{
										aLastAttributeKey = aAttributeKey;
//										theElement->mAttributes[aLastAttributeKey] = aAttributeValue;

										AddAttribute(theElement, aLastAttributeKey, aAttributeValue);

										aAttributeKey = "";
										aAttributeValue = "";
									}

									if (aLastAttributeKey.length() > 0)
									{
										std::string aVal = theElement->mAttributes[aLastAttributeKey];

										int aLen = aVal.length();

										if ((aLen > 0) && (aVal[aLen-1] == '/'))
										{
											// Its an empty element, fake start and end segments
//											theElement->mAttributes[aLastAttributeKey] = aVal.substr(0, aLen - 1);

											AddAttribute(theElement, aLastAttributeKey, aVal.substr(0, aLen - 1));

											insertEnd = true;
										}
									}
									else
									{
										int aLen = theElement->mValue.length();

										if ((aLen > 0) && (theElement->mValue[aLen-1] == '/'))
										{
											// Its an empty element, fake start and end segments
											theElement->mValue = theElement->mValue.substr(0, aLen - 1);
											insertEnd = true;
										}
									}
								}

								// Do we want to fake an ending section?
								if (insertEnd)
								{									
									std::string anAddString = "</" + theElement->mValue + ">";

									int anOldSize = mBufferedText.size();
									int anAddLength = anAddString.length();

									mBufferedText.resize(anOldSize + anAddLength);

									for (int i = 0; i < anAddLength; i++)
										mBufferedText[anOldSize + i] = anAddString[anAddLength - i - 1];																											

									//OLD: mBufferedText = "</" + theElement->mValue + ">" + mBufferedText;
								}

								if (mSection.length() != 0)
									mSection += "/";

								mSection += theElement->mValue;								

								break;
							}
							else if (theElement->mType == XMLElement::TYPE_END)
							{
								int aLastSlash = mSection.rfind('/');
								if ((aLastSlash == -1) && (mSection.length() == 0))
								{
									Fail("Unexpected End");
									return false;
								}

								std::string aLastSectionName = mSection.substr(aLastSlash + 1);
								
								if (aLastSectionName != theElement->mValue)
								{
									Fail("End '" + theElement->mValue + "' Doesn't Match Start '" + aLastSectionName + "'");
									return false;
								}

								if (aLastSlash == -1)
									mSection.erase(mSection.begin(), mSection.end());
								else
									mSection.erase(mSection.begin() + aLastSlash, mSection.end());

								break;
							}
							else
							{
								Fail("Unexpected '>'");
								return false;
							}
						}
						else if ((c == '/') && (theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == ""))
						{					
							theElement->mType = XMLElement::TYPE_END;					
						}				
						else if ((c == '?') && (theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == ""))
						{
							theElement->mType = XMLElement::TYPE_INSTRUCTION;
						}
						else if (::isspace((uchar) c))
						{
							if (theElement->mValue != "")
								hasSpace = true;

							// It's a comment!
							if ((theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == "!--"))
								theElement->mType = XMLElement::TYPE_COMMENT;
						}
						else //if ((uchar) c > 32)
						{
							processChar = true;
						}
						/*else
						{
							Fail("Illegal Character");
							return false;
						}*/
					} 
					else
					{
						processChar = true;
					}

					if (processChar)
					{
						if (theElement->mType == XMLElement::TYPE_NONE)
							theElement->mType = XMLElement::TYPE_ELEMENT;

						if (theElement->mType == XMLElement::TYPE_START)
						{
							if (hasSpace)
							{
								if ((!doingAttribute) || ((!AttributeVal) && (c != '=')) ||
									((AttributeVal) && ((aAttributeValue.length() > 0) || gotEndQuote)))
								{
									if (doingAttribute)
									{
										aAttributeKey = XMLDecodeString(aAttributeKey);
										aAttributeValue = XMLDecodeString(aAttributeValue);

//										theElement->mAttributes[aAttributeKey] = aAttributeValue;

										AddAttribute(theElement, aAttributeKey, aAttributeValue);

										aAttributeKey = "";
										aAttributeValue = "";

										aLastAttributeKey = aAttributeKey;
									}
									else
									{
										doingAttribute = true;
									}
																
									AttributeVal = false;
								}

								hasSpace = false;
							}

							std::string* aStrPtr = NULL;

							if (!doingAttribute)
							{
								aStrPtr = &theElement->mValue;
							}
							else
							{
								if (c == '=')
								{
									AttributeVal = true;
									gotEndQuote = false;
								}
								else
								{
									if (!AttributeVal)
										aStrPtr = &aAttributeKey;
									else
										aStrPtr = &aAttributeValue;
								}
							}

							if (aStrPtr != NULL)
							{								
								*aStrPtr += c;						
							}
						}
						else
						{
							if (hasSpace)
							{
								theElement->mValue += " ";
								hasSpace = false;
							}
							
							theElement->mValue += c;
						}
					}
				}
			}
			else
			{
				if (theElement->mType != XMLElement::TYPE_NONE)
					Fail("Unexpected End of File");
					
				return false;
			}			
		}		

		if (aAttributeKey.length() > 0)
		{
			aAttributeKey = XMLDecodeString(aAttributeKey);
			aAttributeValue = XMLDecodeString(aAttributeValue);
//			theElement->mAttributes[aAttributeKey] = aAttributeValue;

			AddAttribute(theElement, aAttributeKey, aAttributeValue);
		}

		theElement->mValue = XMLDecodeString(theElement->mValue);				

		// Ignore comments
		if ((theElement->mType != XMLElement::TYPE_COMMENT) || mAllowComments)
			return true;
	}
}

bool XMLParser::HasFailed()
{
	return mHasFailed;
}

std::string XMLParser::GetErrorText()
{
	return mErrorText;
}

int XMLParser::GetCurrentLineNum()
{
	return mLineNum;
}

std::string XMLParser::GetFileName()
{
	return mFileName;
}
