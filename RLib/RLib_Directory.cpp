/********************************************************************
	Created:	2014/12/01  17:24
	Filename: 	RLib_Directory.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Directory.h"
#include "RLib_GlobalizeString.h"
#include "RLib_AccessControl.h"
#include "RLib_WindowsIdentity.h"
using namespace System::IO;
using namespace System::Security;
using namespace System::Security::AccessControl;
using namespace System::Security::Principal;

//-------------------------------------------------------------------------

bool Directory::Create(const String &path, ACCESS_MASK mark)
{
	bool br;
	auto lpIdentity = WindowsIdentity::GetCurrent();
	if (lpIdentity) {
		SecurityIdentifier sid(lpIdentity->User);

		RawAcl acl;
		acl.AddAccessAllowedAce(sid, mark);

		RawSecurityDescriptor descriptor;
		descriptor.SetDacl(true, &acl, false);

		br = Directory::Create(path, descriptor);
		delete lpIdentity;
	} else {
		br = Directory::Create(path);
	} //if

	return br;
}

//-------------------------------------------------------------------------

bool Directory::IsDirectory(const String &path)
{
	FileFullAttributes file_info;
	if (!File::GetFullAttributes(path, &file_info)) {
		return false;
	} //if
	return (file_info.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

//-------------------------------------------------------------------------

bool Directory::Create(const String &path, 
					   PSECURITY_DESCRIPTOR lpSecurityDescriptor /* = nullptr */)
{
	GlobalizeString u_path(Path::ToNtPath(path));

	UNICODE_STRING us_path;
	RtlInitUnicodeString(&us_path, u_path.toUnicode());

	OBJECT_ATTRIBUTES obj_attr;
	InitializeObjectAttributes(&obj_attr, &us_path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL, lpSecurityDescriptor);

	IO_STATUS_BLOCK io_stat;

	HANDLE hDirHandle;
	NTSTATUS status = NtCreateFile(&hDirHandle, 
								   FILE_LIST_DIRECTORY | SYNCHRONIZE, 
								   &obj_attr,
								   &io_stat,
								   NULL,
								   FILE_ATTRIBUTE_NORMAL, 
								   FILE_SHARE_READ | FILE_SHARE_WRITE,
								   FILE_CREATE,
								   FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT, 
								   NULL, 
								   NULL);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status);
		return false;
	} //if

	NtClose(hDirHandle);
	return true;
}

//-------------------------------------------------------------------------

bool Directory::CreateIf(const String &path, ACCESS_MASK mark)
{
	return Directory::Exist(path) || Directory::Create(path, mark);
}

//-------------------------------------------------------------------------

bool Directory::CreateIf(const String &path,
						 PSECURITY_DESCRIPTOR lpSecurityDescriptor /* = nullptr */)
{
	return Directory::Exist(path) || Directory::Create(path, lpSecurityDescriptor);
}

//-------------------------------------------------------------------------

bool Directory::Exist(const String &path) 
{
	return File::Exist(path);
}
