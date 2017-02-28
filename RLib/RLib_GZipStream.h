/********************************************************************
	Created:	2012/04/22  8:57
	Filename: 	RLib_GZipStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_COMPRESSION_GZIP
#define _USE_COMPRESSION_GZIP

#include "RLib_Compression.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		namespace Compression
		{
			/// <summary>
			/// Provides methods and properties used to compress and decompress streams.
			/// </summary>
			class RLIB_API GZipStream : public CompressionStream
			{
			public:
				/// <summary>
				/// Initializes a new instance of the GZipStream class by using the specified stream and compression mode,
				/// and optionally leaves the stream open
				/// </summary>
				/// <param name="stream">The stream to compress or decompress</param>
				/// <param name="mode">One of the enumeration values that indicates whether to compress or decompress the stream</param>
				GZipStream(Stream *stream, 
						   CompressionMode mode,
						   CompressionLevel level = CompressionLevel::BestCompression,
						   int windowBits = -RLIB_MAX_WBITS);
				RLIB_DECLARE_DYNCREATE;
			};
			/// <summary>
			/// Provides static methods used to compress and decompress datas using the gzip data format, an industry-standard algorithm for lossless file compression and decompression(the same algorithm as the DeflateStream class)
			/// The format includes a cyclic redundancy check value for detecting data corruption
			/// </summary>
			class RLIB_API RLIB_THREAD_SAFE GZip
			{
			public:
				/// <summary>
				/// compress data
				/// </summary>
				/// <returns>0 if successful, -1 if failed, or a positive number if the output buffer is full</returns>
				static int Compress(IN const unsigned char *data, IN unsigned long ndata,
									OUT unsigned char *zdata, IN OUT unsigned long *nzdata, 
									IN CompressionLevel level, IN OPTIONAL int windowBits = -RLIB_MAX_WBITS);
				/// <summary>
				/// decompress data
				/// </summary>
				/// <returns>0 if successful, -1 if failed, or a positive number if no more input data or the output buffer is full</returns>
				static int Decompress(IN const unsigned char *zdata, IN unsigned long nzdata,
									  OUT unsigned char *data, IN OUT unsigned long *ndata, IN OUT int windowBits = -RLIB_MAX_WBITS);
			};
		};
	};
};
#endif