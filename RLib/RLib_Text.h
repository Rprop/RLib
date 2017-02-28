/********************************************************************
	Created:	2012/02/05  22:43
	Filename: 	RLib_Text.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_TEXT
#define _USE_TEXT
#include "RLib_BufferedStream.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 包含用于字符编码和字符串操作的类型
	/// </summary>
	namespace Text
	{
		/// <summary>
		/// 表示编码格式
		/// For more Code Page Identifiers, visit http://msdn.microsoft.com/en-us/library/dd317756(v=vs.85).aspx
		/// http://www.unicode.org/faq/utf_bom.html
		/// </summary>
		enum Encoding
		{
			// 未知/默认编码
			UnknownEncoding = 0,
			// ANSI/OEM Simplified Chinese (PRC, Singapore)
			// Chinese Simplified (GB2312)
			ASCIIEncoding = 936,
			// Unicode (UTF-8)
			UTF8Encoding = 65001,
			// Unicode UTF-16, little endian byte order (BMP of ISO 10646)
			// available only to managed applications
			UTF16Encoding = 1200,
			// Unicode UTF-16, big endian byte order
			// available only to managed applications
			UTF16BEEncoding = 1201,
		};
		/// <summary>
		/// 提供一组字符编码的转换方法
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Encoder
		{
		public:
			/// <summary>
			/// Determines if a buffer is likely to contain a form of Unicode text
			/// @warning not supported currently
			/// </summary>
			static bool IsTextUnicode(_In_ const void *lpv,
									  _In_ intptr_t size);
			/// <summary>
			/// Determines if a buffer is likely to contain a form of Utf-8 text
			/// </summary>
			static bool IsTextUtf8(_In_ const void *lpv,
								   _In_ intptr_t size);
			/// <summary>
			/// 在头部查找字节顺序标记并返回编码方式
			/// @warning 确保传入的缓冲区至少有3个可读字节
			/// </summary>
			static Encoding DetectEncodingFromByteOrderMarks(LPCVOID ptr, OUT intptr_t *lpbytes = nullptr);
			/// <summary>
			/// 根据指定编码写入字节顺序标记
			/// </summary>
			static bool WriteByteOrderMarks(IO::Stream &stream, Encoding codepage);
			/// <summary>
			/// 获取当前默认的字符编码方式
			/// </summary>
			static Encoding GetCurrentEncoding();
			/// <summary>
			/// 将指定编码方式的数据转换为当前编码格式(UTF-16(_UNICODE编译选项) or GB2312)
			/// </summary>
			static IO::BufferedStream *ToCurrentEncoding(LPCVOID lpdata, intptr_t size,
														 Encoding codepage, bool detectEncodingFromByteOrderMarks = true);
			/// <summary>
			/// 将指定编码方式的数据流转换为当前编码格式(UTF-16(_UNICODE编译选项) or GB2312)
			/// </summary>
			static IO::BufferedStream *ToCurrentEncoding(Encoding codepage, const IO::Stream &stream,
														 intptr_t length = -1, bool detectEncodingFromByteOrderMarks = true);
			/// <summary>
			/// 将指定编码方式的多字节数据流转换为宽字符数据流(any to utf-16)
			/// </summary>
			/// <returns>返回的数据流必须由用户释放(delete), 且默认Pos = 0</returns>
			static IO::BufferedStream *ToWideChar(LPCVOID pdata, intptr_t size, Encoding codepage);
			/// <summary>
			/// 将指定编码方式的多字节数据流转换为宽字符数据流(any to utf-16)
			/// </summary>
			/// <returns>返回的数据流必须由用户释放(delete), 且默认Pos = 0</returns>
			static IO::BufferedStream *ToWideChar(Encoding codepage, const IO::Stream &stream, 
												  intptr_t length = -1);
			/// <summary>
			/// 将宽字符数据流转换为指定编码方式的多字节数据流(utf-16 to any)
			/// </summary>
			/// <returns>返回的数据流必须由用户释放(delete), 且默认Pos = 0</returns>
			static IO::BufferedStream *WideCharTo(LPCVOID pdata, intptr_t size, Encoding codepage);
			/// <summary>
			/// 将宽字符数据流转换为指定编码方式的多字节数据流(utf-16 to any)
			/// </summary>
			/// <returns>返回的数据流必须由用户释放(delete), 且默认Pos = 0</returns>
			static IO::BufferedStream *WideCharTo(Encoding codepage, const IO::Stream &stream, 
												  intptr_t length = -1);
			/// <summary>
			/// 以指定编码将文本流写入指定流
			/// </summary>
			/// <param name="outputStream">目标流</param>
			/// <param name="textStream">源文本流, 编码与编译环境相关</param>
			/// <param name="bytesToWrite">写入长度, in bytes</param>
			/// <param name="useBOM">是否包含BOM文件头</param>
			/// <param name="codepage">输出的编码方式</param>
			static bool WriteTextStream(OUT IO::Stream &outputStream, 
										IN const IO::Stream &textStream,
										intptr_t bytesToWrite = -1, 
										bool useBOM = true,
										Text::Encoding codepage = Text::UnknownEncoding);
			/// <summary>
			/// 以指定编码将文本流写入指定流
			/// </summary>
			/// <param name="outputStream">目标流</param>
			/// <param name="textData">源文本指针, 编码与编译环境相关</param>
			/// <param name="bytesToWrite">写入长度, in bytes</param>
			/// <param name="useBOM">是否包含BOM文件头</param>
			/// <param name="codepage">输出的编码方式</param>
			static bool WriteTextStream(OUT IO::Stream &outputStream,
										IN LPCVOID textData,
										intptr_t bytesToWrite,
										bool useBOM = true,
										Text::Encoding codepage = Text::UnknownEncoding);
		};
	};
};
#endif