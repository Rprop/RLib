/********************************************************************
	Created:	2016/07/30  10:08
	Filename: 	RLib_AccessControl.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_SecurityIdentifier.h"

namespace System
{
	namespace Security
	{
		/// <summary>
		/// provides programming elements that control access to and
		/// audit security-related actions on securable objects
		/// </summary>
		namespace AccessControl
		{
			/// <summary>
			/// Represents an Access Control Entry (ACE), 
			/// and is the base class for all other ACE classes
			/// </summary>
			class GenericAce
			{
			};
			/// <summary>
			/// Represents an access control entry(ACE)
			/// </summary>
			class CommonAce : GenericAce
			{
			};
			/// <summary>
			/// Represents an access control list (ACL),
			/// and is the base class for the CommonAcl, and SystemAcl classes
			/// </summary>
			class GenericAcl
			{
			};
			/// <summary>
			/// Represents an access control list (ACL),
			/// and is the base class for the DiscretionaryAcl and SystemAcl classes
			/// </summary>
			class CommonAcl : GenericAcl
			{
			};
			/// <summary>
			/// Represents a System Access Control List (SACL)
			/// </summary>
			class SystemAcl : CommonAcl
			{
			};
			/// <summary>
			/// Represents an Access Control List (ACL)
			/// </summary>
			class RLIB_API RawAcl : GenericAcl
			{
			protected:
				union
				{
					ACL  m_acl;
					char m_data[sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + sizeof(Principal::SecurityIdentifier)];
				};

			public:
				RawAcl();
				~RawAcl() = default;
				RLIB_DECLARE_DYNCREATE;

			public:
				operator PACL();
				/// <summary>
				/// Adds an access-allowed access control entry (ACE) to an access control list (ACL). 
				/// The access is granted to the specified security identifier (SID)
				/// </summary>
				bool AddAccessAllowedAce(_In_ Principal::SecurityIdentifier &sid,
										 _In_ ACCESS_MASK accessMask);
			};
			/// <summary>
			/// Represents a Discretionary Access Control List (DACL)
			/// </summary>
			typedef RawAcl DiscretionaryAcl;
			/// <summary>
			/// Represents a security descriptor. 
			/// A security descriptor includes an owner, a primary group, a Discretionary Access Control List (DACL), and a System Access Control List (SACL)
			/// </summary>
			class GenericSecurityDescriptor
			{
			};
			/// <summary>
			/// Represents a security descriptor.
			/// A security descriptor includes an owner, a primary group, a Discretionary Access Control List (DACL), and a System Access Control List (SACL)
			/// </summary>
			class RLIB_API RawSecurityDescriptor : GenericSecurityDescriptor
			{
			protected:
				SECURITY_DESCRIPTOR m_descriptor;

			public:
				RawSecurityDescriptor(ULONG revision = SECURITY_DESCRIPTOR_REVISION);
				~RawSecurityDescriptor() = default;
				RLIB_DECLARE_DYNCREATE;

			public:
				operator PSECURITY_DESCRIPTOR();
				/// <summary>
				/// Sets the DACL information of an absolute-format security descriptor,
				/// or if there is already a DACL present in the security descriptor, it is superseded
				/// </summary>
				void SetDacl(_In_ bool DaclPresent,
							 _In_opt_ DiscretionaryAcl *pDacl, _In_opt_ bool DaclDefaulted);
			};		
		}
	}
}
