/********************************************************************
	Created:	2014/07/19  14:14
	Filename: 	RLib_MemoryStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_BUFFERED_STREAM
#define _USE_BUFFERED_STREAM
#include "RLib_MemoryStream.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// Creates a stream whose backing store is memory
		/// </summary>
		class RLIB_API BufferedStream : public MemoryStream
		{
		public:
			/// <summary>
			/// Initializes a new instance of the BufferedStream class with a default buffer size
			/// </summary>
			explicit BufferedStream(intptr_t size = RLIB_DEFAULT_BUFFER_SIZE);
			~BufferedStream();
			RLIB_DECLARE_DYNCREATE;

		public:		
			/// <summary>
			/// Sets the length in bytes of the stream
			/// </summary>
			/// <param name="len">&lt;size</param>
			virtual void SetLength(intptr_t len) override;		
			/// <summary>
			/// Gets or sets the length in bytes of the stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Length, GetLength, SetLength);
			/// <summary>
			/// Closes the current stream and releases memory associated with the current stream
			/// </summary>
			virtual void Close() override;
			/// <summary>
			/// Writes a sequence of bytes to the current stream
			/// and advances the current position within this stream by the number of bytes written
			virtual intptr_t Write(LPCVOID buffer, intptr_t count) override;
			/// <summary>
			/// Identifies whether current class is a derived class of (or the same class as) BufferedStream
			/// </summary>
			virtual bool InheritFromBufferedStream() const override final {
				return true;
			}
			/// <summary>
			/// Sets the capacity in bytes of the stream.
			/// The method is deprecated and not supported currently
			/// </summary>
			RLIB_DEPRECATED virtual void SetSize(intptr_t) override final {
				trace(!"not supported");
			}
			/// <summary>
			/// Gets or sets the position within the current stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Position, GetPos, SetPos);

		public: //non-virtual
			/// <summary>
			/// Swaps two BufferedStream instance(shallow move)
			/// </summary>
			void ExChange(BufferedStream &);
			/// <summary>
			/// Ensures that the capacity of this instance is at least the specified value
			/// </summary>
			bool EnsureCapacity(intptr_t value);
			/// <summary>
			/// Copies the bytes(&lt;=size) from the location offsetTo to offsetFrom, and may change the length of stream.
			/// Copying takes place as if an intermediate buffer were used, allowing the destination and source to overlap.
			/// </summary>
			void Move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes);
		};
	}
}
#endif // _USE_BUFFERED_STREAM