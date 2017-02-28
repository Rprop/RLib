/********************************************************************
	Created:	2014/07/14  16:10
	Filename: 	RLib_Net.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_NET
#define _USE_NET
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		/// <summary>
		/// Ipv4 host info
		/// </summary>
		typedef struct
		{
			unsigned long  addr;
			unsigned short port;
		} Ipv4Host;	
		/// <summary>
		/// 表示文件压缩和解压缩编码格式，该格式将用来解压缩在 HttpRequest 的响应中收到的数据
		/// </summary>
		enum class DecompressionMethod
		{
			// 摘要:
			//     不使用压缩。
			None = 0,
			//
			// 摘要:
			//     使用 gZip 压缩/解压缩算法。
			GZip,
			//
			// 摘要:
			//     使用 Deflate 压缩/解压缩算法。
			Deflate,
			//
			// 摘要:
			//     使用 gZip 或者 Deflate 压缩/解压缩算法。
			Auto
		};
	}
}
#endif