;.model flat
.code

__RTC_CheckEsp PROC
	ret
__RTC_CheckEsp ENDP

__security_check_cookie PROC ;_StackCookie
	ret
__security_check_cookie ENDP

_RTC_CheckStackVars PROC
	ret
_RTC_CheckStackVars ENDP

@_RTC_CheckStackVars@8 PROC ;SYSCALL
	ret
@_RTC_CheckStackVars@8 ENDP

END