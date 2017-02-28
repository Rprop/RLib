/*
www.sourceforge.net/projects/tinyxml
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
// Deprecated library function hell. Compilers want to use the
// new safe versions. This probably doesn't fully address the problem,
// but it gets closer. There are too many compilers for me to fully
// test. If you get compilation troubles, undefine RLIBXML_SAFE
#define RLIBXML_SAFE

#ifdef RLIBXML_SAFE
#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
// Microsoft visual studio, version 2005 and higher.
#define RLIBXML_SNPRINTF _sntprintf_s
#define RLIBXML_SSCANF   _stscanf_s
#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
// Microsoft visual studio, version 6 and higher.
//#pragma message( "Using _sn* functions." )
#define RLIBXML_SNPRINTF _sntprintf
#define RLIBXML_SSCANF   _stscanf
#elif defined(__GNUC__) && (__GNUC__ >= 3 )
// GCC version 3 and higher.s
//#warning( "Using sn* functions." )
#define RLIBXML_SNPRINTF snprintf
#define RLIBXML_SSCANF   _stscanf
#else
#define RLIBXML_SNPRINTF snprintf
#define RLIBXML_SSCANF   _stscanf
#endif
#endif	

const int RLIBXML_MAJOR_VERSION = 2;
const int RLIBXML_MINOR_VERSION = 6;
const int RLIBXML_PATCH_VERSION = 2;

// Only used by Attribute::Query functions
enum 
{ 
	RLIBXML_SUCCESS,
	RLIBXML_NO_ATTRIBUTE,
	RLIBXML_WRONG_TYPE
};
