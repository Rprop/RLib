/********************************************************************
	Created:	2016/07/30  14:37
	Filename: 	RLib_WindowsIdentity.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

namespace System
{
	namespace Security
	{
		namespace Principal
		{
			/// <summary>
			/// Represents a Windows user
			/// </summary>
			class RLIB_API WindowsIdentity
			{
			protected:
				union
				{
					TOKEN_USER m_user;
					struct
					{
						TOKEN_USER user;
						DWORD      authority[7];
					} m_data;
				};

			protected:
				WindowsIdentity(HANDLE token);

			public:
				~WindowsIdentity() = default;
				RLIB_DECLARE_DYNCREATE;

			public:
				/// <summary>
				/// Gets the security identifier (SID) for the user
				/// </summary>
				PISID GetUser();
				/// <summary>
				/// Gets the security identifier (SID) for the user
				/// </summary>
				RLIB_PROPERTY_GET(PISID User, GetUser);

			public:
				/// <summary>
				/// Returns a WindowsIdentity object that represents the current Windows user
				/// </summary>
				static WindowsIdentity *GetCurrent();
				/// <summary>
				/// Returns a WindowsIdentity object that represents the Windows identity for either the thread or the process,
				/// depending on the value of the ifImpersonating parameter
				/// </summary>
				/// <param name="ifImpersonating">true to return the WindowsIdentity only if the thread is currently impersonating; false to return the WindowsIdentity of the thread if it is impersonating or the WindowsIdentity of the process if the thread is not currently impersonating</param>
				/// <returns>If ifImpersonating is true and the thread is not impersonating, the returned WindowsIdentity object is nullptr.If ifImpersonating is false and the thread is impersonating, the WindowsIdentity for the thread is returned.If ifImpersonating is false and the thread is not impersonating, the WindowsIdentity for the process is returned</returns>
				static WindowsIdentity *GetCurrent(bool ifImpersonating);
				/// <summary>
				/// wmic useraccount get name, sid
				/// rrrfff S-1-5-21-2532220216-1092076848-2747801457-1001
				/// </summary>
				static String GetCurrentUser();
			};
		}
	}
}

