#include "Common.h"

namespace Sexy
{

BOOL FileTimeToUnixTime (const FILETIME* pft, time_t* put, BOOL local_time);
BOOL UnixTimeToFileTime (time_t ut, FILETIME* pft, BOOL local_time);
tm* UnixTimeToLocalTime(time_t ut);

}