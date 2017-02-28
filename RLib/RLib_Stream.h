/********************************************************************
	Created:	2012/04/22  8:57
	Filename: 	RLib_Stream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_STREAM
#define _USE_STREAM
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// Provides a generic view of a sequence of bytes.
		/// This is an abstract class
		/// </summary>
		class RLIB_API RLIB_NO_VTABLE Stream
		{
		public:
			/// <summary>
			/// When overridden in a derived class, gets the position within the current stream
			/// </summary>
			virtual intptr_t GetPos() const = 0;
			/// <summary>
			/// When overridden in a derived class, sets the position within the current stream
			/// </summary>
			virtual void SetPos(intptr_t Pos) = 0;
			/// <summary>
			/// When overridden in a derived class, gets or sets the position within the current stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Position, GetPos, SetPos);
			/// <summary>
			/// When overridden in a derived class, gets the capacity in bytes of the stream
			/// </summary>
			virtual intptr_t GetSize() const = 0;
			/// <summary>
			/// When overridden in a derived class, sets the capacity in bytes of the stream
			/// </summary>
			virtual void SetSize(intptr_t size) = 0;
			/// <summary>
			/// When overridden in a derived class, gets or sets the capacity in bytes of the stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Size, GetSize, SetSize);
			/// <summary>
			/// When overridden in a derived class, gets the length in bytes of the stream
			/// </summary>
			virtual intptr_t GetLength() const = 0;
			/// <summary>
			/// When overridden in a derived class, sets the length in bytes of the stream
			/// </summary>
			virtual void SetLength(intptr_t len) = 0;
			/// <summary>
			/// When overridden in a derived class, gets or sets the length in bytes of the stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Length, GetLength, SetLength);
			/// <summary>
			/// When overridden in a derived class, reads a sequence of bytes from the current stream
			/// and advances the position within the stream by the number of bytes read
			/// </summary>
			virtual intptr_t Read(LPVOID buffer, intptr_t count) const = 0;
			/// <summary>
			/// When overridden in a derived class, writes a sequence of bytes to the current stream
			/// and advances the current position within this stream by the number of bytes written
			/// </summary>
			virtual intptr_t Write(LPCVOID buffer, intptr_t count) = 0;
			/// <summary>
			/// When overridden in a derived class, clears all buffers for this stream 
			/// and causes any buffered data to be written to the underlying device.
			/// </summary>
			virtual void Flush() = 0;
			/// <summary>
			/// When overridden in a derived class, Closes the current stream
			/// and releases any resources associated with the current stream
			/// </summary>
			virtual void Close() = 0;
			/// <summary>
			/// When overridden in a derived class, identifies whether current class is a derived class of (or the same class as) MemoryStream
			/// </summary>
			virtual bool InheritFromMemoryStream() const = 0;
			/// <summary>
			/// When overridden in a derived class, identifies whether current class is a derived class of (or the same class as) BufferedStream
			/// </summary>
			virtual bool InheritFromBufferedStream() const = 0;

		public: // non-virtual
			/// <summary>
			/// Gets the maximum bytes of readable data from current position 
			/// </summary>
			intptr_t GetMaxReadSize() const {
				assert(this->Length >= this->Position);
				return this->Length - this->Position;
			}
			/// <summary>
			/// Gets the maximum bytes of readable data from current position 
			/// </summary>
			RLIB_PROPERTY_GET(intptr_t MaxReadSize, GetMaxReadSize);
			/// <summary>
			/// Gets the maximum writeable bytes from current position, based on current capacity
			/// </summary>
			intptr_t GetMaxWriteSize() const {
				assert(this->Size >= this->Position);
				return this->Size - this->Position;
			}
			/// <summary>
			/// Gets the maximum writeable bytes from current position, based on current capacity
			/// </summary>
			RLIB_PROPERTY_GET(intptr_t MaxWriteSize, GetMaxWriteSize);
			/// <summary>
			/// Reads the bytes from the current stream and writes them to another stream,
			/// using a specified buffer size
			/// </summary>
			/// <param name="destination">The stream to which the contents of the current stream will be copied</param>
			/// <param name="bufferSize">The size of the buffer</param>
			/// <returns></returns>
			void CopyTo(Stream *destination, intptr_t bufferSize = RLIB_DEFAULT_BUFFER_SIZE) const;
		};
		/// <summary>
		/// Provide a interface to read stream conveniently
		/// </summary>
		class RLIB_API StreamReader
		{
		public:
			LPBYTE   BufferedData;
			intptr_t Capacity;
			bool     CanWrite;

		public:
			StreamReader(const Stream *input, intptr_t size = -1);
			StreamReader(const class MemoryStream *input, intptr_t size = -1);
			~StreamReader();
			RLIB_DECLARE_DYNCREATE;
		};
	}
}

#endif