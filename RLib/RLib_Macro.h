
/*
 *	线程安全的标志
 *	该标志不影响代码行为
 */
#define RLIB_THREAD_SAFE

/*
 *	禁用关键字宏
 */
#define _ALLOW_KEYWORD_MACROS

/*
 *  NTSTATUS 成功标志
 */
#define STATUS_SUCCESS          0

/*
 *  是否禁止启用Native API支持
 *  使用该设置以适应不同的Windows版本
 */
#define RLIB_DISABLE_NATIVE_API 0

/*
 *	是否启用浮点支持
 *	必须启用, String::Format才能正常处理浮点数
 */
#define RLIB_HAS_FLOAT_SUPPORT  1

/*
 *  全局内存池配置 @TODO 保留大小可不一致
 */
# define RLIB_COMMITSIZE        1024 * 1024 * 1   /*默认提交MB*/
# define RLIB_ALLOCTRYCOUNT     128               /*内存随机分配失败重试次数*/
# define RLIB_FRAGTHRESHOLD     1024 * 4          /*碎片阀值kb*/
#ifdef _WIN64
# define RLIB_RESERVESIZE       1024 * 1024 * 256 /*默认保留MB*/          
# define RLIB_PAGECOUNT         64                /*默认维护的内存页数量*/
#else
# define RLIB_RESERVESIZE       1024 * 1024 * 80  /*默认保留MB*/          
# define RLIB_PAGECOUNT         18                /*默认维护的内存页数量*/   
#endif // _WIN64

/*
 *  定义换行符
 */
#ifdef _WINDOWS
# define RLIB_NEWLINE           _T("\r\n")
# define RLIB_NEWLINEA          "\r\n"
# define RLIB_NEWLINEW          L"\r\n"
#else // !_WINDOWS
# define RLIB_NEWLINE           _T("\n")
# define RLIB_NEWLINEA          "\n"
# define RLIB_NEWLINEW          L"\n"
#endif // _WINDOWS

/*
 *	内部定义
 */
#define RLIB_INTERNAL_TO_STRING(x)   #x
#define RLIB_INTERNAL_EXCEPTION(e,i) class RLIB_API e : public i \
									 { \
									 public: \
										 e() : i() {} \
										 e(const TCHAR *description, INT id = -1) : i(description, id) {} \
										 RLIB_DECLARE_DYNCREATE; \
								 	 public: \
										 LPCTSTR ToString() { \
											 return _T(#i) _T("::") _T(#e); \
									 	 } \
									 }
#ifdef _DEBUG
# define RLIB_INTERNAL_DEBUG_PARAM         , LPCTSTR function, LPCTSTR file
# define RLIB_INTERNAL_DEBUG_PARAM_VALUE   , function, file
# define RLIB_INTERNAL_DEBUG_PARAM_HERE    , RLIB_FUNCTION, RLIB_FILE _T(":") RLIB_LINE
#else
# define RLIB_INTERNAL_DEBUG_PARAM
# define RLIB_INTERNAL_DEBUG_PARAM_VALUE
# define RLIB_INTERNAL_DEBUG_PARAM_HERE
#endif // _DEBUG
#ifdef RLIB_BUILDING
# define RLIB_INTERNAL_CALC_ON_NEED        (-RLIB_COMPILE_LINE)
# define RLIB_INTERNAL_BLOCK_SIZE          RLIB_SIZEOF(BLOCK_INFO)                 
# define RLIB_INTERNAL_USER_TO_HANDLE(p)   reinterpret_cast<PBLOCK_INFO>(                                       \
										   const_cast<char *>(reinterpret_cast<const char *>(p) - RLIB_INTERNAL_BLOCK_SIZE) \
										   )
# define RLIB_INTERNAL_HANDLE_TO_USER(p)   const_cast<char *>(reinterpret_cast<const char *>(p) + RLIB_INTERNAL_BLOCK_SIZE)
/*
 *	仅供 MemoryPool 及其派生类和衍生类使用
 */
