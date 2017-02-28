/********************************************************************
	Created:	2015/01/20  13:50
	Filename: 	RLib_Tls.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#if !(defined _USE_TLS) && !(defined _DISABLE_TLS)
#define _USE_TLS
#include "RLib_Threading.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		/// <summary>
		/// 表示线程局部存储地址量
		/// </summary>
		typedef unsigned long TlsId;
		/// <summary>
		/// 表示线程局部存储对象
		/// </summary>
		class RLIB_API TlsItem
		{
		public:
			TlsId tlsIndex;
		public:
			TlsItem(TlsId tlsIndex) {
				this->tlsIndex = tlsIndex;
			}
			~TlsItem() = default; // TLS_OUT_OF_INDEXES
			RLIB_DECLARE_DYNCREATE;
		public:
			void *GetValue() const;			
			void SetValue(const void *);
			RLIB_PROPERTY_GET_SET(void *Value, GetValue, SetValue);
		public:
			operator void *() const {
				return this->GetValue();
			}
			const class TlsItem& operator=(const void *obj) {
				this->SetValue(obj);
				return *this;
			}
		};
		/// <summary>
		/// 表示线程局部存储接口
		/// </summary>
		class RLIB_API Tls
		{
		public:
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 分配线程局部变量存储地址量
			/// 方法在主线程或者初始化线程中被调用
			/// </summary>
			static TlsItem AllocateLocallyUniqueId();
			/// <summary>
			/// 回收分配的存储地址量
			/// </summary>
			static void Finalize(TlsId);
			/// <summary>
			/// 回收分配的存储地址量
			/// </summary>
			static void Finalize(TlsItem &tlsItem) {
				Tls::Finalize(tlsItem.tlsIndex);
			}
			/// <summary>
			/// 获取线程局部存储对象
			/// </summary>
			static TlsItem GetAt(TlsId tlsIndex) {
				return TlsItem(tlsIndex);
			}

		public:
			TlsItem operator [](TlsId tlsIndex) const {
				return this->GetAt(tlsIndex);
			}
		};
	}
}

#endif // _USE_TLS