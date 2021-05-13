#include "TimeTools.h"
#include <time.h>

using namespace Sexy;

static const WORD month_len [12] = 
{
    31, /* Jan */
    28, /* Feb */
    31, /* Mar */
    30, /* Apr */
    31, /* May */
    30, /* Jun */
    31, /* Jul */
    31, /* Aug */
    30, /* Sep */
    31, /* Oct */
    30, /* Nov */
    31  /* Dec */
};

/* One second = 10,000,000 * 100 nsec */
static const ULONGLONG systemtime_second = 10000000L;

/* Is the year a leap year?
 * 
 * Use standard Gregorian: every year divisible by 4
 * except centuries indivisible by 400.
 *
 * INPUTS: WORD year the year (AD)
 *
 * OUTPUTS: TRUE if it's a leap year.
 */
BOOL IsLeapYear ( WORD year )
{
    return ( ((year & 3u) == 0)
            && ( (year % 100u == 0)
                || (year % 400u == 0) ));
}

/* A fairly complicated comparison:
 *
 * Compares a test date against a target date.
 * If the test date is earlier, return a negative number
 * If the test date is later, return a positive number
 * If the two dates are equal, return zero.
 *
 * The comparison is complicated by the way we specify
 * TargetDate.
 *
 * TargetDate is assumed to be the kind of date used in 
 * TIME_ZONE_INFORMATION.DaylightDate: If wYear is 0,
 * then it's a kind of code for things like 1st Sunday 
 * of the month. As described in the Windows API docs,
 * wDay is an index of week: 
 *      1 = first of month
 *      2 = second of month
 *      3 = third of month
 *      4 = fourth of month
 *      5 = last of month (even if there are only four such days).
 *
 * Thus, if wYear = 0, wMonth = 4, wDay = 2, wDayOfWeek = 4
 * it specifies the second Thursday of April.
 *
 * INPUTS: SYSTEMTIME * p_test_date     The date to be tested
 *
 *         SYSTEMTIME * p_target_date   The target date. This should be in
 *                                      the format for a TIME_ZONE_INFORMATION
 *                                      DaylightDate or StandardDate.
 *
 * OUTPUT:  -4/+4 if test month is less/greater than target month
 *          -2/+2 if test day is less/greater than target day
 *          -1/+2 if test hour:minute:seconds.milliseconds less/greater than target
 *          0     if both dates/times are equal.
 * 
 */
