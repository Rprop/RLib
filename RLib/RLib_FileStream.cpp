/********************************************************************
	Created:	2014/07/01  14:28
	Filename: 	RLib_FileStream.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_StringHelper.h"
#include "RLib_Path.h"
#include "RLib_GlobalizeString.h"
#include "RLib_FileStream.h"
#include "RLib_MemoryPool.h"
#if (WINVER > _WIN32_WINNT_WIN7)
# include <fileapi.h>
# include <handleapi.h>
#endif

using namespace System::IO;

#define return_large_integer_val(a)  LARGE_INTEGER big_int;big_int.QuadPart = a;return big_int;

//-------------------------------------------------------------------------

FileStream::FileStream() : m_native_handle(INVALID_HANDLE_VALUE), m_file_pos{ 0 }
{
}

//-------------------------------------------------------------------------

FileStream::FileStream(HANDLE hFile) : m_file_pos { 0 }
{
	assert(hFile != INVALID_HANDLE_VALUE);
	this->m_native_handle = hFile;

// 	#pragma todo("This function may be changed or removed from Windows without further notice")
// 	OBJECT_NAME_INFORMATION file_info;
// 	NTSTATUS status = NtQueryObject(hFile, ObjectNameInformation,
// 									&file_info, sizeof(file_info), NULL);
// 	if (status != STATUS_SUCCESS) {
// 		this->setException(status);
// 		RLIB_SetExceptionInfo(this->m_error);
// 		return;
// 	} //if
}

//-------------------------------------------------------------------------

FileStream::~FileStream()
{
	this->Close();
}

//-------------------------------------------------------------------------

void FileStream::Close()
{
	if (this->m_native_handle != INVALID_HANDLE_VALUE) {
		NTSTATUS status = NtClose(this->m_native_handle);
		if (status != STATUS_SUCCESS) {
			this->setException(status);
			RLIB_SetExceptionInfo(this->m_error);
		} //if

		this->m_native_handle = INVALID_HANDLE_VALUE;
	} //if
}

//-------------------------------------------------------------------------

HANDLE FileStream::GetSafeFileHandle()
{
	return this->m_native_handle;
}

//-------------------------------------------------------------------------

intptr_t FileStream::GetPos() const
{
#ifdef _WIN64
	return this->GetPos64().QuadPart;
#else
	return static_cast<intptr_t>(this->GetPos64().LowPart);
#endif // _WIN64
}

//-------------------------------------------------------------------------

LARGE_INTEGER FileStream::GetPos64() const
{
	return this->m_file_pos;
}

//-------------------------------------------------------------------------

void FileStream::SetPos(intptr_t pos)
{
	this->SetPos64(pos);
}

//-------------------------------------------------------------------------

void FileStream::SetPos64(LONGLONG pos)
{
	this->m_file_pos.QuadPart = pos;
}

//-------------------------------------------------------------------------

void FileStream::SetLength(intptr_t length)
{
	this->SetLength64(length);
}

//-------------------------------------------------------------------------

void FileStream::SetLength64(LONGLONG length)
{
	FILE_END_OF_FILE_INFORMATION FileEndOf;
	FileEndOf.EndOfFile.QuadPart = length;

	IO_STATUS_BLOCK io_stat;
	NtSetInformationFile(this->m_native_handle, &io_stat,
						 &FileEndOf, sizeof(FileEndOf), FileEndOfFileInformation);
	if (io_stat.Status != STATUS_SUCCESS) {
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
	} //if
}

//-------------------------------------------------------------------------

IOException *FileStream::GetLastException() const
{
	return &this->m_error;
}

//-------------------------------------------------------------------------

intptr_t FileStream::GetLength() const
{
#ifdef _WIN64
	return this->GetLength64().QuadPart;
#else
	return static_cast<intptr_t>(this->GetLength64().LowPart);
#endif // _WIN64

}

//-------------------------------------------------------------------------

LARGE_INTEGER FileStream::GetLength64() const
{
	FileFullAttributes file_info;
	if (!this->GetFullAttributes(&file_info)) {
		return_large_integer_val(-1);
	} //if
	return file_info.EndOfFile;
}

//-------------------------------------------------------------------------

LARGE_INTEGER FileStream::GetAllocationSize() const
{
	FileFullAttributes file_info;
	if (!this->GetFullAttributes(&file_info)) {
		return_large_integer_val(-1);
	} //if
	return file_info.AllocationSize;
}

//-------------------------------------------------------------------------

bool FileStream::GetFullAttributes(OUT FileFullAttributes *lpFileInformation) const
{
	static_assert(sizeof(FileFullAttributes) >= sizeof(FILE_NETWORK_OPEN_INFORMATION), "invalid size");

	IO_STATUS_BLOCK io_stat;
	NTSTATUS status = NtQueryInformationFile(this->m_native_handle, &io_stat,
											 reinterpret_cast<PFILE_NETWORK_OPEN_INFORMATION>(lpFileInformation),
											 sizeof(FILE_NETWORK_OPEN_INFORMATION),
											 FileNetworkOpenInformation);
	if (status != STATUS_SUCCESS) {
		this->setException(status);
		RLIB_SetExceptionInfo(this->m_error);
		return false;
	} //if
	return true;
}

//-------------------------------------------------------------------------

bool FileStream::Delete()
{
	FILE_DISPOSITION_INFORMATION file_info;
	file_info.bDeleteFile = TRUE;

	IO_STATUS_BLOCK io_stat;
	io_stat.Status = NtSetInformationFile(this->m_native_handle, &io_stat, &file_info,
										  sizeof(file_info), FileDispositionInformation);
	if (io_stat.Status != STATUS_SUCCESS) {
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
		return false;
	} //if
	return true;
}

//-------------------------------------------------------------------------

bool FileStream::Move(const String &path, bool bReplaceIfExists /* = true */)
{
	// https://msdn.microsoft.com/en-us/library/windows/hardware/ff567096(v=vs.85).aspx
	IO_STATUS_BLOCK io_stat;
	size_t nsize = sizeof(FILE_RENAME_INFORMATION) + TSIZE(RLIB_MAX_PATH);
	auto lpinfo  = RLIB_GlobalAllocAny(FILE_RENAME_INFORMATION *, nsize);
	if (lpinfo != nullptr) {
		lpinfo->ReplaceIfExists = static_cast<BOOLEAN>(bReplaceIfExists);
		lpinfo->RootDirectory   = nullptr;

		GlobalizeString u_path(Path::ToNtPath(path));
		lpinfo->FileNameLength = static_cast<ULONG>(RLIB_MIN(TSIZE(RLIB_MAX_PATH),
															 u_path.sizeofUnicode()));
		memcpy(lpinfo->FileName, u_path.toUnicode(), lpinfo->FileNameLength);
		
		// Change the current file name, which is supplied in a FILE_RENAME_INFORMATION structure
		// The caller must have DELETE access to the file
		io_stat.Status = NtSetInformationFile(this->m_native_handle, &io_stat, lpinfo,
											  static_cast<ULONG>(nsize), FileRenameInformation);

		RLIB_GlobalCollect(lpinfo);
	} else {
		io_stat.Status = STATUS_NO_MEMORY;
	} //if

	if (io_stat.Status != STATUS_SUCCESS) {
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
		return false;
	} //if

	return true;
}

