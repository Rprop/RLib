/************************************************************************/
/* Include this file!                                                   */
/************************************************************************/
#pragma once

//#pragma warning(push)

#include "RLib_AppBase.h"
#include "RLib_Utility.h"

#include "RLib_ClassInfo.h"

#include "RLib_Helper.h"

#include "RLib_MemoryAllocator.h"
#include "RLib_MemoryPage.h"
#include "RLib_MemoryPool.h"

#include "RLib_Exception.h"
#include "RLib_ExceptionHelper.h"

#include "RLib_Interlocked.h"
#include "RLib_IntrinsicLock.h"
#include "RLib_Threading.h"
#include "RLib_Monitor.h"
#include "RLib_Thread.h"
#include "RLib_ThreadPool.h"
#include "RLib_Tls.h"

#include "RLib_Object.h"

#include "RLib_SafeObject.h"

#include "RLib_Arithmetic.h"
#include "RLib_Search.h"
#include "RLib_Hash.h"

#include "RLib_CollectionGeneric.h"
#include "RLib_Array.h"
#include "RLib_List.h"
#include "RLib_Stack.h"
#include "RLib_Queue.h"
#include "RLib_PriorityQueue.h"
#include "RLib_BitArray.h"

#include "RLib_Hash.h"
#include "RLib_HashMap.h"
#include "RLib_HashSet.h"
#include "RLib_HashTable.h"

#include "RLib_Stream.h"
#include "RLib_MemoryStream.h"
#include "RLib_BufferedStream.h"
#include "RLib_UnmanagedStream.h"

#include "RLib_Compression.h"
#include "RLib_DeflateStream.h"
#include "RLib_GZipStream.h"

#include "RLib_Text.h"
#include "RLib_String.h"
#include "RLib_StringConvHelper.h"
#include "RLib_StringHelper.h"
#include "RLib_StringArray.h"
#include "RLib_GlobalizeString.h"

#include "RLib_Fundamental.h"

#include "RLib_Random.h"

#include "RLib_DateTime.h"
#include "RLib_Timer.h"

#include "RLib_Hex.h"

#include "RLib_MD5.h"

#include "RLib_Cipher.h"
#include "RLib_RSA.h"
#include "RLib_AES.h"

#include "RLib_RegExp.h"

#include "RLib_Crc.h"

#include "RLib_Winsock.h"
#include "RLib_Net.h"
#include "RLib_Http.h"
#include "RLib_HttpCookie.h"
#include "RLib_HttpResponse.h"
#include "RLib_HttpRequest.h"
#include "RLib_WebClient.h"
#include "RLib_HttpUtility.h"

#include "RLib_Path.h"

#include "RLib_FileStream.h"
#include "RLib_File.h"
#include "RLib_Directory.h"

#include "RLib_Xml.h"

#include "RLib_Base64.h"

#include "RLIB_Ini.h"

#ifdef _WINDOWS
# include "windows/RLib_NativeModule.h"
# include "windows/RLib_FileRegistration.h"
# include "windows/RLib_TextSpeaker.h"
# include "windows/RLib_UAC.h"
#endif // _WINDOWS

//#pragma warning(pop)

#ifndef  RLIB_NAMESPACE
# define RLIB_NAMESPACE 1
#endif // !RLIB_NAMESPACE

#if RLIB_NAMESPACE
using namespace System;
using namespace System::IO;
using namespace System::Xml;
using namespace System::Net;
using namespace System::Text;
using namespace System::Utility;
using namespace System::Security;
using namespace System::Threading;
using namespace System::Collections;
using namespace System::Collections::Generic;
#endif // RLIB_NAMESPACE

#ifndef  RLIB_INCLUDE_RLIB_LIB
# define RLIB_INCLUDE_RLIB_LIB 1
#endif // !RLIB_INCLUDE_NATIVE_API

#if RLIB_INCLUDE_RLIB_LIB
# ifdef _DEBUG
#  ifdef RLIB_LIB
	RLIB_IMPORT_LIB("RLib_" RLIB_PLATFORM "dl")
#  else
	RLIB_IMPORT_LIB("RLib_" RLIB_PLATFORM "d")
#  endif // RLIB_LIB
# else
#  ifdef RLIB_LIB
	RLIB_IMPORT_LIB("RLib_" RLIB_PLATFORM "l")
#  else
	RLIB_IMPORT_LIB("RLib_" RLIB_PLATFORM)
#  endif // RLIB_LIB
# endif
#endif // !RLIB_INCLUDE_RLIB_LIB

#ifndef RLIB_INCLUDE_NATIVE_API
# define RLIB_INCLUDE_NATIVE_API 0
#endif // !RLIB_INCLUDE_NATIVE_API

#if RLIB_INCLUDE_NATIVE_API
# include <RLib_Native.h>
	RLIB_IMPORT_LIB("ntdll_" RLIB_PLATFORM)
#endif // RLIB_INCLUDE_NATIVE_API

#ifndef  RLIB_INCLUDE_SUPPORT_LIB
# define RLIB_INCLUDE_SUPPORT_LIB 0 
#endif // !RLIB_INCLUDE_SUPPORT_LIB

#if RLIB_INCLUDE_SUPPORT_LIB
# ifdef _DEBUG
	RLIB_IMPORT_LIB("SupportLib_" RLIB_PLATFORM "d")
# else
	RLIB_IMPORT_LIB("SupportLib_" RLIB_PLATFORM)
# endif // _DEBUG
#endif // RLIB_INCLUDE_SUPPORT_LIB