static int CompareTargetDate (
    const SYSTEMTIME * p_test_date,
    const SYSTEMTIME * p_target_date
    )
{
    WORD first_day_of_month; /* day of week of the first. Sunday = 0 */
    WORD end_of_month;       /* last day of month */
    WORD temp_date;
    int test_milliseconds, target_milliseconds;

    /* Check that p_target_date is in the correct foramt: */
    if (p_target_date->wYear) 
    {
        //error(0,0,"Internal error: p_target_date is not in TIME_ZONE_INFORMATION format.");
        return 0;
    }
    if (!p_test_date->wYear) 
    {
        //error(0,0,"Internal error: p_test_date must be an actual date, not TIME_ZONE_INFORMAATION format.");
        return 0;
    }

    /* Don't waste time calculating if we can shortcut the comparison... */
    if (p_test_date->wMonth != p_target_date->wMonth) 
    {
        return (p_test_date->wMonth > p_target_date->wMonth) ? 4 : -4;
    }

    /* Months equal. Now we neet to do some calculation.
     * If we know that y is the day of the week for some arbitrary date x, 
     * then the day of the week of the first of the month is given by 
     * (1 + y - x) mod 7.
     * 
     * For instance, if the 19th is a Wednesday (day of week = 3), then
     * the date of the first Wednesday of the month is (19 mod 7) = 5.
     * If the 5th is a Wednesday (3), then the first of the month is 
     * four days earlier (it's the first, not the zeroth):
     * (3 - 4) = -1; -1 mod 7 = 6. The first is a Saturday.
     *
     * Check ourselves: The 19th falls on a (6 + 19 - 1) mod 7 
     * = 24 mod 7 = 3: Wednesday, as it should be.
     */
    first_day_of_month = (WORD)( (1u + p_test_date->wDayOfWeek - p_test_date->wDay) % 7u);

    /* If the first of the month comes on day y, then the first day of week z falls on
     * (z - y + 1) mod 7. 
     *
     * For instance, if the first is a Saturday (6), then the first Tuesday (2) falls on a
     * (2 - 6 + 1) mod 7 = -3 mod 7 = 4: The fourth. This is correct (see above).
     *
     * temp_date gets the first <target day of week> in the month.
     */
    temp_date = (WORD)( (1u + p_target_date->wDayOfWeek - first_day_of_month) % 7u);
    /* If we're looking for the third Tuesday in the month, find the date of the first
     * Tuesday and add (3 - 1) * 7. In the example, it's the 4 + 14 = 18th.
     *
     * temp_date now should hold the date for the wDay'th wDayOfWeek of the month.
     * we only need to handle the special case of the last <DayOfWeek> of the month.
     */
    temp_date = (WORD)( temp_date + 7 * p_target_date->wDay );

    /* what's the last day of the month? */
    end_of_month = month_len [p_target_date->wMonth - 1];
    /* Correct if it's February of a leap year? */
    if ( p_test_date->wMonth == 2 && IsLeapYear(p_test_date->wYear) ) 
    {
        ++ end_of_month;
    }

    /* if we tried to calculate the fifth Tuesday of the month
     * we may well have overshot. Correct for that case.
     */
    while ( temp_date > end_of_month)
        temp_date -= 7;

    /* At long last, we're ready to do the comparison. */
    if ( p_test_date->wDay != temp_date ) 
    {
        return (p_test_date->wDay > temp_date) ? 2 : -2;
    }
    else 
    {
        test_milliseconds = ((p_test_date->wHour * 60 + p_test_date->wMinute) * 60 
                                + p_test_date->wSecond) * 1000 + p_test_date->wMilliseconds;
        target_milliseconds = ((p_target_date->wHour * 60 + p_target_date->wMinute) * 60 
                                + p_target_date->wSecond) * 1000 + p_target_date->wMilliseconds;
        test_milliseconds -= target_milliseconds;
        return (test_milliseconds > 0) ? 1 : (test_milliseconds < 0) ? -1 : 0;
    }

}

//
//  Get time zone bias for local time *pst.
//
//  UTC time = *pst + bias.
//
static int GetTimeZoneBias( const SYSTEMTIME * pst )
{
    TIME_ZONE_INFORMATION tz;
    int n, bias;
    
    GetTimeZoneInformation ( &tz );

    /*  I only deal with cases where we look at 
     *  a "last sunday" kind of thing.
     */
    if (tz.DaylightDate.wYear || tz.StandardDate.wYear) 
    {
        //error(0,0, "Cannont handle year-specific DST clues in TIME_ZONE_INFORMATION");
        return 0;
    }

    bias = tz.Bias;

    n = CompareTargetDate ( pst, & tz.DaylightDate );
    if (n < 0)
        bias += tz.StandardBias;
    else 
    {
        n = CompareTargetDate ( pst, & tz.StandardDate );
        if (n < 0)
            bias += tz.DaylightBias;
        else
            bias += tz.StandardBias;
    }
    return bias;
}

/* Convert a file time to a Unix time_t structure. This function is as 
 * complicated as it is because it needs to ask what time system the 
 * filetime describes.
 * 
 * INPUTS:
 *      const FILETIME * ft: A file time. It may be in UTC or in local 
 *                           time (see local_time, below, for details).
 *
 *      time_t * ut:         The destination for the converted time.
 *
 *      BOOL local_time:     TRUE if the time in *ft is in local time 
 *                           and I need to convert to a real UTC time.
 *
 * OUTPUTS:
 *      time_t * ut:         Store the result in *ut.
 */
