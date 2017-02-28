/********************************************************************
	Created:	2016/07/30  10:54
	Filename: 	RLib_SecurityIdentifier.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_SecurityIdentifier.h"
#include "RLib_Native.h"
using namespace System::Security::Principal;

//-------------------------------------------------------------------------

SecurityIdentifier::SecurityIdentifier(_In_ PSID sid)
{
	auto lsid = SecurityIdentifier::GetLength(sid);
	assert(lsid <= sizeof(this->m_data));
	memcpy(&this->m_data, sid, RLIB_MIN(lsid, sizeof(this->m_data)));
}

//-------------------------------------------------------------------------

SecurityIdentifier::SecurityIdentifier(_In_ UCHAR subAuthorityCount) 
	: SecurityIdentifier(subAuthorityCount, SECURITY_NT_AUTHORITY)
{
}

//-------------------------------------------------------------------------

SecurityIdentifier::SecurityIdentifier(_In_ UCHAR subAuthorityCount,
									   _In_ SID_IDENTIFIER_AUTHORITY identifierAuthority)
{
	assert(subAuthorityCount <= RLIB_COUNTOF(this->m_data.subAuthorities) + 1);
	RtlInitializeSid(&this->m_sid, &identifierAuthority, subAuthorityCount);
}

//-------------------------------------------------------------------------

SecurityIdentifier::operator PSID()
{
	return &this->m_sid;
}

//-------------------------------------------------------------------------

ULONG &SecurityIdentifier::operator [] (uintptr_t index)
{
	assert(index <= RLIB_COUNTOF(this->m_data.subAuthorities) + 1);
	return *RtlSubAuthoritySid(&this->m_sid, static_cast<ULONG>(index));
}

//-------------------------------------------------------------------------

ULONG SecurityIdentifier::GetLength()
{
	return SecurityIdentifier::GetLength(&this->m_sid);
}

//-------------------------------------------------------------------------

ULONG SecurityIdentifier::GetLength(_In_ PSID sid)
{
	return RtlLengthSid(sid);
}

//-------------------------------------------------------------------------

ULONG SecurityIdentifier::GetLengthRequired(_In_ ULONG subAuthorityCount)
{
	return RtlLengthRequiredSid(subAuthorityCount);
}