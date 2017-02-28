/********************************************************************
	Created:	2014/07/01  14:27
	Filename: 	RLib_FileStream.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_FILE_STREAM
#define _USE_FILE_STREAM
#include "RLib_Stream.h"
#include "RLib_String.h"
#include "RLib_Exception.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// file attributes information
		/// </summary>
		struct FileFullAttributes
		{
			LARGE_INTEGER CreationTime;
			LARGE_INTEGER LastAccessTime;
			LARGE_INTEGER LastWriteTime;
			LARGE_INTEGER ChangeTime;
			LARGE_INTEGER AllocationSize;
			LARGE_INTEGER EndOfFile;
			ULONG         Attributes;
		};
		/// <summary>
		/// The exception that is set when an I/O error occurs
		/// </summary>
		RLIB_INTERNAL_EXCEPTION(IOException, Exception);
		/// <summary>
		/// Provides a Stream for a file, supporting both synchronous and asynchronous read and write operations
		/// </summary>
		class RLIB_API FileStream : public Stream
		{
			friend class File;

		protected:
			HANDLE                m_native_handle;
			mutable IOException   m_error;
			mutable LARGE_INTEGER m_file_pos;
			
		protected:
			FileStream();
			void setException(NTSTATUS status) const;

		public:
			/// <summary>
			/// Initializes a new instance of the FileStream class
			/// for the specified file handle
			/// </summary>
			FileStream(HANDLE);
			~FileStream();
			RLIB_DECLARE_DYNCREATE;

		private:
			/// <summary>
			/// [Obsolete] This property is not supported and no action is performed
			/// </summary>
			RLIB_DEPRECATED virtual void SetSize(intptr_t) override final { 
				alert(_T("not supported")); 
			}
			/// <summary>
			/// [Obsolete] This property is not supported and always returns -1
			/// </summary>
			RLIB_DEPRECATED virtual intptr_t GetSize() const override final {
				alert(_T("not supported")); return -1; 
			}

		public:
			/// <summary>
			/// Supplies network open information for the file
			/// </summary>
			bool GetFullAttributes(OUT FileFullAttributes *lpFileInformation) const;
			/// <summary>
			/// Gets the length in bytes of the associated file
			/// </summary>
			virtual intptr_t GetLength() const override;
			/// <summary>
			/// Gets the length in bytes of the associated file
			/// </summary>
			LARGE_INTEGER GetLength64() const;
			/// <summary>
			/// Sets the length in bytes of the associated file
			/// </summary>
			virtual void SetLength(intptr_t) override;
			/// <summary>
			/// Sets the length in bytes of the associated file
			/// </summary>
			void SetLength64(LONGLONG);
			/// <summary>
			/// Gets or sets the length in bytes of the associated file
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Length, GetLength, SetLength);
			/// <summary>
			/// Gets the current position of this file stream
			/// </summary>
			virtual intptr_t GetPos() const override;
			/// <summary>
			/// Gets the current position of this large file stream
			/// </summary>
			LARGE_INTEGER GetPos64() const;
			/// <summary>
			/// Sets the current position of this file stream
			/// </summary>
			virtual void SetPos(intptr_t pos) override;
			/// <summary>
			/// Sets the current position of this large file stream
			/// </summary>
			void SetPos64(LONGLONG pos);
			/// <summary>
			/// Gets or sets the current position of this file stream
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Position, GetPos, SetPos);
			/// <summary>
			/// Reads a sequence of bytes from the file
			/// and advances the position within the stream by the number of bytes read
			/// </summary>
			virtual intptr_t Read(LPVOID buffer, intptr_t count) const override;
			/// <summary>
			/// Writes a sequence of bytes to the file
			/// and advances the position within the stream by the number of bytes read
			/// </summary>
			virtual intptr_t Write(const void *buffer, intptr_t count) override;
			/// <summary>
			/// Clears buffers for this stream and causes any buffered data 
			/// to be written to the file
			/// </summary>
			virtual void Flush() override;
			/// <summary>
			/// Closes the associated native file handle
			/// </summary>
			virtual void Close() override;
			/// <summary>
			/// Identifies whether current class is a derived class of (or the same class as) MemoryStream
			/// </summary>
			virtual bool InheritFromMemoryStream() const override final {
				return false;
			}
			/// <summary>
			/// Identifies whether current class is a derived class of (or the same class as) BufferedStream
			/// </summary>
			virtual bool InheritFromBufferedStream() const override final {
				return false;
			}

		public: // non-virtual
			/// <summary>
			/// Gets the physical length in bytes of the associated file
			/// </summary>
			LARGE_INTEGER GetAllocationSize() const;
			/// <summary>
			/// Deletes the associated file.
			/// The method does not close the file handle, and any attempt to refer it is undefined
			/// </summary>
			bool Delete(); 	                         
			/// <summary>
			/// Moves the associated file to another path,
			/// and a value indicating whether to replace if the target path already exist
			/// </summary>
			bool Move(const String &path, bool bReplaceIfExists = true); 
			/// <summary>
			/// Advances the position to the end of the file, and calls Write
			/// </summary>
			void Append(void *buffer, intptr_t count); 
			/// <summary>
			/// Gets a Handle object that represents the operating system file handle
			/// for the file that the current FileStream object encapsulates
			/// </summary>
			operator HANDLE() {
				return this->GetSafeFileHandle();
			}
			/// <summary>
			/// Gets a Handle object that represents the operating system file handle
			/// for the file that the current FileStream object encapsulates
			/// </summary>
			HANDLE GetSafeFileHandle();
			/// <summary>
			/// Gets a Handle object that represents the operating system file handle
			/// for the file that the current FileStream object encapsulates
			/// </summary>
			RLIB_PROPERTY_GET(HANDLE SafeFileHandle, GetSafeFileHandle);
			/// <summary>
			/// Gets the last exception.
			/// The method will never return a nullptr pointer unless 'this' pointer is invalid
			/// </summary>
			IOException *GetLastException() const;
		};
	};
};
#endif