/********************************************************************
	Created:	2014/06/30  18:24
	Filename: 	RLib_Exception.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

#ifndef _USE_RLIB_EXCEPTION
#define _USE_RLIB_EXCEPTION

namespace System
{
	/// <summary>
	/// 表示标准异常类
	/// </summary>
	class RLIB_API Exception
	{
	public:
		/// <summary>
		/// 获取或设置 HRESULT，它是分配给特定异常的编码数值
		/// </summary>
		union
		{
			INT   HResult;
			DWORD ErrorNo;
		};
#ifdef _WIN64
		INT __dummy;
#endif // _WIN64
		/// <summary>
		/// 获取描述当前异常的消息
		/// </summary>
		TCHAR Message[RLIB_EXCEPTION_MSG_LENGTH];
#ifdef _DEBUG
		/// <summary>
		/// 获取导致错误的源代码文件名称
		/// </summary>
		const TCHAR *Source;
		/// <summary>
		/// 获取引发当前异常的方法
		/// </summary>
		const TCHAR *TargetSite; 	
#endif // _DEBUG

	public:
		/// <summary>
		/// 初始化 Exception 类的新实例
		/// </summary>
		Exception();
		/// <summary>
		/// 使用指定的错误编号和错误消息初始化 Exception 类的新实例
		/// </summary>
		Exception(const TCHAR *lpmsg, INT id = -1);
		RLIB_DECLARE_DYNCREATE;
		/// <summary>
		/// 设置异常信息
		/// </summary>
#ifdef _DEBUG
		void Set(INT HResult, const TCHAR *Message, const TCHAR *Source, const TCHAR *TargetSite);
		void SetDebugInfo(const TCHAR *Source, const TCHAR *TargetSite);
#else
		void Set(INT HResult, const TCHAR *Message);
		void SetDebugInfo(const TCHAR *, const TCHAR *) {}
#endif // _DEBUG
		/// <summary>
		/// 引用异常类
		/// </summary>
		void Ref(const class Exception &);

	public:
		/// <summary>
		/// 获取最近一次错误编号
		/// </summary>
		static DWORD GetLastErrorId();
		/// <summary>
		/// Formats a message string, the caller can ask the function 
		/// to search the system's message table resource(s) for the message definition
		/// </summary>
		/// <param name="lptstr">A pointer to a buffer that receives the null-terminated string that specifies the formatted message</param>
		/// <param name="nchar">Specifies the size of the output buffer, in TCHARs, with null-terminator</param>
		/// <param name="err_code">Optional custom error code</param>
		static bool FormatErrorMessage(OUT LPTSTR lptstr, DWORD nchar, IN DWORD err_code);
		/// <summary>
		/// 获取文本错误信息
		/// </summary>
		static bool FormatException(OUT Exception *lpex, IN DWORD err_code);
		/// <summary>
		/// 获取最近一次的异常对象
		/// </summary>
		static Exception *GetLastException();
		/// <summary>
		/// 获取最近一次的异常对象, 并指定 HResult 属性
		/// </summary>
		static Exception *GetLastException(DWORD err_code);
		/// <summary>
		/// 设置最近一次发生的错误
		/// </summary>
		static void SetLastErrorId(DWORD err_code);
		/// <summary>
		/// Sets the last exception from NTSTATUS
		/// </summary>
		static void SetLastException(NTSTATUS status, bool bAlertInDebug = true);
	};
}
#endif // _USE_RLIB_EXCEPTION