BOOL Sexy::FileTimeToUnixTime ( const FILETIME* pft, time_t* put, BOOL local_time )
{
    BOOL success = FALSE;

   /* FILETIME = number of 100-nanosecond ticks since midnight 
    * 1 Jan 1601 UTC. time_t = number of 1-second ticks since 
    * midnight 1 Jan 1970 UTC. To translate, we subtract a
    * FILETIME representation of midnight, 1 Jan 1970 from the
    * time in question and divide by the number of 100-ns ticks
    * in one second.
    */

    SYSTEMTIME base_st = 
    {
        1970,   /* wYear            */
        1,      /* wMonth           */
        0,      /* wDayOfWeek       */
        1,      /* wDay             */
        0,      /* wHour            */
        0,      /* wMinute          */
        0,      /* wSecond          */
        0       /* wMilliseconds    */
    };
    
    ULARGE_INTEGER itime;
    FILETIME base_ft;
    int bias = 0;

    if (local_time)
    {
        SYSTEMTIME temp_st;
        success = FileTimeToSystemTime(pft, & temp_st);
        bias =  GetTimeZoneBias(& temp_st);
    }

    success = SystemTimeToFileTime ( &base_st, &base_ft );
    if (success) 
    {
        itime.QuadPart = ((ULARGE_INTEGER *)pft)->QuadPart;

        itime.QuadPart -= ((ULARGE_INTEGER *)&base_ft)->QuadPart;
        itime.QuadPart /= systemtime_second;	// itime is now in seconds.
        itime.QuadPart += bias * 60;    // bias is in minutes.

        *put = itime.LowPart;
    }

    if (!success)
    {
        *put = -1;   /* error value used by mktime() */
    }
    return success;
}

/* Create a FileTime from a time_t, taking into account timezone if required */
BOOL Sexy::UnixTimeToFileTime ( time_t ut, FILETIME* pft, BOOL local_time )
{
    BOOL success = FALSE;

   /* FILETIME = number of 100-nanosecond ticks since midnight 
    * 1 Jan 1601 UTC. time_t = number of 1-second ticks since 
    * midnight 1 Jan 1970 UTC. To translate, we subtract a
    * FILETIME representation of midnight, 1 Jan 1970 from the
    * time in question and divide by the number of 100-ns ticks
    * in one second.
    */

    SYSTEMTIME base_st = 
    {
        1970,   /* wYear            */
        1,      /* wMonth           */
        0,      /* wDayOfWeek       */
        1,      /* wDay             */
        0,      /* wHour            */
        0,      /* wMinute          */
        0,      /* wSecond          */
        0       /* wMilliseconds    */
    };
    
    ULARGE_INTEGER itime;
    FILETIME base_ft;
    int bias = 0;

    SystemTimeToFileTime ( &base_st, &base_ft );
	itime.HighPart=0;
	itime.LowPart = ut;
	itime.QuadPart *= systemtime_second;
	itime.QuadPart += ((ULARGE_INTEGER *)&base_ft)->QuadPart;

    if (local_time)
    {
        SYSTEMTIME temp_st;
        success = FileTimeToSystemTime((FILETIME*)&itime, & temp_st);
        bias =  GetTimeZoneBias(& temp_st);

		itime.QuadPart += (bias * 60) *systemtime_second;
    }

	*(ULARGE_INTEGER*)pft=itime;

    return success;
}

tm* Sexy::UnixTimeToLocalTime(time_t ut)
{
	static tm aTM;
	
	FILETIME aTempFileTime;
	UnixTimeToFileTime(ut, &aTempFileTime, FALSE);
	
	SYSTEMTIME temp_st;
    FileTimeToSystemTime((FILETIME*)&aTempFileTime, &temp_st);
    int aFileBias =  GetTimeZoneBias(&temp_st);

	SYSTEMTIME aTimeNow;
	GetSystemTime(&aTimeNow);
	int aBiasNow =  GetTimeZoneBias(&aTimeNow);

	ULARGE_INTEGER* aBigTime = (ULARGE_INTEGER*) &aTempFileTime;
	aBigTime->QuadPart += ((aFileBias - aBiasNow) * 60) *systemtime_second;

	time_t aNewTimeT;
    FileTimeToUnixTime(&aTempFileTime, &aNewTimeT, FALSE);
	
	return localtime(&aNewTimeT);
}
