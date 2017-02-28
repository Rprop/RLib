/********************************************************************
	Created:	2014/11/02  9:47
	Filename: 	RLib_CRC.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_CryptographyBase.h"

#if !(defined _USE_CRC) && !(defined _DISABLE_CRC)
#define _USE_CRC

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			/// <summary>
			/// Cyclic redundancy check algorithm
			/// </summary>
			class RLIB_THREAD_SAFE RLIB_API CRC
			{
			public:
				/// <summary>
				/// this function returns the required initial value for the crc.
				/// </summary>
				static unsigned long crc32_get_initial_value();
				/// <summary>
				/// calculate a checksum on a buffer -- start address = p, length = bytelength
				/// CRC32 is a checksum/hashing algorithm that is very commonly used in kernels,
				/// and for Internet checksums.
				/// </summary>
				static unsigned long crc32_block(_In_ unsigned long initialCrc,
												 _In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes);

#if !RLIB_DISABLE_NATIVE_API
			public: // crc32 using native system api
				static unsigned long native_crc32(_In_ unsigned long initialCrc,
												  _In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes);
				static unsigned long long native_crc64(_In_ unsigned long long initialCrc,
												  _In_reads_bytes_(sizeInBytes) const void *buffer, _In_ size_t sizeInBytes);
#endif // !RLIB_DISABLE_NATIVE_API

			};
		}
	}
}

#endif
