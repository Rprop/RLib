/********************************************************************
	Created:	2014/06/30  18:54
	Filename: 	RLib_Object.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_OBJECT
#define _USE_OBJECT
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 表示封装对象基类
	/// </summary>
	template<class R> class Object
	{
	public:
		Object() : Object(nullptr) {}
		Object(R *ptr) {
			this->ObjectPtr = ptr;
		}

	public:
		/// <summary>
		/// 获取实例对象托管的指针
		/// </summary>
		operator R *() {
			return this->ObjectPtr;
		}
		/// <summary>
		/// 访问实例对象托管的指针
		/// </summary>
		R *operator->() const {
			return const_cast<R *>(this->ObjectPtr);
		}
		/// <summary>
		/// 获取实例对象引用
		/// </summary>
		R &GetInstance() {
			return *this->ObjectPtr;
		}
		/// <summary>
		/// Requests that the system not call the finalizer for the specified object
		/// </summary>
		R *SuppressFinalize() {
			R *obj = this->ObjectPtr;
			this->ObjectPtr = nullptr;
			return obj;
		}
		/// <summary>
		/// 判断关联的指针是否为 nullptr
		/// </summary>
		bool IsNull() {
			return this->ObjectPtr == nullptr;
		}
		/// <summary>
		/// 判断关联的指针是否不为 nullptr
		/// </summary>
		bool IsNotNull() {
			return this->ObjectPtr != nullptr;
		}

	public:
		R *ObjectPtr;
	};
	/// <summary>
	/// 提供内存块的自动释放方法
	/// </summary>
	template<typename R = unsigned char> class ManagedMemoryBlock : public Object<R>
	{
	public:
		using Object<R>::Object;
		explicit ManagedMemoryBlock(intptr_t count) {
			this->ObjectPtr = construct(count);
		}
		explicit ManagedMemoryBlock(uintptr_t count) : ManagedMemoryBlock(static_cast<intptr_t>(count)) {}
		~ManagedMemoryBlock() {
			this->Finalize();
		}
		RLIB_DECLARE_DYNCREATE;

	public:
		ManagedMemoryBlock(class ManagedMemoryBlock &src) {
			*this = src;
		}
		ManagedMemoryBlock& operator=(class ManagedMemoryBlock &src) {
			*this = src.SuppressFinalize();
			return *this;
		}

	public:
		/// <summary>
		/// 执行指针类型转换
		/// </summary>
		template <typename T = void> T *ToAny() {
			return reinterpret_cast<T *>(this->ObjectPtr);
		}
		/// <summary>
		/// 重置内存指针, 并释放之前的内存块
		/// </summary>
		ManagedMemoryBlock& operator=(R *ptr) {
			this->~ManagedMemoryBlock();
			RLIB_InitClass(this, ManagedMemoryBlock(ptr));
			return *this;
		}
		/// <summary>
		/// 回收内存
		/// </summary>
		void Finalize() {
			if (this->ObjectPtr != nullptr) {
				destroy(this->ObjectPtr);
				this->ObjectPtr = nullptr;
			} //if
		}

	public:
		static R *construct(intptr_t count) {
			return static_cast<R *>(RLIB_GlobalAlloc(RLIB_SIZEOF(R) * count));
		}
		static void destroy(const R *obj) {
			RLIB_GlobalCollectAny(obj);
		}
	};
	/// <summary>
	/// 提供对象指针的自动释放方法
	/// @warning 该类不支持互相赋值
	/// </summary>
	template<class R> class ManagedObject : public Object<R>
	{
	private:
		ManagedObject(const class ManagedObject &) = delete;
		ManagedObject& operator = (const class ManagedObject &) = delete;

	public:
		using Object<R>::Object;
		ManagedObject() : Object<R>() {}
		~ManagedObject() {
			this->Finalize();
		}

	public:
		/// <summary>
		/// 重置对象指针, 并释放之前的对象内存
		/// </summary>
		ManagedObject& operator=(R *ptr) {
			this->~ManagedObject();
			RLIB_InitClass(this, ManagedObject(ptr));
			return *this;
		}
		/// <summary>
		/// 调用对象终结器
		/// </summary>
		void Finalize() {
			if (this->ObjectPtr != nullptr) {
				destroy(this->ObjectPtr);
				this->ObjectPtr = nullptr;
			} //if
		}

	public:
		template<class... Args> static R *construct(Args&&... args) {
			return new R( Standard::forward<Args>(args)...);
		}
		static inline void destroy(const R *obj) {
			delete obj;
		}
	};
}
#endif // _USE_OBJECT