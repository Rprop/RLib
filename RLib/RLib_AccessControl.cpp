/********************************************************************
	Created:	2016/07/30  10:08
	Filename: 	RLib_AccessControl.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AccessControl.h"
#include "RLib_Native.h"
#include "RLib_Exception.h"
using namespace System::Security;
using namespace System::Security::AccessControl;
 
//-------------------------------------------------------------------------

RawSecurityDescriptor::RawSecurityDescriptor(ULONG revision /* = SECURITY_DESCRIPTOR_REVISION */)
{
	RtlCreateSecurityDescriptor(&this->m_descriptor, revision);
}

//-------------------------------------------------------------------------

RawSecurityDescriptor::operator PSECURITY_DESCRIPTOR()
{
	return &this->m_descriptor;
}

//-------------------------------------------------------------------------

void RawSecurityDescriptor::SetDacl(_In_ bool DaclPresent, 
									_In_opt_ DiscretionaryAcl *pDacl,
									_In_opt_ bool DaclDefaulted)
{
	// https://msdn.microsoft.com/en-us/library/ff562781(v=vs.85).aspx
	RtlSetDaclSecurityDescriptor(&this->m_descriptor, 
								 static_cast<BOOLEAN>(DaclPresent), *pDacl, static_cast<BOOLEAN>(DaclDefaulted));
}

//-------------------------------------------------------------------------

RawAcl::RawAcl()
{
	RtlCreateAcl(&this->m_acl, sizeof(this->m_data), ACL_REVISION);
}

//-------------------------------------------------------------------------

RawAcl::operator PACL()
{
	return &this->m_acl;
}

//-------------------------------------------------------------------------

bool RawAcl::AddAccessAllowedAce(_In_ Principal::SecurityIdentifier &sid, 
								 _In_ ACCESS_MASK accessMask)
{
	// https://msdn.microsoft.com/en-us/library/ff552092(v=vs.85).aspx
	NTSTATUS status = RtlAddAccessAllowedAce(&this->m_acl, ACL_REVISION, accessMask, sid);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status);
		return false;
	} //if

	return true;
}