# define RLIB_INTERNAL_USER_TO_HEADER(p)   reinterpret_cast<PMEMBLOCK>(                   \
										   reinterpret_cast<LPBYTE>(p) - sizeof(MEMORY_BLOCK_HEADER) \
										   )
/*
 *	仅供 String 及其派生类和衍生类使用
 */
# define RLIB_INTERNAL_ALLOCATE_STRING(size)           RLIB_GlobalAllocAny(TCHAR *, RLIB_ALIGNMENT + size)
# define RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(size) static_cast<InternalString *>(RLIB_GlobalAlloc(RLIB_ALIGNMENT + size))
#endif // RLIB_BUILDING

/*
 *  编译器指令
 */
// Obsolete function or variable
#define RLIB_DEPRECATED               __declspec(deprecated("This function or variable has been deprecated"))
// COMDAT 折叠
#define RLIB_ICF                      __declspec(selectany)
#define RLIB_THREAD                   __declspec(thread)
#define RLIB_NAKED                    __declspec(naked)
#define RLIB_NO_ALIAS                 __declspec(noalias)
#define RLIB_NO_VTABLE                __declspec(novtable)
#define RLIB_NO_RETURN                __declspec(noreturn)
#define RLIB_RESTRICT_RETURN          __declspec(restrict)
#define RLIB_PROPERTY_GET(a,g)        __declspec(property(get = g)) a
#define RLIB_PROPERTY_SET(a,p)        __declspec(property(put = p)) a
#define RLIB_PROPERTY_GET_SET(a,g,p)  __declspec(property(get = g, put = p)) a
#define RLIB_ALIGN(n)                 __declspec(align(n))
#define RLIB_PACK(n)			      __pragma(pack(n))
#define RLIB_WARNING_PUSH             __pragma(warning(push))
#define RLIB_WARNING_DISABLE(v)       __pragma(warning(disable:v))
#define RLIB_WARNING_POP              __pragma(warning(pop))
#define RLIB_IMPORT_LIB(a)            __pragma(comment(lib, a))
#define RLIB_NO_INLINE                __declspec(noinline)
#define RLIB_ALIGNOF(a)               __alignof(a)
#define RLIB_RESTRICT                 __restrict
#define RLIB_INLINE		              __inline
#define RLIB_FORCE_INLINE             __forceinline
#define RLIB_ASSUME(e)                __assume(e)
#define RLIB_TYPE(v)                  decltype(v)
#define RLIB_REF_AS(a,t)              *reinterpret_cast<t *>(&a)
#define RLIB_RENAME(a,b)              RLIB_TYPE(a) &b = reinterpret_cast<RLIB_TYPE(a) &>(a)
#define RLIB_STRINGIZE(x)             RLIB_INTERNAL_TO_STRING(x)
#define RLIB_COUNTER                  __COUNTER__
// 00:00:00
#define RLIB_COMPILE_TIME             __TIME__
// Jul 12 2016
#define RLIB_COMPILE_DATE             __DATE__
// Sat Jan 01 00:00:00 2015
#define RLIB_COMPILE_TIMESTAMP        __TIMESTAMP__
#define RLIB_COMPILE_LINE             __LINE__
#define RLIB_COMPILE_FUNCTION         __FUNCTION__
#define RLIB_WARNING(s)               "\t" __FILE__ "(" RLIB_STRINGIZE(RLIB_COMPILE_LINE) "): warning C9999: " s

/*
 *	编译器优化提示
 */
#ifdef _DEBUG
# define RLIB_NODEFAULT(e)            trace(!(e))
#else
# define RLIB_NODEFAULT(e)            __assume(0)
#endif // _DEBUG

/*
 *  定义编译方式
 */
#ifdef RLIB_LIB
# define RLIB_API
extern int __stdcall RLib_Init(_In_ void *_DllHandle, _In_ unsigned long _Reason);
#else
# ifdef RLIB_BUILDING
#  define RLIB_API                    __declspec(dllexport) 
# else
#  define RLIB_API                    __declspec(dllimport)
# endif // RLIB_BUILDING
#endif // RLIB_LIB

