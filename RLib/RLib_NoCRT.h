#pragma once
//
// includes this file to your project and compiles if you don't use CRT
//
#ifdef _WIN64
typedef unsigned __int64 uintptr_t;
# pragma comment(lib, "ntdllp_x64.lib")
#else
typedef unsigned int uintptr_t;
# pragma comment(lib, "ntdllp_x86.lib")
#endif

#undef  RLIB_NOCRT
#define RLIB_NOCRT
#ifndef _WINDLL
# pragma comment(linker, "/ENTRY:__main")
#else
# pragma comment(linker, "/ENTRY:DllMain")
#endif // !_WINDLL

#ifndef RLIB_CHECK_STACK_TYPE
# define RLIB_CHECK_STACK_TYPE 2
#endif // !RLIB_CHECK_STACK_TYPE

# pragma comment(linker, "/NODEFAULTLIB:\"LIBCMTD.lib\"")

//-------------------------------------------------------------------------

#ifndef _WINDLL
void __main()
{
 	extern int main();
# ifdef RLIB_VER
	System::AppBase::Exit(static_cast<DWORD>(main()));
# else
#  pragma message("\t" __FILE__ "(23): warning C9999: exit the process explicitly")
# endif //RLIB_VER
}
#endif // !_WINDLL

//-------------------------------------------------------------------------

#ifdef _DEBUG

uintptr_t __security_cookie;

//-------------------------------------------------------------------------

extern "C" void _RTC_InitBase()
{

}

//-------------------------------------------------------------------------

extern "C" void _RTC_Shutdown()
{

}

//-------------------------------------------------------------------------

#ifdef _M_IX86
extern "C" __declspec(naked) void _RTC_CheckEsp()
{
	__asm ret;
}
extern "C" __declspec(naked) void  __fastcall _RTC_CheckStackVars(void *_Esp, void *_Fd)
{
	UNREFERENCED_PARAMETER(_Esp);
	UNREFERENCED_PARAMETER(_Fd);
	__asm ret;
}
#endif // _M_IX86

//-------------------------------------------------------------------------

extern "C" void __report_rangecheckfailure()
{
}

#endif // _DEBUG

//-------------------------------------------------------------------------

#ifdef _M_IX86
extern "C" __declspec(naked) void __fastcall __security_check_cookie(uintptr_t _StackCookie)
{
	UNREFERENCED_PARAMETER(_StackCookie);
	__asm ret;
}
#else
extern "C" void _cdecl __security_check_cookie(uintptr_t/* _StackCookie*/);
RLIB_TODO("add RLib_NoCRTx64.asm to your project");
#endif // _M_IX86