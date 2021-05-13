#ifndef __EMAIL_H__
#define __EMAIL_H__

#include "Common.h"

void SendMail(const std::string& theToAddr, const std::string& theFromAddr, const std::string& theSubject, const std::string& theBody);

#endif //__EMAIL_H__