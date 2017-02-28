/********************************************************************
	Created:	2014/07/19  14:14
	Filename: 	RLib_MemoryStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_UNMANAGED_STREAM
#define _USE_UNMANAGED_STREAM
#include "RLib_MemoryStream.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// Provides access to unmanaged blocks of memory
		/// </summary>
		class RLIB_API UnmanagedMemoryStream : public MemoryStream
		{
		public:
			/// <summary>
			/// Initializes a new instance of the UnmanagedMemoryStream class
			/// </summary>
			UnmanagedMemoryStream()
			{
				this->m_buffer = nullptr;
				this->m_size   = 0;
				this->m_length = 0;
				this->m_pos    = 0;
			}
			/// <summary>
			/// Initializes a new instance of the UnmanagedMemoryStream class
			/// in a safe buffer with a specified size and length
			/// </summary>
			UnmanagedMemoryStream(LPVOID lpbuffer, intptr_t size, intptr_t len)
			{
				this->m_buffer = lpbuffer;
				this->m_size   = size;
				this->m_length = len;
				this->m_pos    = 0;
			}
			/// <summary>
			/// Initializes a new instance of the UnmanagedMemoryStream class
			/// using the specified location and memory length
			/// </summary>
			UnmanagedMemoryStream(LPCVOID lpbuffer, intptr_t len)
			{
				this->m_buffer = const_cast<LPVOID>(lpbuffer);
				this->m_size   = 0;
				this->m_length = len;
				this->m_pos    = 0;
			}
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// Sets the size of a stream to a specified value
			/// </summary>
			virtual void SetSize(intptr_t size) override { 
				this->m_size = size; 
			}
			/// <summary>
			/// Sets the length of a stream to a specified value
			/// </summary>
			virtual void SetLength(intptr_t len) override { 
				this->m_length = len; 
			}
			/// <summary>
			/// Writes a block of bytes to the current stream using data from a buffer
			/// </summary>
			virtual intptr_t Write(LPCVOID buffer, intptr_t count) override;
			/// <summary>
			/// Identifies whether current class is a derived class of (or the same class as) BufferedStream
			/// </summary>
			virtual bool InheritFromBufferedStream() const override final {
				return false;
			}
			/// <summary>
			/// Overrides the Close method so that no action is performed
			/// </summary>
			virtual void Close() override {}
			/// <summary>
			/// Gets or sets the length in bytes of the stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Length, GetLength, SetLength);
			/// <summary>
			/// Gets or sets the position within the current stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Position, GetPos, SetPos);

		public: //non-virtual
			/// <summary>
			/// Sets the pointer to an unmanaged memory location
			/// </summary>
			void SetObjectData(LPVOID pData) 
			{
				this->m_buffer = pData;
			}
			/// <summary>
			/// Gets or sets the pointer to an unmanaged memory location
			/// </summary>
			RLIB_PROPERTY_GET_SET(LPVOID ObjectData, GetObjectData, SetObjectData);
			/// <summary>
			/// Swaps two UnmanagedMemoryStream instance(shallow move)
			/// </summary>
			void ExChange(UnmanagedMemoryStream &);
			/// <summary>
			/// Copies the bytes(&lt;=size) from the location offsetTo to offsetFrom, and may change the length of stream.
			/// Copying takes place as if an intermediate buffer were used, allowing the destination and source to overlap.
			/// </summary>
			void Move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes);			

		public:
			/// <summary>
			/// The function shall associate the buffer given by the buf
			/// and size arguments with a stream(read only)
			/// </summary>
			static UnmanagedMemoryStream *Open_memstream(LPCVOID buf, intptr_t size)
			{
				return new UnmanagedMemoryStream(buf, size);
			}
			/// <summary>
			/// The function shall associate the buffer given by the buf
			/// and size arguments with a stream(read-write)
			/// </summary>
			static UnmanagedMemoryStream *Open_wmemstream(LPVOID buf, intptr_t size)
			{
				return new UnmanagedMemoryStream(buf, size, 0);
			}
		};
	}
}
#endif // _USE_MEMORY_STREAM