/********************************************************************
	Created:	2014/07/14  15:55
	Filename: 	RLib_ExternDefine.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_EXTERN_DEFINE

#define _USE_EXTERN_DEFINE

#ifdef RLIB_BUILDING

#include "native/RLib_Native.h"
typedef _OBJECT_ATTRIBUTES RLIB_OBJECT_ATTRIBUTES, *PRLIB_OBJECT_ATTRIBUTES;
typedef _IO_STATUS_BLOCK   RLIB_IO_STATUS_BLOCK, *PRLIB_IO_STATUS_BLOCK;
typedef UNICODE_STRING     RLIB_UNICODE_STRING, *PRLIB_UNICODE_STRING;

#else

typedef long NTSTATUS;
typedef struct _OBJECT_ATTRIBUTES_STRUCT
{
	ULONG  Length;
	HANDLE RootDirectory;
	PVOID  ObjectName;
	ULONG  Attributes;
	PVOID  SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID  SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} RLIB_OBJECT_ATTRIBUTES, *PRLIB_OBJECT_ATTRIBUTES;
typedef struct _IO_STATUS_BLOCK_STRUCT
{
	union
	{
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} RLIB_IO_STATUS_BLOCK, *PRLIB_IO_STATUS_BLOCK;
typedef struct _UNICODE_STRING_STRUCT
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} RLIB_UNICODE_STRING, *PRLIB_UNICODE_STRING;
#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#endif // RLIB_BUILDING

/*
 * Maximum queue length specifiable by listen.
 */
#define RLIB_SOMAXCONN       0x7fffffff
/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define RLIB_SD_RECEIVE      0x00
#define RLIB_SD_SEND         0x01
#define RLIB_SD_BOTH         0x02
/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef UINT_PTR             RLIB_SOCKET;
typedef LPVOID               RLIB_SSL_CONTEXT;
/*
 * This is used instead of -1, since the
 * SOCKET type is unsigned.
 */
#define RLIB_INVALID_SOCKET  (RLIB_SOCKET)(~0)
/*
 *	Maximum value for windowBits in deflateInit2 and inflateInit2
 */
#define RLIB_MAX_WBITS   15 /* 32K LZ77 window */

#endif // _USE_EXTERN_DEFINE
