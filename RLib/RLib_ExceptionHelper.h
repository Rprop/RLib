/********************************************************************
	Created:	2016/06/30  22:39
	Filename: 	RLib_ExceptionHelper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#if _HAS_EXCEPTIONS

# define RAISE_EXCEPTION(e,a) { \
	auto ex = e; \
	a;           \
	throw ex;    \
}
# define BEGIN_TRY     try
# define END_TRY(et,a) catch(et *e){ \
	a;              \
	RLIB_Delete(e); \
}
# define END_TRY_EX(et,b,a) catch(et *e){ \
	b;              \
	RLIB_Delete(e); \
	a;              \
}
# define END_TRY_BREAK(et,a) catch(et *e){ \
	a;              \
	RLIB_Delete(e); \
	break;          \
}
# define END_TRY_RETURN(et,a) catch(et *e){ \
	a;              \
	RLIB_Delete(e); \
	return;         \
}
# define END_TRY_RETURN_V(et,a,v) catch(et *e){ \
	a;              \
	RLIB_Delete(e); \
	return v;       \
}

#endif // _HAS_EXCEPTIONS