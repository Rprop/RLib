/********************************************************************
	Created:	2012/04/22  8:57
	Filename: 	RLib_DeflateStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_COMPRESSION_DEFLATE
#define _USE_COMPRESSION_DEFLATE

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
			class RLIB_API DeflateStream : public CompressionStream
			{
			public:
				/// <summary>
				/// Initializes a new instance of the DeflateStream class by using the specified stream and compression mode,
				/// and optionally leaves the stream open
				/// </summary>
				/// <param name="stream">The stream to compress or decompress</param>
				/// <param name="mode">One of the enumeration values that indicates whether to compress or decompress the stream</param>
				DeflateStream(Stream *stream, 
							  CompressionMode mode, 
							  CompressionLevel level = CompressionLevel::BestCompression);
				RLIB_DECLARE_DYNCREATE;
			};
			/// <summary>
			/// Provides static methods for compressing and decompressing datas by using the Deflate algorithm
			/// Starting with the RLib v4.5, the Deflate class uses the zlib library. 
			/// As a result, it provides a better compression algorithm and, in most cases, a smaller compressed file than it provides in earlier versions of the RLib
			/// </summary>
			class RLIB_API RLIB_THREAD_SAFE Deflate
			{
			public:
				/// <summary>
				/// compress data with Z_BEST_COMPRESSION
				/// </summary>
				/// <returns>0 if successful, -1 if failed, or a positive number if the output buffer is full</returns>
				static int Compress(IN const unsigned char *data, IN unsigned long ndata,
									OUT unsigned char *zdata, IN OUT unsigned long *nzdata, IN CompressionLevel level);
				/// <summary>
				/// decompress data
				/// </summary>
				/// <returns>0 if successful, -1 if failed, or a positive number if no more input data or the output buffer is full</returns>
				static int Decompress(IN const unsigned char *zdata, IN unsigned long nzdata,
									  OUT unsigned char *data, IN OUT unsigned long *ndata);
			};
		}
	};
};
#endif