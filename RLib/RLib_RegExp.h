/********************************************************************
	Created:	2012/04/03  16:47
	Filename: 	RLib_RegExp.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
	Used:       DEELX Regular Expression Engine (v1.2)
*********************************************************************/
#include "RLib_String.h"
#include "RLib_MemoryPool.h"

#if !(defined _USE_REGEXP) && !(defined _DISABLE_REGEXP)
#define _USE_REGEXP

//-------------------------------------------------------------------------

namespace System
{
	namespace Text
	{
		/// <summary>
		/// 包含一些类，这些类提供对正则表达式引擎的访问
		/// </summary>
		namespace RegularExpressions
		{
			/// <summary>
			/// C++ 环境下的与 Perl 兼容的正则表达式引擎
			/// </summary>
			namespace Deelx
			{
				#include "support/deelx.h"
			}
			class RLIB_API Builder
			{
			public:
				typedef Deelx::CDelegateElxT  <TCHAR> CDelegateElx;
				typedef Deelx::CBracketElxT   <TCHAR> CBracketElx;
				typedef Deelx::CBackrefElxT   <TCHAR> CBackrefElx;
				typedef Deelx::CConditionElxT <TCHAR> CConditionElx;

				// Methods
			public:
				Deelx::ElxInterface * Build(const Deelx::RefBuffer <TCHAR> & pattern, int flags);
				int GetNamedNumber(const Deelx::RefBuffer <TCHAR> & named) const;
				void Clear();

			public:
				Builder();
				~Builder();
				RLIB_DECLARE_DYNCREATE;
				// Public Attributes
			public:
				Deelx::ElxInterface * m_pTopElx;
				int            m_nFlags;
				int            m_nMaxNumber;
				int            m_nNextNamed;
				int            m_nGroupCount;

				Deelx::Buffer <Deelx::ElxInterface *> m_objlist;
				Deelx::Buffer <Deelx::ElxInterface *> m_grouplist;
				Deelx::Buffer <Deelx::CListElx     *> m_namedlist;
				Deelx::Buffer <CDelegateElx  *> m_recursivelist;
				Deelx::Buffer <CBackrefElx   *> m_namedbackreflist;
				Deelx::Buffer <CConditionElx *> m_namedconditionlist;

				// TCHAR_INFO
			protected:
				struct RLIB_API TCHAR_INFO
				{
				public:
					TCHAR ch;
					int   type;
					int   pos;
					int   len;

				public:
					TCHAR_INFO(TCHAR c, int t, int p = 0, int l = 0) { ch = c; type = t; pos = p; len = l;    }
					RLIB_INLINE int operator == (const TCHAR_INFO & ci)   { return ch == ci.ch && type == ci.type; }
					RLIB_INLINE int operator != (const TCHAR_INFO & ci)   { return ! operator == (ci);             }
				};

			protected:
				static unsigned int Hex2Int(const TCHAR * pcsz, int length, int & used);
				static int ReadDec(char * & str, unsigned int & dec);
				void MoveNext();
				int  GetNext2();

				Deelx::ElxInterface * BuildAlternative(int vaflags);
				Deelx::ElxInterface * BuildList       (int & flags);
				Deelx::ElxInterface * BuildRepeat     (int & flags);
				Deelx::ElxInterface * BuildSimple     (int & flags);
				Deelx::ElxInterface * BuildCharset    (int & flags);
				Deelx::ElxInterface * BuildRecursive  (int & flags);
				Deelx::ElxInterface * BuildBoundary   (int & flags);
				Deelx::ElxInterface * BuildBackref    (int & flags);

				Deelx::ElxInterface * GetStockElx     (int nStockId);
				Deelx::ElxInterface * Keep(Deelx::ElxInterface * pElx);

				// Private Attributes
			protected:
				Deelx::RefBuffer<TCHAR> m_pattern;
				TCHAR_INFO prev, curr, next, nex2;
				int m_nNextPos;
				int m_nCharsetDepth;
				int m_bQuoted;
				Deelx::POSIX_FUNC m_quote_fun;

				Deelx::ElxInterface * m_pStockElxs[Deelx::STOCKELX_COUNT];
			};
			/// <summary>
			/// MatchResult 类用来记录匹配结果
			/// MatchResult 对象中记录了所匹配到的字符串在整个文本中的位置, 以及各个捕获组的位置
			/// </summary>
			class RLIB_API MatchResult:public Deelx::MatchResultT<0>
			{
			private:
				const TCHAR *ori_string;
			public:
				MatchResult(const MatchResult & from):Deelx::MatchResultT<0>(from)
				{
					this->ori_string = from.ori_string;
				};
				MatchResult(Deelx::CContext * pContext = 0, int nMaxNumber = -1, const TCHAR *tstring = nullptr):Deelx::MatchResultT<0>(pContext, nMaxNumber)
				{
					this->ori_string = tstring;
				};
			public:
				/// <summary>
				/// 获取指定编号匹配组的内容
				/// </summary>
				String GetGroupValue(int nGroupNumber) const;
			};
			/// <summary>
			/// 负责编译正则表达式和进行匹配替换, 对象内部只记录正则表达式编译的相关信息.
			/// 除了构造方法和 Compile 方法外, 其他的所有匹配和替换方法都没有对 Regexp 对象内部数据做任何更改,
			/// 因此，这些匹配和替换方法都是线程安全的.
			/// @传入的const TCHAR *参数由于没有拷贝而存在严重BUG
			/// </summary>
			class RLIB_API Regexp
			{
			public:
				RLIB_DECLARE_DYNCREATE;
				/// <summary>
				/// 构造函数将调用 Compile 方法进行编译
				/// </summary>
				/// <param name="pattern">正则表达式, 使用 \0 作为结束标志</param>
				/// <param name="flags">表达式匹配模式</param>
				Regexp(const TCHAR * pattern = 0, int flags = 0);
				Regexp(const TCHAR * pattern, int length, int flags);
				/// <summary>
				/// 根据正则表达式语法, 对正则表达式文本进行编译
				/// </summary>
				/// <param name="pattern">正则表达式, 使用 \0 作为结束标志</param>
				/// <param name="flags">表达式匹配模式</param>
				void Compile(const TCHAR * pattern, int flags = 0);
				void Compile(const TCHAR * pattern, int length, int flags);

