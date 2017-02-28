/********************************************************************
	Created:	2014/07/01  14:50
	Filename: 	RLib_CryptographyBase.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_CRYP_BASE
#define _USE_CRYP_BASE

#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 提供安全系统的基础结构
	/// </summary>
	namespace Security
	{
		/// <summary>
		/// 提供加密服务，包括安全的数据编码和解码，以及许多其他操作
		/// </summary>
		namespace Cryptography
		{
			/// <summary>
			/// 公开统一的内存管理接口
			/// </summary>
			class RLIB_API RLIB_THREAD_SAFE CryptographyBase
			{
//			protected:
// 				static void *Alloc(ULONG size)
// 				{
// 					return RLIB_GlobalAlloc(size);
// 				}

			public:
				static void Collect(LPVOID ptr_data)
				{
					RLIB_GlobalCollect(ptr_data);
				}
			};
		}
	}
}
#endif // _USE_CRYP_BASE