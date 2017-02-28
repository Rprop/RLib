/********************************************************************
	Created:	2016/07/28  9:33
	Filename: 	RLib_Uri.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

namespace System
{
	namespace Net
	{
		/// <summary>
		/// 提供统一资源标识符 (URI) 的对象表示形式和对 URI 各部分的轻松访问
		/// 示例: Scheme://Host/Path[;URICookie]?Query
		/// </summary>
		typedef struct RLIB_API Uri
		{
		public:
			//  获取此实例的主机部分
			String Host;
			//  获取此 URI 的端口号
			USHORT Port;
			//  获取此 URI 的方案名称
			String Scheme;
			//  获取 AbsolutePath 和用问号 (?) 分隔的 Query 属性
			String PathAndQuery;
			// 获取传递给 Uri 构造函数的原始 URI 字符串
//			String OriginalString;

		public:
			/// <summary>
			/// 用指定 URI 初始化 Uri 类的新实例
			/// </summary>
			Uri(const String &url);
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 取得顶级域名
			/// </summary>
			String GetTopDomain();
			/// <summary>
			/// 取得以 / 开头的绝对目录路径
			/// </summary>
			String GetDirectory();
			/// <summary>
			/// 获取文件名, 如果存在
			/// </summary>
			String GetFile();
			/// <summary>
			/// 取得以 / 开头的绝对路径
			/// </summary>
			String GetAbsolutePath();
			/// <summary>
			/// 取得 QueryString 所有字段值
			/// </summary>
			String GetQueryString();
			/// <summary>
			/// 取得 QueryString 特定字段值
			/// </summary>
			String GetQueryString(const String &name);

		public:
			/// <summary>
			/// 提供一种方法对省略的路径进行补足处理
			/// </summary>
			static String ProcessUri(const String &path, const Uri *lpfather);
			/// <summary>
			/// 判断是否为子域名
			/// </summary>
			static bool Uri::IsSubDomain(const String &child, const String &parent);
		} *LPURI;
	}
}