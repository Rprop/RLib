/********************************************************************
	Created:	2015/04/14  19:38
	Filename: 	RLib_Compression.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_COMPRESSION
#define _USE_COMPRESSION

#include "RLib_Stream.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// The namespace contains classes that provide basic compression and decompression services for streams
		/// </summary>
		namespace Compression
		{
			/// <summary>
			/// Specifies whether to compress or decompress the underlying stream
			/// </summary>
			enum class CompressionMode
			{
				Decompress, // Decompresses the underlying stream
				Compress,   // Compresses the underlying stream
			};
			/// <summary>
			/// Specifies compression levels
			/// </summary>
			enum class CompressionLevel
			{
				Default = -1,
				None = 0,
				BestSpeed = 1,
				BestCompression = 9
			};
			/// <summary>
			/// Provides methods and properties used to compress and decompress streams
			/// </summary>
			class RLIB_API CompressionStream : public Stream
			{
			protected:
				unsigned char   *m_buffer;
				Stream          *m_underlying_stream;
				CompressionMode  m_mode;
				char             m_external_struct[14 * sizeof(intptr_t)];
				bool             m_gzip_header_flag;

			private:
				void           __fetch_data() const; //used only in decompress mode

			protected:
				CompressionStream(Stream *stream, CompressionMode mode);
				~CompressionStream();
				RLIB_DECLARE_DYNCREATE;

			public: // non-virtual
				/// <summary>
				/// Gets a value indicating whether the stream supports reading while decompressing
				/// </summary>
				bool GetCanRead() const {
					assert(this->m_mode == CompressionMode::Decompress);
					return this->m_underlying_stream->MaxReadSize > 0;
				}
				/// <summary>
				/// Gets a value indicating whether the stream supports reading while decompressing
				/// </summary>
				RLIB_PROPERTY_GET(bool CanRead, GetCanRead);
				/// <summary>
				/// Gets a reference to the underlying stream
				/// </summary>
				Stream *GetBaseStream() {
					return this->m_underlying_stream;
				}
				/// <summary>
				/// Gets a reference to the underlying stream
				/// </summary>
				RLIB_PROPERTY_GET(Stream *BaseStream, GetBaseStream);

			public:
				/// <summary>
				/// Reads a number of decompressed bytes into the specified byte array
				/// </summary>
				virtual intptr_t Read(LPVOID buffer, intptr_t count) const override;
				/// <summary>
				/// Writes compressed bytes to the underlying stream from the specified byte array
				/// </summary>
				/// <returns>the total output count to the underlying stream currently, or -1 if failed</returns>
				virtual intptr_t Write(LPCVOID data, intptr_t count) override;
				/// <summary>
				/// Flushs any pending output to the underlying stream
				/// No action is performed when using decompress mode
				/// </summary>
				virtual void Flush() override;
				/// <summary>
				/// Closes the current stream and releases any resources associated with the current stream
				/// @warning This function discards any unprocessed input and does not flush any pending output
				/// </summary>
				virtual void Close() override;
				/// <summary>
				/// 指示当前实例是否继承自 MemoryStream
				/// </summary>
				virtual bool InheritFromMemoryStream() const override final {
					return false;
				}
				/// <summary>
				/// 指示当前实例是否继承自 BufferedStream
				/// </summary>
				virtual bool InheritFromBufferedStream() const override final {
					return false;
				}

			private:
				/// <summary>
				/// This property is not supported and always returns -1
				/// </summary>
				RLIB_DEPRECATED virtual intptr_t GetSize() const override final {
					alert(_T("not supported")); return -1;
				}
				/// <summary>
				/// This property is not supported and no action is performed
				/// </summary>
				RLIB_DEPRECATED virtual void SetSize(intptr_t) override final {
					alert(_T("not supported"));
				}
				/// <summary>
				/// This property is not supported and always returns -1
				/// </summary>
				RLIB_DEPRECATED virtual intptr_t GetLength() const override final {
					alert(_T("not supported")); return -1;
				}
				/// <summary>
				/// This property is not supported and no action is performed
				/// </summary>
				RLIB_DEPRECATED virtual void SetLength(intptr_t) override final {
					alert(_T("not supported"));
				}
				/// <summary>
				/// This property is not supported and always returns -1
				/// </summary>
				RLIB_DEPRECATED virtual intptr_t GetPos() const override final {
					alert(_T("not supported")); return -1;
				}
				/// <summary>
				/// This property is not supported and no action is performed
				/// </summary>
				RLIB_DEPRECATED virtual void SetPos(intptr_t) override final {
					alert(_T("not supported"));
				}
			};
		}
	}
}
#endif