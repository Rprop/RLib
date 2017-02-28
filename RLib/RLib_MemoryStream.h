/********************************************************************
	Created:	2014/07/19  14:14
	Filename: 	RLib_MemoryStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_MEMORY_STREAM
#define _USE_MEMORY_STREAM
#include "RLib_Stream.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// Creates a stream whose backing store is memory.
		/// This is an abstract class
		/// </summary>
		class RLIB_API MemoryStream : public Stream
		{
		private:
			RLIB_DECLARE_DYNCREATE;

		protected:
			intptr_t stream_move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes);

		protected:
			LPVOID    m_buffer;
			intptr_t  m_pos;    // next avail position
			intptr_t  m_length; // actual written data length in bytes
			union
			{
				intptr_t m_size;
				intptr_t m_capacity;
			};
		public:
			/// <summary>
			/// Gets the capacity in bytes of the stream
			/// </summary>
			virtual intptr_t GetSize() const override { return this->m_size; }
			/// <summary>
			/// Gets the length in bytes of the stream
			/// </summary>
			virtual intptr_t GetLength() const override { return this->m_length; };
			/// <summary>
			/// Gets the position within the current stream
			/// </summary>
			virtual intptr_t GetPos() const override { return this->m_pos; };
			/// <summary>
			/// Sets the position within the current stream
			/// </summary>
			virtual void SetPos(intptr_t Pos) override;
			/// <summary>
			/// Reads a sequence of bytes from the current stream
			/// and advances the position within the stream by the number of bytes read
			/// </summary>
			/// <returns>
			/// The total number of bytes read into the buffer.This can be less than the number of bytes requested if that many bytes are not currently available,
			/// or -1 if any exception occurred
			/// </returns>
			virtual intptr_t Read(LPVOID buffer, intptr_t count) const override;
			/// <summary>
			/// identifies whether current class is a derived class of (or the same class as) MemoryStream
			/// </summary>
			virtual bool InheritFromMemoryStream() const override final {
				return true;
			}

		public: // non-virtual
			/// <summary>
			/// Gets the pointer of bytes from which this stream was created
			/// </summary>
			LPVOID GetObjectData() const{ return this->m_buffer; }
			/// <summary>
			/// Gets the pointer of bytes from which this stream was created
			/// </summary>
			RLIB_PROPERTY_GET(LPVOID ObjectData, GetObjectData);
			/// <summary>
			/// Gets the pointer of bytes at the current position
			/// </summary>
			void *GetCurrentPtr() const {
				return reinterpret_cast<LPBYTE>(this->ObjectData) + this->Position;
			}
			/// <summary>
			/// Gets the pointer of bytes at the current position
			/// </summary>
			RLIB_PROPERTY_GET(void *CurrentPtr, GetCurrentPtr);
			/// <summary>
			/// Gets or sets the number of bytes allocated for this object
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Capacity, GetSize, SetSize);

		private:
			/// <summary>
			/// Overrides the Stream.Flush method so that no action is performed
			/// </summary>
			RLIB_DEPRECATED void Flush() override final {
				alert(_T("not supported"));
			}
		};
	}
}
#endif // _USE_MEMORY_STREAM