//-------------------------------------------------------------------------

intptr_t FileStream::Read(void *buffer, intptr_t count) const
{
	LARGE_INTEGER file_point = this->GetPos64();
	assert(file_point.QuadPart != -1);

	IO_STATUS_BLOCK io_stat;
	io_stat.Status = NtReadFile(this->m_native_handle, NULL, NULL, NULL,
								&io_stat, buffer, static_cast<ULONG>(count),
								&file_point, nullptr);
	switch (io_stat.Status)
	{
	case STATUS_SUCCESS:
		goto __successful;
	case STATUS_PENDING:
		io_stat.Status = NtWaitForSingleObject(this->m_native_handle, TRUE, NULL);
		switch (io_stat.Status)
		{
		case STATUS_SUCCESS:
		case STATUS_ALERTED:
		case STATUS_USER_APC:
		case STATUS_TIMEOUT: // TODO ?
			goto __successful;
		}
	default:
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
		return -1;
	}

__successful: 
	// The Information member receives the number of bytes actually read from the file
	this->m_file_pos.QuadPart = file_point.QuadPart + static_cast<LONGLONG>(io_stat.Information);
	return static_cast<intptr_t>(io_stat.Information);
}

//-------------------------------------------------------------------------

intptr_t FileStream::Write(const void *buffer, intptr_t count)
{
	// https://msdn.microsoft.com/en-us/library/windows/hardware/ff567121(v=vs.85).aspx
	LARGE_INTEGER file_point = this->GetPos64();
	assert(file_point.QuadPart != -1);
	
	IO_STATUS_BLOCK io_stat;
	io_stat.Status = NtWriteFile(this->m_native_handle, NULL, NULL, NULL, &io_stat,
								 const_cast<PVOID>(buffer),
								 static_cast<ULONG>(count), &file_point, nullptr);
	switch (io_stat.Status)
	{
	case STATUS_SUCCESS:
		goto __successful;
	case STATUS_PENDING:
		io_stat.Status = NtWaitForSingleObject(this->m_native_handle, TRUE, NULL);
		switch (io_stat.Status)
		{
		case STATUS_SUCCESS:
		case STATUS_ALERTED:
		case STATUS_USER_APC:
		case STATUS_TIMEOUT: // TODO ?
			goto __successful;
		}
	default:
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
		return -1;
	}

__successful:
	// The Information member receives the number of bytes actually written to the file
	this->m_file_pos.QuadPart = file_point.QuadPart + static_cast<LONGLONG>(io_stat.Information);
	return static_cast<intptr_t>(io_stat.Information);
}

//-------------------------------------------------------------------------

void FileStream::Flush()
{
	IO_STATUS_BLOCK io_stat;
	NtFlushBuffersFile(this->m_native_handle, &io_stat);
	if (io_stat.Status != STATUS_SUCCESS) {
		this->setException(io_stat.Status);
		RLIB_SetExceptionInfo(this->m_error);
	} //if
}

//-------------------------------------------------------------------------

void FileStream::Append(void *buffer, intptr_t count)
{
	this->SetPos(this->Length);
	this->Write(buffer, count);
}

//-------------------------------------------------------------------------

void FileStream::setException(NTSTATUS status) const
{
	Exception::FormatException(&this->m_error, RtlNtStatusToDosErrorNoTeb(status));
	alert(this->m_error.Message);
	Exception::SetLastErrorId(RtlNtStatusToDosErrorNoTeb(status));
}