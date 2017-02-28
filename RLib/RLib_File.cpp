/********************************************************************
Created:	2011/02/16  15:29
Filename: 	RLib_File.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_File.h"
#include "RLib_Object.h"
#include "RLib_StringHelper.h"
#include "RLib_GlobalizeString.h"
#include "RLib_UnmanagedStream.h"
#include <windows.h>

using namespace System::IO;

//-------------------------------------------------------------------------

FileStream *File::Open(const String &path, 
					   FileMode mode /* = FileMode::OpenExist */,
					   FileAccess access /* = FileAccess::ReadWrite */,
					   FileShare share /* = FileShare::None */, 
					   FileOptions options /* = FileOptions::None */)
{
	assert(mode != FileMode::CreateOnly && mode != FileMode::CreateNew);

	ManagedObject<FileStream> file = new FileStream;
	if (file.IsNull()) {
		Exception::SetLastException(STATUS_NO_MEMORY, false);
		return nullptr;
	} //if

	// converts to full nt path
	TCHAR fullpath[RLIB_MAX_PATH];
	StringCopyToW(Path::ToNtPath(path), fullpath);
	
	UNICODE_STRING us_path;
    RtlInitUnicodeString(&us_path, fullpath);
    
	// others parameters
	OBJECT_ATTRIBUTES obj_attr;
    InitializeObjectAttributes(&obj_attr, &us_path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
							   NULL, NULL);

	IO_STATUS_BLOCK io_stat;
	NTSTATUS status = NtOpenFile(&file->m_native_handle,
								 static_cast<ACCESS_MASK>(access),
								 &obj_attr,
								 &io_stat,
								 static_cast<ULONG>(share),
								 static_cast<ULONG>(options));
	if (status == STATUS_SUCCESS) {
		switch (mode) {
		case FileMode::Truncate:
		case FileMode::TruncateCreate:
			{
				file->Length = 0;
				break;
			}
		case FileMode::AppendAlways:
			{
				file->Position = file->Length;
				break;
			}
		}
		return file.SuppressFinalize();
	} //if

	if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
		switch (mode) 
		{
		case FileMode::AppendAlways:
		case FileMode::TruncateCreate:
		case FileMode::OpenOrCreate:
			{
				status = NtCreateFile(&file->m_native_handle,
									  static_cast<ACCESS_MASK>(access),
									  &obj_attr,
									  &io_stat,
									  NULL,
									  static_cast<ULONG>(FileAttributes::Normal),
									  static_cast<ULONG>(share),
									  static_cast<ULONG>(mode != FileMode::AppendAlways ? mode : FileMode::OpenOrCreate),
									  static_cast<ULONG>(options),
									  NULL, 
									  NULL);
				if (status == STATUS_SUCCESS) {
					if (mode == FileMode::AppendAlways) {
						file->Position = file->Length;
					} //if
					return file.SuppressFinalize();
				} //if
			}
		}
	} //if

	file->m_native_handle = INVALID_HANDLE_VALUE;
	Exception::SetLastException(status);

    return nullptr;
}

//-------------------------------------------------------------------------

FileStream *File::Create(const String &path,
						 FileMode mode /* = FileMode::CreateOnly */, 
						 FileAccess access /* = FileAccess::ReadWrite */, 
						 FileAttributes attr /* = FileAttributes::Normal */, 
						 FileShare share /* = FileShare::None */, 
						 FileOptions options /* = FileOptions::None */)
{
	ManagedObject<FileStream> file = new FileStream;
	if (file.IsNull()) {
		Exception::SetLastException(STATUS_NO_MEMORY, false);
		return nullptr;
	} //if

	  // converts to full nt path
	TCHAR fullpath[RLIB_MAX_PATH];
	StringCopyToW(Path::ToNtPath(path), fullpath);

	UNICODE_STRING us_path;
	RtlInitUnicodeString(&us_path, fullpath);

	// others parameters
	OBJECT_ATTRIBUTES obj_attr;
	InitializeObjectAttributes(&obj_attr, &us_path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL, NULL);

	IO_STATUS_BLOCK io_stat;
	NTSTATUS status = NtCreateFile(&file->m_native_handle,
								   static_cast<ACCESS_MASK>(access), 
								   &obj_attr, 
								   &io_stat, 
								   NULL,
								   static_cast<ULONG>(attr), 
								   static_cast<ULONG>(share),
								   static_cast<ULONG>(mode != FileMode::AppendAlways ? mode : FileMode::OpenOrCreate),
								   static_cast<ULONG>(options), 
								   NULL,
								   NULL);
	if (status == STATUS_SUCCESS) {
		if (mode == FileMode::AppendAlways) {
			file->Position = file->Length;
		} //if
		return file.SuppressFinalize();
	} //if

	file->m_native_handle = INVALID_HANDLE_VALUE;
	Exception::SetLastException(status);

    return nullptr;
}

//-------------------------------------------------------------------------

bool File::Delete(const String &path)
{
	ManagedObject<FileStream> file = File::Open(path, 
												FileMode::OpenExist, 
												FileAccess::NT_DELETE);
    return file.IsNull() ? false : file->Delete();
}

//-------------------------------------------------------------------------

bool File::TryDelete(const String &path)
{
	return File::Exist(path) ? File::Delete(path) : false;
}

//-------------------------------------------------------------------------