			public:
				/// <summary>
				/// 验证传入的文本是否刚好匹配正则表达式,
				/// 所谓刚好匹配, 就是要求正则表达式从文本开始位置刚好匹配到文本结束位置
				/// </summary>
				/// <param name="tstring">进行匹配的字符串, 使用 \0 作为结束标志</param>
				/// <param name="pContext">从同一段文本中连续匹配时, 使用同一个正则表达式上下文对象</param>
				/// <returns>返回匹配结果 MatchResult 对象</returns>
				MatchResult MatchExact(const TCHAR * tstring, Deelx::CContext * pContext = 0) const;
				MatchResult MatchExact(const TCHAR * tstring, int length, Deelx::CContext * pContext = 0) const;
				/// <summary>
				/// 从文本中查找匹配符合表达式的子字符串
				/// </summary>
				/// <param name="tstring">进行匹配的字符串, 使用 \0 作为结束标志</param>
				/// <param name="start">开始查找匹配的位置</param>
				/// <param name="pContext">从同一段文本中连续匹配时, 使用同一个正则表达式上下文对象</param>
				/// <returns>返回匹配结果 MatchResult 对象</returns>
				MatchResult Match(const TCHAR * tstring, int start = -1, Deelx::CContext * pContext = 0) const;
				MatchResult Match(const TCHAR * tstring, int length, int start, Deelx::CContext * pContext = 0) const;
				MatchResult Match(Deelx::CContext * pContext) const;
				/// <summary>
				/// 初始化上下文对象, 准备从文本中查找匹配符合表达式的子字符串
				/// </summary>
				/// <param name="tstring">进行匹配的字符串, 使用 \0 作为结束标志</param>
				/// <param name="start">开始查找匹配的位置</param>
				/// <param name="pContext">从同一段文本中连续匹配时, 使用同一个正则表达式上下文对象</param>
				/// <returns>经过初始化的查找匹配上下文对象, 用来在 Match 中使用</returns>
				Deelx::CContext * PrepareMatch(const TCHAR * tstring, int start = -1, Deelx::CContext * pContext = 0) const;
				Deelx::CContext * PrepareMatch(const TCHAR * tstring, int length, int start, Deelx::CContext * pContext = 0) const;
				/// <summary>
				/// 进行文本替换操作, 替换返回的文本，使用完毕后需要进行释放
				/// </summary>
				/// <param name="tstring">被进行替换的初始文本</param>
				/// <param name="replaceto">"替换为"字符串, 将匹配到的子字符串替换成 replaceto 字符串</param>
				/// <param name="start">进行查找替换的开始位置. 默认(-1)表示根据是否是 RIGHTTOLEFT 自动决定开始位置</param>
				/// <param name="ntimes">指定进行替换的次数</param>
				/// <param name="result">未知</param>
				/// <param name="pContext">从同一段文本中连续匹配时, 使用同一个正则表达式上下文对象</param>
				/// <returns>替换后得到的新字符串, 需要调用 ReleaseString 进行释放</returns>
				TCHAR * Replace(const TCHAR * tstring, const TCHAR * replaceto, int start = -1, int ntimes = -1, MatchResult * result = 0,
					Deelx::CContext * pContext = 0) const;
				TCHAR * Replace(const TCHAR * tstring, int string_length, const TCHAR * replaceto, int to_length, int & result_length,
					int start = -1, int ntimes = -1, MatchResult * result = 0, Deelx::CContext * pContext = 0) const;
				/// <summary>
				/// 通过命名分组名, 返回命名分组编号
				/// </summary>
				/// <param name="group_name">命名分组名</param>
				/// <returns>返回命名分组的编号. 如果返回值小于0, 则表示没有该命名分组</returns>
				int GetNamedGroupNumber(const TCHAR * group_name) const;

			public:
				/// <summary>
				/// 释放字符串
				/// </summary>
				/// <param name="tstring">由 Replace 返回的字符串</param>
				static void Collect (TCHAR    * tstring );
				/// <summary>
				/// 释放上下文对象
				/// </summary>
				/// <param name="pContext">由 PrepareMatch 返回的上下文对象</param>
				static void Collect(Deelx::CContext * pContext);

			public:
				Builder m_builder;
			};
		}
	}
}
#endif