/*
 *  提供调试辅助
 */
#undef assert
#ifdef _DEBUG
extern void RLIB_API __rlib_alert(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);
# define alert(str) { \
	__rlib_alert(str, RLIB_FILE, RLIB_FUNCTION, RLIB_LINE); \
}
# define trace(expression) (void)(                                                          \
            (!!(expression)) ||                                                             \
            (__rlib_alert(_T(#expression), RLIB_FILE, RLIB_FUNCTION, RLIB_LINE), 0) \
)
# define assert(expression) (void)(                                                         \
            (!!(expression)) ||                                                             \
            (__rlib_alert(_T(#expression), RLIB_FILE, RLIB_FUNCTION, RLIB_LINE), 0) \
)
# define mask_thread_begin() Threading::Thread::SetThreadName(AppBase::GetCurrentThreadId(), __FUNCTION__)
#else
# define alert(str)          ((void)0)
# define trace(_Expression)  ((void)0)
# define assert(_Expression) ((void)0)
# define mask_thread_begin() ((void)0)
#endif // _DEBUG

/*
 *	#pragma todo("")
 */
#define todo(s)      message(RLIB_WARNING(s))
#define RLIB_TODO(s) __pragma(message(RLIB_WARNING(s)))

 /*
  *	Platform dependent alignment
  */
#define RLIB_ALIGNMENT  static_cast<intptr_t>(sizeof(intptr_t))
#ifdef _WIN64
# define RLIB_ROUNDUP   Utility::round_up_8
#else
# define RLIB_ROUNDUP   Utility::round_up_4
#endif // _WIN64

/*
 *  全局内存池操作
 */
#ifdef _DEBUG
# define RLIB_PageAlloc(p,n)			p.AllocByte(n RLIB_INTERNAL_DEBUG_PARAM_HERE)
# define RLIB_GlobalAlloc(n)            RLIB_GlobalAllocDebug(n, RLIB_INTERNAL_DEBUG_PARAM_HERE)
# define RLIB_GlobalAllocDebug(n,a)     System::AppBase::Allocate(n a)
#else
# define RLIB_PageAlloc(p,n)            p.AllocByte(n)
# define RLIB_GlobalAlloc(n)            System::AppBase::Allocate(n)
# define RLIB_GlobalAllocDebug(n,a)     RLIB_GlobalAlloc(n)
#endif // _DEBUG
#define RLIB_GlobalAllocObj(o,n)        reinterpret_cast<o *RLIB_RESTRICT>(RLIB_GlobalAlloc(RLIB_SIZEOF(o) * n))
#define RLIB_GlobalAllocAny(t,n)        reinterpret_cast<t RLIB_RESTRICT>(RLIB_GlobalAlloc(static_cast<intptr_t>(n)))
#define RLIB_GlobalReAlloc(p,n)         System::AppBase::Reallocate(p, n)
#define RLIB_GlobalCollect(p)           System::AppBase::Collect(p)
#define RLIB_GlobalCollectAny(p)        RLIB_GlobalCollect(const_cast<void *>(reinterpret_cast<const void *>(p)))

/*
 *	在全局内存池分配可容纳指定长度字符串的缓冲区
 */
#define RLIB_FastAllocateString(length) RLIB_GlobalAllocAny(TCHAR *, TSIZE(length));

/*
 *  字符串自适应宏
 */
#ifdef _UNICODE
# define RLIB_FILE      _CRT_WIDE(__FILE__)
# define RLIB_FUNC      _CRT_WIDE(__func__)
# define RLIB_FUNCTION  _CRT_WIDE(__FUNCTION__)
# define RLIB_FUNCDNAME _CRT_WIDE(__FUNCDNAME__)
# define RLIB_FUNCSIG   _CRT_WIDE(__FUNCSIG__)
# define RLIB_LINE      _T(RLIB_STRINGIZE(RLIB_COMPILE_LINE))
#else
# define RLIB_FILE      __FILE__
# define RLIB_FUNC      __func__
# define RLIB_FUNCTION  __FUNCTION__
# define RLIB_FUNCDNAME __FUNCDNAME__
# define RLIB_FUNCSIG   __FUNCSIG__
# define RLIB_LINE      RLIB_STRINGIZE(RLIB_COMPILE_LINE)
#endif // _UNICODE

/*
 *  异常填充方法
 */
#define  RLIB_EXCEPTION_BASEID        (0L)
#define  RLIB_EXCEPTION_ID            (RLIB_EXCEPTION_BASEID - RLIB_COMPILE_LINE)
#ifdef _DEBUG
# define RLIB_SetException(a, i, s)   a.Set(i, s, RLIB_FILE, RLIB_FUNCSIG)
# define RLIB_SetExceptionInfo(a)     a.SetDebugInfo(RLIB_FILE, RLIB_FUNCSIG)
#else
# define RLIB_SetException(a, i, s)   a.Set(i, s)
# define RLIB_SetExceptionInfo(a)     void(0)
#endif // _DEBUG

/*
 *	max. length of full pathname
 */
# define RLIB_MAX_PATH               256

/*
 *  String::Format内部缓冲区大小, in TCHARs
 */
#define RLIB_STRING_FORMAT_LENGTH    256

/*
 *  存储异常信息的缓冲区长度, in TCHARs
 */
#define RLIB_EXCEPTION_MSG_LENGTH    192

 /*
  *  default container capacity
  */
#define RLIB_DEFAULT_CAPACITY        16

/*
 *  默认缓冲区长度
 */
#define RLIB_DEFAULT_LENGTH          256

/*
 *  默认内部缓冲区大小
 */
#define RLIB_DEFAULT_BUFFER_SIZE     1024
#define RLIB_DEFAULT_MAX_BUFFER_SIZE 4096

/*
 *  网络缓冲区大小
 */
#define RLIB_NETWORK_BUFFER_SIZE     8192

/*
 *	Enables the ability for objects of the classes to be created dynamically at run time.
 */
#define RLIB_DECLARE_DYNCREATE        void *operator new(size_t size) { return RLIB_GlobalAlloc(static_cast<intptr_t>(size)); } void operator delete(void *p) { RLIB_GlobalCollect(p); } void *operator new[](size_t size) { return RLIB_GlobalAlloc(static_cast<intptr_t>(size)); } void operator delete[](void *p) { RLIB_GlobalCollect(p); }
#define RLIB_IMPLEMENT_DYNCREATE(cls) void *cls::operator new(size_t size) { return RLIB_GlobalAlloc(static_cast<intptr_t>(size)); } void cls::operator delete(void *p) { RLIB_GlobalCollect(p); } void *cls::operator new[](size_t size) { return RLIB_GlobalAlloc(static_cast<intptr_t>(size)); } void cls::operator delete[](void *p) { RLIB_GlobalCollect(p); }

/*
 *	Generates the C++ code necessary for objects class that can be serialized.(unsupported)
 */
#define RLIB_DECLARE_SERIAL
#define RLIB_IMPLEMENT_SERIAL(cls)

/*
 *	determine number of elements in an array (not bytes)
 */
#define RLIB_COUNTOF(a)              static_cast<intptr_t>(sizeof(a) / sizeof(a[0]) + 0)
#define RLIB_COUNTOF_STR(s)          static_cast<intptr_t>(RLIB_COUNTOF(s) - 1)
#define RLIB_SIZEOF(a)               static_cast<intptr_t>(sizeof(a))

/*
 *	useful macro
 */
#define RLIB_BUFFER_SIZE(o)          o, sizeof(o)
#define RLIB_BUFFERT_SIZE(o,t)       reinterpret_cast<t>(o), sizeof(o)
#define RLIB_BUF_SIZE(o)             RLIB_BUFFER_SIZE(o)
#define RLIB_BUFT_SIZE(o,t)          RLIB_BUFFERT_SIZE(o)
#define RLIB_STR_SIZE(o)             o, sizeof(o) - sizeof(o[0])
#define RLIB_STRT_SIZE(o,t)          reinterpret_cast<t>(o), (sizeof(o) - sizeof(o[0]))
#define RLIB_STR_LEN(o)              o, RLIB_COUNTOF_STR(o)
#define RLIB_STRT_LEN(o,t)           reinterpret_cast<t>(o), RLIB_COUNTOF_STR(o)

/*
 *	leading bytes comparison
 */
#define RLIB_CMP_BYTE(a,b)           *reinterpret_cast<const unsigned char *>(a) == *reinterpret_cast<const unsigned char*>(b)
#define RLIB_CMP_SHORT(a,b)	         *reinterpret_cast<const short *>(a) == *reinterpret_cast<const short *>(b)
#define RLIB_CMP_SHORT_BYTE(a,b)     RLIB_CMP_SHORT(a,b) && RLIB_CMP_BYTE(reinterpret_cast<const unsigned char *>(a) + 2, reinterpret_cast<const unsigned char *>(b) + 2)
#define RLIB_CMP_INT(a,b)            *reinterpret_cast<const int *>(a) == *reinterpret_cast<const int *>(b)
#define RLIB_CMP_LONGLONG(a,b)       *reinterpret_cast<const long long *>(a) == *reinterpret_cast<const long long *>(b)

/*
 *	selects max/min value
 */
#define RLIB_MAX(a,b)                (((a) > (b)) ? (a) : (b))
#define RLIB_MIN(a,b)                (((a) < (b)) ? (a) : (b))

/*
 *	(assert(b <= c), a >= b && a <= c)
 */
#define RLIB_INRANGE_ANY(a,b,c,t,v)  (assert((v)(b) <= (v)(c)), (t)(a) - (t)(b) <= (t)(c) - (t)(b))
#define RLIB_INRANGE32(a,b,c)        RLIB_INRANGE_ANY(a, b, c, unsigned int, int)
#define RLIB_INRANGE64(a,b,c)        RLIB_INRANGE_ANY(a, b, c, unsigned __int64, __int64)
#define RLIB_INRANGE32U(a,b,c)       RLIB_INRANGE_ANY(a, b, c, unsigned int, unsigned int)
#define RLIB_INRANGE64U(a,b,c)       RLIB_INRANGE_ANY(a, b, c, unsigned __int64, unsigned __int64)
#ifdef _WIN64
# define RLIB_INRANGE(a,b,c)         RLIB_INRANGE64(a, b, c)
# define RLIB_INRANGEU(a,b,c)        RLIB_INRANGE64U(a, b, c)
#else
# define RLIB_INRANGE(a,b,c)         RLIB_INRANGE32(a, b, c)
# define RLIB_INRANGEU(a,b,c)        RLIB_INRANGE32U(a, b, c)
#endif // _WIN64

 /*
  *	equals (a >= b && a < c)
  * (b < v) required
  */
#define RLIB_ARRAYRANGE_ANY(a,b,c,t,v) (assert((v)(b) < (v)(c)), (t)(a) - (t)(b) <= ((t)(c) - 1) - (t)(b))
#define RLIB_ARRAYRANGE32(a,b,c)       RLIB_ARRAYRANGE_ANY(a, b, c, unsigned int, int)
#define RLIB_ARRAYRANGE64(a,b,c)       RLIB_ARRAYRANGE_ANY(a, b, c, unsigned __int64, __int64)
#define RLIB_ARRAYRANGE32U(a,b,c)      RLIB_ARRAYRANGE_ANY(a, b, c, unsigned int, unsigned int)
#define RLIB_ARRAYRANGE64U(a,b,c)      RLIB_ARRAYRANGE_ANY(a, b, c, unsigned __int64, unsigned __int64)
#ifdef _WIN64
# define RLIB_ARRAYRANGE(a,b,c)        RLIB_ARRAYRANGE64(a, b, c)
# define RLIB_ARRAYRANGEU(a,b,c)       RLIB_ARRAYRANGE64U(a, b, c)
#else
# define RLIB_ARRAYRANGE(a,b,c)        RLIB_ARRAYRANGE32(a, b, c)
# define RLIB_ARRAYRANGEU(a,b,c)       RLIB_ARRAYRANGE32U(a, b, c)
#endif // _WIN64

/*
 *	Create an instance of the object by calling the default constructor
 */
#define RLIB_DEFAULT_VAR(t)          t()

/*
 *	Delay to call constructor of a class
 */
#define RLIB_DELAY(t,a)              RLIB_DELAY_DATA(t,__delay_##a);t &a = *reinterpret_cast<t *>(__delay_##a)
#define RLIB_DELAY_INIT(t,d,c)       RLIB_InitClass(reinterpret_cast<t *>(d), c)
#define RLIB_DELAY_DESTROY(t,d)      reinterpret_cast<t *>(d)->~t()
#define RLIB_DELAY_ALIAS(t,d,a)      auto a = reinterpret_cast<t *>(d)
#define RLIB_DELAY_DATA(t,d)	     char d[RLIB_ROUNDUP(sizeof(t))]

/*
 *  为指定内存调用相应的类构造函数
 */
#define RLIB_InitClass(p, c)         (::new (p) c)
#define RLIB_Delete(a)               if(a != nullptr) { delete a; }

/*
 *	
 */
#define RLIB_SET_BIT_AT(v, bit)      (v |= (1 << bit))
#define RLIB_CLR_BIT_AT(v, bit)      (v &= ~(1 << bit))
#define RLIB_IS_POWER_OF_TWO(x)      (((x) != 0) && (((x) & ((x) - 1)) == 0))

/*
 *	Inline byte flipping -- can be done in registers
 */
#define RLIB_WORD_FLIP(out, in)     \
        {                           \
            WORD _in = WORD(in);    \
            (out)    = WORD((_in << 8) | (_in >> 8)); \
        }
#define RLIB_DWORD_FLIP(out, in)    \
        {                           \
            DWORD _in = DWORD(in);  \
            (out) = DWORD(((_in << 8) & 0x00ff0000) | \
                    (_in << 24)                | \
                    ((_in >> 8) & 0x0000ff00)  | \
                    (_in >> 24));                \
        }
#if RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN
# define RLIB_HTONS(out, in)   RLIB_WORD_FLIP(out, in)
# define RLIB_NTOHS(out, in)   RLIB_WORD_FLIP(out, in)
# define RLIB_NTOHL(out, in)   RLIB_DWORD_FLIP(out, in) // _byteswap_ulong
# define RLIB_HTONL(out, in)   RLIB_DWORD_FLIP(out, in) // _byteswap_ulong
#else
# define RLIB_HTONS(out, in)   (out) = (in)
# define RLIB_NTOHS(out, in)   (out) = (in)
# define RLIB_NTOHL(out, in)   (out) = (in)
# define RLIB_HTONL(out, in)   (out) = (in)
#endif // RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN

/*
 *	流辅助函数
 */
#define RLIB_StreamWriteString(p,a)  p->Write(a.GetConstData(), a.CanReadSize)
#define RLIB_StreamWriteA(p,a)       p->Write(a, RLIB_SIZEOF(a) - RLIB_SIZEOF(char))
#define RLIB_StreamWriteW(p,a)       p->Write(L##a, RLIB_SIZEOF(L##a) - RLIB_SIZEOF(wchar_t))
#define RLIB_StreamWriteStringA(p,a) {          \
	GlobalizeString u(a);                       \
	p->Write(u.toGBK(), u.sizeofGBK());         \
}
#define RLIB_StreamWriteStringW(p,a) {          \
	GlobalizeString u(a);                       \
	p->Write(u.toUnicode(), u.sizeofUnicode()); \
}
#ifdef _UNICODE
# define RLIB_StreamWrite(p,a)       RLIB_StreamWriteW(p,a)
#else
# define RLIB_StreamWrite(p,a)       RLIB_StreamWriteA(p,a)
#endif // _UNICODE
