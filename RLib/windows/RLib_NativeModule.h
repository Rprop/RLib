/********************************************************************
	Created:	2016/12/08  11:49
	Filename: 	RLib_NativeModule.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include <RLib_MemoryAllocator.h>

namespace System
{
	namespace Runtime
	{
		/// <summary>
		/// Provides a wide variety of members that support platform invoke services
		/// </summary>
		class RLIB_API Marshal
		{
		public:
			/// <summary>
			/// Loads the specified module
			/// </summary>
			static HMODULE LoadNativeModule(_In_ LPCWSTR lpModuleFileName);
			/// <summary>
			/// Unloads the specified module
			/// </summary>
			static void UnLoadNativeModule(_In_ HMODULE hModule);
			/// <summary>
			/// Returns the instance handle(HINSTANCE) for the specified module
			/// </summary>
			static HMODULE GetNativeModule(_In_opt_ LPCWSTR lpModuleName = nullptr,
										   _In_opt_ LPWSTR lpModulePath = nullptr);
			/// <summary>
			/// Gets the specified method pointer
			/// </summary>
			static LPVOID GetNativeMethod(_In_ HMODULE hModule,
										  _In_opt_ LPCSTR lpProcedureName,
										  _In_opt_ ULONG nProcedureNumber = 0);

		public:
			template<typename func_t> static func_t EvaluateJmp(func_t pTarget)
			{
				// 	0xE9    JMP   +  4 bytes offset
				// 	0xEB    JMP   +  2 bytes offset
				// 	0xFF25  JMP   +  4 bytes pointer of address
				// 	0xE8    CALL  +  4 bytes offset
				// 	0xFF15  CALL  +  4 bytes pointer of address
				// 	0x68    PUSH  +  4 bytes
				// 	0x6A    PUSH  +  1 bytes
				LPBYTE addr = reinterpret_cast<LPBYTE>(pTarget);
				do {
					switch (addr[0]) {
					case 0xe9:
						{
							addr = addr + 5 + *PLONG(addr + 1);
							break;
						}
					case 0xeb:
						{
							addr = addr + 3 + *PSHORT(addr + 1);
							break;
						}
					case 0xff:
						{
							if (addr[1] == 0x25) {
#ifdef _WIN64
								addr = reinterpret_cast<LPBYTE>(*PLONG_PTR(addr + 6 + *PLONG(addr + 2)));
#else 
								addr = reinterpret_cast<LPBYTE>(*PLONG_PTR(*PLONG(addr + 2)));
#endif // _WIN64
								break;
							} //if
						}
					default:
						pTarget = NULL;
					}
				} while (pTarget);
				return reinterpret_cast<func_t>(addr);
			}

			//-------------------------------------------------------------------------

			template<typename param_t = LPBYTE> 
			static void InstructionHotpatch(LPVOID pPatchTarget, SIZE_T patchSize,
									 void(*pCallback)(param_t), LPCVOID pData = NULL)
			{
				DWORD oldProtect = MemoryAllocator::Protect(AppBase::GetCurrentProcess(),
															pPatchTarget, patchSize, PAGE_EXECUTE_READWRITE);
				if (oldProtect == NULL) return; // failed

				pCallback(pData ? static_cast<param_t>(const_cast<LPVOID>(pData)) : static_cast<param_t>(pPatchTarget));

				MemoryAllocator::Protect(AppBase::GetCurrentProcess(),
										 pPatchTarget, patchSize, oldProtect);

				// Just-in-case measure.
				::FlushInstructionCache(AppBase::GetCurrentProcess(), pPatchTarget, patchSize);
			}
			
		public:
			/// <summary>
			/// Converts a delegate of a specified type to a function pointer that is callable
			/// </summary>
			template<typename func_t> static func_t GetFunctionPointerForDelegate(LPCVOID __imp_func)
			{
				return static_cast<func_t>(const_cast<LPVOID>(__imp_func));
			}
			
			//-------------------------------------------------------------------------

			template<typename func_t> static func_t Invoke(HMODULE hModule, LPSTR pName, HMODULE hModuleAlternatively = NULL)
			{
				LPVOID __imp_func = GetNativeMethod(hModule, pName);
				if (!__imp_func && hModuleAlternatively) {
					// for win7
					__imp_func = GetNativeMethod(hModuleAlternatively, pName);
				} //if
				return GetFunctionPointerForDelegate<func_t>(__imp_func);
			}

			//-------------------------------------------------------------------------
			
			template<typename func_t> static func_t InvokeCached(HMODULE hModule, LPSTR pName, HMODULE hModuleAlternatively = NULL)
			{
#ifdef _DEBUG
				static HMODULE cachedModule;
				static char    cachedName[64] = { 0 };
#endif // _DEBUG
				static LPVOID __imp_func = NULL;
				if (!__imp_func) {
					__imp_func = GetNativeMethod(hModule, pName);
#ifdef _DEBUG
					if (__imp_func != NULL) {
						cachedModule = hModule;
						strcpy_s(cachedName, RLIB_COUNTOF(cachedName), pName);
					} //if
#endif // _DEBUG
					if (!__imp_func && hModuleAlternatively) {
						__imp_func = GetNativeMethod(hModuleAlternatively, pName);
#ifdef _DEBUG
						if (__imp_func != NULL) {
							cachedModule = hModule;
							strcpy_s(cachedName, RLIB_COUNTOF(cachedName), pName);
						} //if
#endif // _DEBUG
					} //if
				} //if
#ifdef _DEBUG
				if (__imp_func != NULL) {
					assert(cachedModule == hModule);
					assert(_stricmp(cachedName, pName) == 0);
				} //if
#endif // _DEBUG
				return GetFunctionPointerForDelegate<func_t>(__imp_func);
			}
			
			//-------------------------------------------------------------------------
			
			template<typename func_t> static func_t Invoke(LPCTSTR szModule, LPSTR pName, LPCTSTR szModuleAlternatively = NULL)
			{
				return Invoke<func_t>(GetNativeModule(szModule), pName,
									  szModuleAlternatively ? GetNativeModule(szModuleAlternatively) : NULL);
			}

			//-------------------------------------------------------------------------

			template<typename func_t> static func_t InvokeCached(LPCTSTR szModule, LPSTR pName, LPCTSTR szModuleAlternatively = NULL)
			{
				return InvokeCached<func_t>(GetNativeModule(szModule), pName,
											szModuleAlternatively ? GetNativeModule(szModuleAlternatively) : NULL);
			}
		};
	}
}

//-------------------------------------------------------------------------

#define PInvoke(a, b)           System::Runtime::Marshal::Invoke<decltype(&b)>(a, RLIB_STRINGIZE(b))
#define PInvokeCached(a, b)     System::Runtime::Marshal::InvokeCached<decltype(&b)>(a, RLIB_STRINGIZE(b))
#define PInvokeX(a, b, c)       System::Runtime::Marshal::Invoke<decltype(&b)>(a, RLIB_STRINGIZE(b), c)
#define PInvokeXCached(a, b, c) System::Runtime::Marshal::InvokeCached<decltype(&b)>(a, RLIB_STRINGIZE(b), c)

