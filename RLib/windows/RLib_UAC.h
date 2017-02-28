/********************************************************************
	Created:	2014/07/31  18:28
	Filename: 	RLib_UAC.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_AppBase.h>

#if !(defined _USE_UAC) && !(defined _DISABLE_UAC)
#define _USE_UAC

//-------------------------------------------------------------------------

namespace System
{
	namespace Microsoft
	{
		class RLIB_API UAC
		{
		public:
			/// <summary>
			/// Launch itself or specified file as administrator
			/// The caller must check IsRunAsAdmin() before call, and quit itself after call
			/// </summary>
			/// <returns>ERROR_SUCCESS if successful,
			/// ERROR_CANCELLED if the user refused the elevation
			/// </returns>
			static DWORD RunAsAdministrator(LPCTSTR path = NULL, LPCTSTR params = NULL);
			// 
			//   FUNCTION: IsRunAsAdmin()
			//
			//   PURPOSE: The function checks whether the current process is run as 
			//   administrator. In other words, it dictates whether the primary access 
			//   token of the process belongs to user account that is a member of the 
			//   local Administrators group and it is elevated.
			//
			//   RETURN VALUE: Returns TRUE if the primary access token of the process 
			//   belongs to user account that is a member of the local Administrators 
			//   group and it is elevated. Returns FALSE if the token does not.
			//
			static bool IsRunAsAdmin();
		};
	}
}

#endif //  _USE_UAC