bool File::Exist(const String &str_path)
{
    GlobalizeString u_path(Path::ToNtPath(str_path));
   
    UNICODE_STRING us_path;
    RtlInitUnicodeString(&us_path, u_path.toUnicode());
    
    OBJECT_ATTRIBUTES obj_attr;
    InitializeObjectAttributes(&obj_attr, &us_path,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL, NULL);
    
    IO_STATUS_BLOCK io_stat;
   
    HANDLE hFile;
	NTSTATUS status = NtOpenFile(&hFile,
								 static_cast<ACCESS_MASK>(FileAccess::NT_READ_ATTRIBUTES),
								 &obj_attr,
								 &io_stat,
								 static_cast<ULONG>(FileShare::All), 
								 NULL);
	switch (status)
	{
	case STATUS_SUCCESS:
		NtClose(hFile);
		return true;
	case STATUS_OBJECT_PATH_NOT_FOUND:
//	case STATUS_OBJECT_NAME_INVALID:
	case STATUS_OBJECT_NAME_NOT_FOUND:
		return false;
	}

	Exception::SetLastException(status);

    return io_stat.Information != FILE_DOES_NOT_EXIST;
}

//-------------------------------------------------------------------------

bool File::Copy(const String &path, const String &new_path,
				bool ReplaceIfExists /* = true */)
{
	return ::CopyFile(Path::ToDosPath(path),
					  Path::ToDosPath(new_path),
					  ReplaceIfExists ? FALSE : TRUE) != FALSE;
}

//-------------------------------------------------------------------------

bool File::GetFullAttributes(IN const String &path, OUT FileFullAttributes *lpFileInformation)
{
	static_assert(sizeof(FileFullAttributes) >= sizeof(FILE_NETWORK_OPEN_INFORMATION), "invalid size");

	// converts to full nt path
	TCHAR fullpath[RLIB_MAX_PATH];
	StringCopyToW(Path::ToNtPath(path), fullpath);

	UNICODE_STRING us_path;
	RtlInitUnicodeString(&us_path, fullpath);

	// others parameters
	OBJECT_ATTRIBUTES obj_attr;
	InitializeObjectAttributes(&obj_attr, &us_path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL, NULL);

	NTSTATUS status = NtQueryFullAttributesFile(&obj_attr,
												reinterpret_cast<PFILE_NETWORK_OPEN_INFORMATION>(lpFileInformation));
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		return false;
	} //if
	return true;
}

//-------------------------------------------------------------------------

static RLIB_FORCE_INLINE FileStream *__openForReadOnly(const String &path)
{
	return File::Open(path, 
					  FileMode::OpenExist,
					  FileAccess::Read,
					  FileShare::Read,
					  FileOptions::SequentialScan);
}

//-------------------------------------------------------------------------

String File::ReadAllText(const String &path)
{
    auto lpfile = __openForReadOnly(path);
    if (lpfile != nullptr) {
		auto buf_stream = Text::Encoder::ToCurrentEncoding(Text::UnknownEncoding,
														   *lpfile,
														   lpfile->Length);
		delete lpfile;

		if (buf_stream != nullptr) {
			assert(buf_stream->Position == 0);

			String strResult(buf_stream->Length / RLIB_SIZEOF(TCHAR));
			strResult.copy(
				reinterpret_cast<LPCTSTR>(buf_stream->ObjectData),
				buf_stream->Length / RLIB_SIZEOF(TCHAR)
			);

			delete buf_stream;
			return strResult;
		} //if
    } //if
	return Nothing;
}

//-------------------------------------------------------------------------

bool File::WriteAllText(const String &path, const String &text, 
						Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	return WriteText(path, text, text.Length, codepage);
}

//-------------------------------------------------------------------------

bool File::AppendAllText(const String &path, const String &text,
						 Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	return AppendText(path, text, text.Length, codepage);
}

//-------------------------------------------------------------------------

intptr_t File::ReadText(const String &path, OUT LPTSTR buffer, intptr_t max_length)
{
	auto lpfile = __openForReadOnly(path);
	if (lpfile != nullptr) {
		intptr_t size = 6/*BOM*/ + max_length * RLIB_SIZEOF(wchar_t)/*UNICODE*/;
		auto buf_stream = Text::Encoder::ToCurrentEncoding(Text::UnknownEncoding,
														   *lpfile,
														   RLIB_MIN(size, lpfile->Length));
		delete lpfile;

		if (buf_stream != nullptr) {
			assert(buf_stream->Position == 0);

			intptr_t length = RLIB_MIN(buf_stream->Length / RLIB_SIZEOF(TCHAR), max_length);
			String::FastStringCopy(
				buffer,
				reinterpret_cast<LPCTSTR>(buf_stream->ObjectData),
				length
			);
			if (length < max_length) buffer[max_length] = 0;

			delete buf_stream;
			return max_length;
		} //if
	} //if

	return 0;
}

//-------------------------------------------------------------------------

RLIB_INLINE bool __writeText(const String &path,
							 LPCTSTR lptext, intptr_t length,
							 Text::Encoding codepage /* = Text::UnknownEncoding */,
							 FileMode mode)
{
	ManagedObject<FileStream> output = File::Open(path, mode);
	if (output.IsNotNull()) {
		return Text::Encoder::WriteTextStream(*output,
											  lptext,
											  length * static_cast<intptr_t>(sizeof(TCHAR)),
											  output->Length != 0 ? false : true,
											  codepage);
	} //if

	return false;
}

//-------------------------------------------------------------------------

bool File::WriteText(const String &path,
					 LPCTSTR lptext, intptr_t length, 
					 Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	return __writeText(path, lptext, length, codepage, FileMode::TruncateCreate);
}

//-------------------------------------------------------------------------

bool File::AppendText(const String &path, 
					  LPCTSTR lptext, intptr_t length, 
					  Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	return __writeText(path, lptext, length, codepage, FileMode::AppendAlways);
}
