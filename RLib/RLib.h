/********************************************************************
	Created:	2010/03/10  13:37
	Filename: 	RLib.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#ifndef _RLIB
#define _RLIB
#define RLIB_VER 5.7

#ifndef _DEBUG
# undef  NDEBUG
# define NDEBUG
#endif // _DEBUG

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && defined(_M_IX86)
# define _X86_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && defined(_M_AMD64)
# define _AMD64_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && defined(_M_ARM)
# define _ARM_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && !defined(_ARM_) && defined(_M_IA64)
# if !defined(_IA64_)
# define _IA64_
# endif /* !_IA64_ */
#endif

#define RLIB_LITTLE_ENDIAN   1234    // least-significant byte first (vax)
#define RLIB_BIG_ENDIAN      4321    // most-significant byte first (IBM, net)
#define RLIB_PDP_ENDIAN      3412    // LSB first in word, MSW first in long (pdp)

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(MIPSEL) || \
    defined(BIT_ZERO_ON_RIGHT)
# define RLIB_BYTE_ORDER RLIB_LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined (BIT_ZERO_ON_LEFT)
# define RLIB_BYTE_ORDER RLIB_BIG_ENDIAN
#endif

#ifndef RLIB_BYTE_ORDER      // still not defined
# if defined(u3b2) || defined(m68k)
#  define RLIB_BYTE_ORDER RLIB_BIG_ENDIAN
# endif
# if defined(i286) || defined(i386) || defined(_AMD64_) || defined(_IA64_) || defined(_ARM_)
#  define RLIB_BYTE_ORDER RLIB_LITTLE_ENDIAN
# endif

# ifndef RLIB_BYTE_ORDER
#  define RLIB_BYTE_ORDER RLIB_LITTLE_ENDIAN
# endif

#endif

/*
 * Disables inessential warnings
 */
#pragma warning(disable:4668 4820 4265 4061 4062 4710 4711 4464 5031 5032)
//The following warnings are turned off by default.
// C4668 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
// C4820 'bytes' bytes padding added after construct 'member_name'
// C4265 'class': class has virtual functions, but destructor is not virtual
// C4061 enumerator 'identifier' in a switch of enum 'enumeration' is not explicitly handled by a case label
// C4062 enumerator 'identifier' in a switch of enum 'enumeration' is not handled
// C4710 function not inlined
// C4711 function selected for automatic inline expansion
// C4464 #include: use of parent-directory specifier '..' in pathname (only affects /Wall /WX)
// C5031,C5032 Mismatched #pragma warning(push) and #pragma warning(pop)
#if VSVER <= 120
# pragma warning(default:4616 4619)
#endif // VSVER <= 120

#ifdef RLIB_BUILDING
# undef  _HAS_EXCEPTIONS
# define _HAS_EXCEPTIONS 0
#endif // RLIB_BUILDING

#undef WINVER
#ifdef _USING_V110_SDK71_ 
// _WIN32_WINNT_WINXP
# define WINVER 0x501
#else 
// _WIN32_WINNT_WIN8
# define WINVER 0x0602
#endif // _USING_V110_SDK71_

#pragma warning(push)
#pragma warning(disable:4514) // unreferenced inline function has been removed
#if (WINVER > 0x601)
# include <minwindef.h>
#else
# include <windef.h>
# pragma warning(push)
# pragma warning(disable:4201)
# include <winbase.h>
# pragma warning(pop)
#endif
#include "RLib_Config.h"
#include "RLib_Macro.h"
#include "RLib_ExternDefine.h"
#pragma warning(pop)

#endif // _RLIB