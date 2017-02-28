/********************************************************************
	Created:	2016/07/30  10:54
	Filename: 	RLib_SecurityIdentifier.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_AppBase.h"

namespace System
{
	namespace Security
	{
		/// <summary>
		/// Defines a principal object that represents the security context under which code is running
		/// </summary>
		namespace Principal
		{
			/// <summary>
			/// Represents a security identifier (SID) and provides marshaling and comparison operations for SIDs
			/// </summary>
			class RLIB_API SecurityIdentifier
			{
			protected:
				union
				{
					SID m_sid;
					struct
					{
						SID   sid; // 1
						DWORD subAuthorities[1 + 2 + 2 + 2]; // 7
					} m_data; // 8
				};

			public:
				SecurityIdentifier(_In_ PSID sid);
				SecurityIdentifier(_In_ PISID sid) : SecurityIdentifier(static_cast<PSID>(sid)) {}
				SecurityIdentifier(_In_ UCHAR subAuthorityCount);
				SecurityIdentifier(_In_ UCHAR subAuthorityCount,
								   _In_ SID_IDENTIFIER_AUTHORITY identifierAuthority);
				~SecurityIdentifier() = default;
				RLIB_DECLARE_DYNCREATE;

			public:
				operator PSID();
				ULONG &operator [] (uintptr_t index);
				ULONG GetLength();
				RLIB_PROPERTY_GET(ULONG Length, GetLength);

			public:
				static ULONG GetLength(_In_ PSID sid);
				static ULONG GetLengthRequired(_In_ ULONG subAuthorityCount);
			};
		}
	}
}
