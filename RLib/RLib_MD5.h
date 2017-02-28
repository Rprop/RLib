/********************************************************************
	Created:	2012/04/14  20:13
	Filename: 	RLib_MD5.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_MD5
#define _USE_MD5
#include "RLib_CryptographyBase.h"
#include "RLib_String.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			/// <summary>
			/// 表示 MD5 哈希算法的实现
			/// </summary>
			class RLIB_API MD5
			{
			public:
				/// <summary>
				/// 计算指定字节数组的哈希值
				/// </summary>
				static void ComputeHash(const void *buffer, intptr_t count, OUT unsigned char output[16]);
				/// <summary>
				/// 计算指定长度字符串的字符串哈希值
				/// </summary>
				static String GetHashCode(const String &str);
			};
		}
	}
}
#endif //_USE_MD5