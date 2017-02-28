/********************************************************************
	Created:	2015/01/20  13:35
	Filename: 	RLib_Helper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HELPER
#define _USE_HELPER
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 利用析构函数自动调用终结器
	/// </summary>
	template<typename T = void *> class AutoFinalize
	{
	public:
		void(*finalizer)(T);
		T argv;
	public:
		AutoFinalize(void(*finalizer)(T), const T argv){
			this->finalizer = finalizer;
			this->argv      = const_cast<T>(argv);
		}
		~AutoFinalize(){
			this->finalizer(this->argv);
		}
	};
	/// <summary>
	/// 利用静态对象初始化实现自动初始化
	/// </summary>
	template<typename T = void *> class AutoRunOnce
	{
	public:
#pragma optimize("", off)
		AutoRunOnce(void(*initializer)(T), const T argv) {
			initializer(const_cast<T>(argv));
		}
#pragma optimize("", on)
	};
	/// <summary>
	/// 提供一组利用对象构造/析构来自动进入/退出临界区的方法
	/// </summary>
	template<class T = Threading::Monitor> class RLIB_THREAD_SAFE AutoLock
	{
	public:
		AutoLock(T &locker) : AutoLock(&locker) {}
		AutoLock(T *lplocker) {
			this->m_plocker = lplocker;
			this->m_plocker->Enter();
		}
		~AutoLock() {
			this->m_plocker->Exit();
		}
	protected:
		T *m_plocker;
	};
};

#pragma warning(push)
#pragma warning(disable:4640) // for local static object
#define RLIB_STATIC(c)         static AutoRunOnce<void *> __runonce([](void *) {c}, nullptr)
#define RLIB_STATIC_EX(c, p)   static AutoRunOnce<void *> __runonce_ex([](void *) {c}, const_cast<void *>(p))
#define RLIB_FINALIZE(c)       AutoFinalize<void *> __finalizer([](void *) {c}, nullptr)
#define RLIB_FINALIZE_EX(c, p) AutoFinalize<void *> __finalizer_ex([](void *) {c}, const_cast<void *>(p))
//#pragma warning(pop)

#endif // _USE_HELPER