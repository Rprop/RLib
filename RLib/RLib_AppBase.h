/********************************************************************
	Created:	2011/08/15  10:05
	Filename: 	RLib_AppBase.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_BASE
#define _USE_BASE
#include "RLib.h"
//#include <stdlib.h>
#include <tchar.h>
#include <vcruntime_new.h>

#if RLIB_DISABLE_NATIVE_API
# include <sysinfoapi.h>
#endif // RLIB_DISABLE_NATIVE_API

//-------------------------------------------------------------------------

/// <summary>
/// RLib 是一个由类、接口和值类型组成的库，通过该库中的内容可访问系统功能.
/// System 命名空间包含基本类和基类，这些类定义常用的值、事件和事件处理程序、接口、属性和异常处理
/// </summary>
namespace System
{
	namespace IO
	{
		class RLIB_API RLIB_THREAD_SAFE MemoryPool;
	}
	/// <summary>
	/// 表示当前程序的基础类
	/// </summary>
	class RLIB_API RLIB_THREAD_SAFE AppBase
	{
	public:
		/// <summary>
		/// 系统信息结构
		/// </summary>
#if RLIB_DISABLE_NATIVE_API
		typedef SYSTEM_INFO SystemInformation;
#else
		struct SystemInformation // SYSTEM_BASIC_INFORMATION
		{
			ULONG Unknown;               //Always contains zero
			ULONG MaximumIncrement;      //时钟的计量单位
			ULONG PhysicalPageSize;      //内存页的大小
			ULONG NumberOfPhysicalPages; //系统管理着多少个页
			ULONG LowestPhysicalPage;    //低端内存页
			ULONG HighestPhysicalPage;   //高端内存页
			PVOID AllocationGranularity; //未知
			PVOID LowestUserAddress;     //低端用户地址
			PVOID HighestUserAddress;    //高端用户地址
			PVOID ActiveProcessors;      //激活的处理器
			CCHAR NumberProcessors;      //处理器数量
		};
#endif // RLIB_DISABLE_NATIVE_API

	public: 
		/// <summary>
		/// 获取当前进程句柄
		/// </summary>
		static HANDLE GetCurrentProcess() { return reinterpret_cast<HANDLE>(-1); }
		/// <summary>
		/// 获取当前进程ID
		/// </summary>
		static DWORD GetCurrentProcessId();
		/// <summary>
		/// 获取执行线程句柄
		/// </summary>
		static HANDLE GetCurrentThread() { return reinterpret_cast<HANDLE>(-2); }
		/// <summary>
		/// 获取执行线程ID
		/// </summary>
		static DWORD GetCurrentThreadId();
		/// <summary>
		/// 获取系统信息, 不保证方法的线程安全性
		/// </summary>
		static SystemInformation *GetSystemInfo();
		/// <summary>
		/// 在全局共享内存池上分配内存
		/// </summary>
		static RLIB_RESTRICT_RETURN void *Allocate(intptr_t size
												   RLIB_INTERNAL_DEBUG_PARAM);
		/// <summary>
		/// 在全局共享内存池上重新分配内存
		/// </summary>
		static RLIB_RESTRICT_RETURN void *Reallocate(void *p, intptr_t size);
		/// <summary>
		/// 回收全局共享内存池上分配的内存
		/// </summary>
		static void Collect(void *);
		/// <summary>
		/// 返回全局共享内存池指针
		/// </summary>
		static IO::MemoryPool *GetUsingPool();
		/// <summary>
		/// 指示当前进程是否为调试状态
		/// </summary>
		static bool IsDebuggerPresent();
		/// <summary>
		/// 获取可执行文件的绝对地址, Unicode Only
		/// </summary>
		static LPCWSTR GetAbsolutePath();
		/// <summary>
		/// 获取 GetAbsolutePath() 返回字符串的长度
		/// </summary>
		static unsigned short LengthOfAbsolutePath();
		/// <summary>
		/// 获取系统环境中当前目录的值, Unicode Only
		/// </summary>
		static LPCWSTR GetCurrentPath();
		/// <summary>
		/// 获取 GetCurrentPath() 返回字符串的长度
		/// </summary>
		static unsigned short LengthOfCurrentPath();
		/// <summary>
		/// 获取可执行文件的启动命令, Unicode Only
		/// </summary>
		static LPCWSTR GetProcessCommandLine();
		/// <summary>
		/// 获取 GetProcessCommandLine() 返回字符串的长度
		/// </summary>
		static unsigned short LengthOfProcessCommandLine();
		/// <summary>
		/// 获取可执行文件的路径，不包括可执行文件的名称
		/// </summary>
		static LPCTSTR GetStartupPath();
		/// <summary>
		/// 获取可执行文件的路径长度，不包括可执行文件的名称
		/// </summary>
		static intptr_t LengthOfStartupPath();
		/// <summary>
		/// 获取可执行文件的名称
		/// </summary>
		static LPCTSTR GetImageFileName();
		/// <summary>
		/// 获取可执行文件的名称
		/// </summary>
		static intptr_t LengthOfImageFileName();
		/// <summary>
		/// 获取映像基址
		/// </summary>
		static void *GetImageBaseAddress();
		/// <summary>
		/// 获取PEB结构指针
		/// </summary>
		static void *GetPEBAddress();
		/// <summary>
		/// 退出当前进程
		/// </summary>
		static RLIB_NO_RETURN void Exit(DWORD code = 0);
		/// <summary>
		/// 退出当前线程并释放线程内存
		/// </summary>
		static RLIB_NO_RETURN void ExitThread(DWORD code = 0);
		/// <summary>
		/// 退出当前线程并释放线程内存
		/// </summary>
		static RLIB_NO_RETURN void RtlExitThread(DWORD code = 0);
		/// <summary>
		/// 生成异常转储文件
		/// </summary>
		static bool GenerateCrashDump(PEXCEPTION_POINTERS exceptionInfo);
		/// <summary>
		/// 捕获栈信息
		/// </summary>
		static LPCTSTR CaptureStack(LPTSTR buffer, uintptr_t count, LPCTSTR prefix = nullptr);
		/// <summary>
		/// 注册异常回调
		/// </summary>
		static PVOID AddExceptionHandler(ULONG firstHandler,
										 LONG(NTAPI *exceptionHandler)(PEXCEPTION_POINTERS exceptionInfo));
		/// <summary>
		/// 当程序异常时自动生成异常转储文件
		/// </summary>
		static void EnableCrashDump();
		/// <summary>
		/// 判断是否为致命性异常
		/// </summary>
		static bool IsFatalException(PEXCEPTION_POINTERS exceptionInfo);
		/// <summary>
		/// 以对话框形式展示异常信息, 仅 DEBUG 模式有效
		/// </summary>
		static void PrintException(PEXCEPTION_POINTERS exceptionInfo);
		/// <summary>
		/// 获取当前 RLib 版本号
		/// </summary>
		static int GetEnvironmentVersion();
		/// <summary>
		/// 获取当前 RLib 编译时间戳
		/// </summary>
		static LPCTSTR GetBuildTimestamp();
	};
};

#ifdef RLIB_BUILDING
using namespace System;
#endif // RLIB_BUILDING

#endif // _USE_BASE