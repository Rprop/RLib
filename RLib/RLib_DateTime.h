/********************************************************************
	Created:	2012/06/06  21:51
	Filename: 	RLib_Base64.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_DATETIME
#define _USE_DATETIME
#include "RLib_Fundamental.h"
#include <time.h>

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// Represents an instant in time, typically expressed as a date and time of day
	/// </summary>
	class RLIB_API DateTime
	{
	protected:
		time_t rawtime;

	public:
		Int32 Day;      // Gets the day of the month represented by this instance
		Int32 DayOfWeek;// Gets the day of the week represented by this instance
		Int32 DayOfYear;// Gets the day of the year represented by this instance
		Int32 Hour;     // Gets the hour component of the date represented by this instance
		Int32 Minute;   // Gets the minute component of the date represented by this instance
		Int32 Month;    // Gets the month component of the date represented by this instance
		Int32 Second;   // Gets the seconds component of the date represented by this instance
		Int32 Year;     // Gets the year component of the date represented by this instance

	public:
		DateTime()
		{
			time(&rawtime);
			tm timeinfo;
			localtime_s(&timeinfo, &rawtime);

			this->Second = timeinfo.tm_sec;         /* seconds after the minute - [0,59] */
			this->Minute = timeinfo.tm_min;         /* minutes after the hour - [0,59] */
			this->Hour   = timeinfo.tm_hour;        /* hours since midnight - [0,23] */
			this->Day    = timeinfo.tm_mday;        /* day of the month - [1,31] */
			this->Month  = timeinfo.tm_mon;         /* months since January - [0,11] */
			this->Year   = timeinfo.tm_year + 1900; /* years since 1900 */
			this->DayOfWeek = timeinfo.tm_wday;     /* days since Sunday - [0,6] */
			this->DayOfYear = timeinfo.tm_yday;     /* days since January 1 - [0,365] */
		}
		RLIB_DECLARE_DYNCREATE;

	public:
		String ToString(LPSTR format = "%a, %d %b %Y %H:%M:%S GMT")
		{
			return ToString(&this->rawtime, format);
		}

	public:
		static String ToString(time_t *_time, LPSTR format = "%a, %d %b %Y %H:%M:%S GMT")
		{
			time_t now = _time == nullptr ? time(NULL) : *_time;
			tm _tm;
			gmtime_s(&_tm, &now);

			char date[32];
			strftime(date, sizeof(date), format, &_tm);
			return date;
		}

	public:
		class Now
		{
		public:
			static String ToString(LPSTR format = "%a, %d %b %Y %H:%M:%S GMT")
			{
				return DateTime::ToString(NULL, format);
			}
		};
	};
}

#define RLIB_OBTAIN_DATE(s) String s; { \
	LPCTSTR _ms = _T("JanFebMarAprMayJunJulAugSepOctNovDec"); \
	LPCTSTR _mn = _T("01\0") _T("02\0") _T("03\0") _T("04\0") _T("05\0") _T("06\0") _T("07\0") _T("08\0") _T("09\0") _T("10\0") _T("11\0") _T("12"); \
	TCHAR _dt[] = _T(RLIB_COMPILE_DATE); _dt[3] = _dt[6] = _T('\0'); \
	TCHAR _dn[16]; \
	if (_dt[4] == _T(' ')) _dt[4] = _T('0'); \
	memcpy(&_dn[0], &_dt[7], sizeof(TCHAR) * 4); \
	memcpy(&_dn[4], &_mn[_tcsstr(_ms, &_dt[0]) - _ms], sizeof(TCHAR) * 2); \
	memcpy(&_dn[6], &_dt[4], sizeof(TCHAR) * 3); \
	s = _dn; \
}

#endif /* _USE_DATETIME */
