/********************************************************************
	Created:	2014/11/02  9:47
	Filename: 	RLib_CRC.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Crc.h"
#include "RLib_Helper.h"
#include "windows/RLib_NativeModule.h"
#include "support/zlib/zlib.h"

using namespace System::Security::Cryptography;

//-------------------------------------------------------------------------

#ifdef _USE_CRC

unsigned long CRC::crc32_get_initial_value()
{
	return crc32(0L, Z_NULL, 0);
}

//-------------------------------------------------------------------------

unsigned long CRC::crc32_block(_In_ unsigned long initialCrc, 
							   _In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes)
{
	return crc32(initialCrc, reinterpret_cast<const Bytef *>(buffer), static_cast<uInt>(sizeInBytes));
}

//-------------------------------------------------------------------------

# if !RLIB_DISABLE_NATIVE_API

static ULONGLONG(NTAPI *__RtlCrc64)(_In_reads_bytes_(Size) const void *Buffer,
									_In_ size_t Size,
									_In_ ULONGLONG InitialCrc);

//-------------------------------------------------------------------------

void __rlib_crc_init()
{
	auto ntdll = Runtime::Marshal::GetNativeModule(L"ntdll.dll");
	if (ntdll != NULL) {
		__RtlCrc64 = static_cast<RLIB_TYPE(__RtlCrc64)>(Runtime::Marshal::GetNativeMethod(ntdll, "RtlCrc64"));
	} else {
		__RtlCrc64 = nullptr;
	} //if
}

//-------------------------------------------------------------------------

unsigned long CRC::native_crc32(_In_ unsigned long initialCrc,
								_In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes)
{
	// RtlCrc32 is unavailable in previous system, use RtlComputeCrc32 instead
	return RtlComputeCrc32(initialCrc, buffer, sizeInBytes);
}

//-------------------------------------------------------------------------

unsigned long long CRC::native_crc64(_In_ unsigned long long initialCrc,
									 _In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes)
{
	// RtlCrc64 is unavailable in previous system
	if (__RtlCrc64) {
		return __RtlCrc64(buffer, sizeInBytes, initialCrc);
	} //if

	// use crc32 instead
	return CRC::native_crc32(static_cast<unsigned long>(initialCrc), buffer, sizeInBytes);
}

# endif // !RLIB_DISABLE_NATIVE_API

#endif // _USE_CRC