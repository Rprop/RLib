#pragma once
#define RLIB_INCLUDE_RLIB_LIB    0
#define RLIB_INCLUDE_SUPPORT_LIB 1
#define RLIB_INCLUDE_NATIVE_API  1
#include "RLib_Import.h"

// #if __has_include("RLib_Import.h")
// #endif

#ifdef RLIB_LIB
# pragma init_seg(lib)
#endif // RLIB_LIB

intptr_t   MemoryPage::PageSize = static_cast<intptr_t>(MemoryAllocator::GetPageSize());

static AutoRunOnce<void *> __runOnce([](void *) {
	extern TCHAR    __startupPath[RLIB_MAX_PATH];
	extern TCHAR    __imageFileName[RLIB_MAX_PATH];
	extern intptr_t __startupPathLength;
	extern intptr_t __imageFileNameLength;

	auto image = static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->ImagePathName.Buffer;
	auto pfinder = wcsrchr(image, L'\\');
	assert(pfinder != nullptr);

	intptr_t len_to_copy = pfinder + 1 - image;

#ifdef _UNICODE
	memcpy(__startupPath, image, len_to_copy * sizeof(wchar_t));
	__startupPath[len_to_copy] = L'\0';
	__startupPathLength = len_to_copy;

	image += len_to_copy;
	len_to_copy = static_cast<intptr_t>(wcslen(image));
	memcpy(__imageFileName, image, len_to_copy * sizeof(wchar_t));
	__imageFileName[len_to_copy] = L'\0';
	__imageFileNameLength = len_to_copy;
#else
	size_t size_converted = 0;
	wcstombs_s(&size_converted, __startupPath, RLIB_MAX_PATH, image, static_cast<size_t>(RLIB_MIN(RLIB_MAX_PATH, len_to_copy)));
	__startupPathLength = static_cast<intptr_t>(size_converted / sizeof(TCHAR));
	wcstombs_s(&size_converted, __imageFileName, RLIB_MAX_PATH, image + len_to_copy, _TRUNCATE);
	__imageFileNameLength = static_cast<intptr_t>(size_converted / sizeof(TCHAR));
#endif // _UNICODE

#ifdef _USE_CRC
	extern void __rlib_crc_init();
	__rlib_crc_init(); // avoids static init order issue
#endif // _USE_CRC
}, nullptr);

MemoryPool RLib_GlobalBasePool;