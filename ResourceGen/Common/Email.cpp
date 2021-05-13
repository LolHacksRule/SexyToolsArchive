#include "Email.h"
#include <stdio.h>

#ifdef _WIN32
#include <mapi.h>

void SendMail(const std::string& theToAddr, const std::string& theFromAddr, const std::string& theSubject, const std::string& theBody)
{
    LPMAPISENDMAIL lpfnMAPISendMail;
	HINSTANCE hMapi=LoadLibrary("MAPI32.DLL");

	lpfnMAPISendMail=(LPMAPISENDMAIL)GetProcAddress(hMapi,"MAPISendMail");	

	MapiRecipDesc recip;
	MapiMessage note = {0,(char*)theSubject.c_str(),(char*)theBody.c_str(),NULL,NULL,NULL,0,NULL,1,NULL,0,NULL};
	recip.ulReserved = 0;
	recip.ulRecipClass = MAPI_TO;
	recip.lpszName = (char*)theToAddr.c_str();
	recip.lpszAddress = NULL;
	recip.ulEIDSize = 0;
	recip.lpEntryID = NULL;
	note.lpRecips = &recip;
	ULONG flFlags=0;
	LHANDLE lhSession=0;

	if (lpfnMAPISendMail != NULL)	
	{
		if (lpfnMAPISendMail(0L,0L,&note,flFlags,0L) != SUCCESS_SUCCESS)
		{
			printf("Failed to transmit email!\n");
		}
	}
	
	FreeLibrary(hMapi);
}
#else

void SendMail(const std::string& theToAddr, const std::string& theFromAddr, const std::string& theSubject, const std::string& theBody)
{
	if (theBody.length() >= 16384)
	{
		printf("Body too big!");
		return;
	}

	char aBody[32768];
	char* aBodyPtr = aBody;

	for (int i = 0; i < theBody.length(); i++)
	{
		if (theBody[i] == '"')
			*(aBodyPtr++) = '\\';
		*(aBodyPtr++) = theBody[i];
	}
	*(aBodyPtr++) = 0;

	/*string aCmd = string("echo \"content-type: text/html; charset=\"iso-8859-1\"") +
		"\nto: " + theToAddr + 
		"\nfrom: " + theFromAddr +
		"\nsubject:" +  theSubject + "\n" + 
		
		aBody + "\" | /var/qmail/bin/qmail-inject";*/

	cout << "Emailing: " << theToAddr.c_str() << endl;

	FILE* aFP = fopen("mail.txt", "wb");
	if (aFP == NULL)
	{
		cout << "Failed to create mail.txt" << endl;
		return;
	}
	fwrite(theBody.c_str(), 1, theBody.length(), aFP);
	fclose(aFP);

	std::string aCmd = "mailx -a \"from: " + theFromAddr + "\" -a \"content-type: text/html\" " +
		"-s \"" + theSubject + "\" " +
		theToAddr + " < mail.txt";
	
	system(aCmd.c_str());
	//cout << aCmd.c_str() << endl;

    //system(aCmd.c_str());
}

#endif