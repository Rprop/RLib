/********************************************************************
	Created:	2012/04/22  8:57
	Filename: 	RLib_String.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_STRING
#define _USE_STRING
#include "RLib_Text.h"
#include "RLib_Array.h"

namespace System
{
	/// <summary>
	/// Represents a nonexistent value
	/// </summary>
	enum STRNull
	{
		Nothing
	};
	/// <summary>
	/// Internal C-style string structure
	/// </summary>
	struct STRInfoA
	{
		const char *lpstr;
		intptr_t    length;
	};
	/// <summary>
	/// Internal C-style string structure
	/// </summary>
	struct STRInfoW
	{
		const wchar_t *lpstr;
		intptr_t       length;
	};
#ifdef _UNICODE
	typedef STRInfoW   STRInfo;
#else
	typedef STRInfoA   STRInfo;
#endif // _UNICODE
	/// <summary>
	/// Represents text as a sequence of GBK/UTF-16 code units
	/// </summary>
	class RLIB_API String/* : Nullable*/
	{
		friend class StringArray;

	protected:
		TCHAR           *m_pstr;
		mutable intptr_t m_len;
		intptr_t         m_size;

#ifdef RLIB_BUILDING
	protected:
		struct InternalString *getInternalString() const;
		bool         is_releasable() const;
		bool         is_writable() const;
		void         release_local_data();	
		void         pre_allocate(intptr_t length, bool copyval = true);       
		TCHAR       &get_char(intptr_t index);   
		const TCHAR &get_const_char(intptr_t index) const; 
		/*
		 *  @warning the first parameter must be releaseable
		 */
		void update_reference(LPTSTR pstr, intptr_t size, intptr_t length);
#endif // RLIB_BUILDING

	public:
		static TCHAR nullpstr[1];
		static const TCHAR *nullpcstr;

	public:
		/// <summary>
		/// Initializes a new instance of the String class to Null state
		/// </summary>
		String();
		/// <summary>
		/// Initializes a new instance of the String class to Null state
		/// </summary>
		String(STRNull) : String() {}
		/// <summary>
		/// Initializes a new instance of the String class with the capacity to hold the specified length of characters
		/// </summary>
		explicit String(intptr_t length);
		/// <summary>
		/// Initializes a new instance of the String, and copys the specified length of characters
		/// </summary>
		String(TCHAR *lptstr, intptr_t length = -1);
		/// <summary>
		/// Initializes a new instance of the String, and makes a reference to the specified const string
		/// </summary>
		String(const STRInfoA &si);
		/// <summary>
		/// Initializes a new instance of the String, and makes a reference to the specified const string
		/// </summary>
		String(const char *lpstr);  
		/// <summary>
		/// Initializes a new instance of the String, and copys the specified length of characters
		/// </summary>
		String(const char *lpstr, intptr_t length);
		/// <summary>
		/// Initializes a new instance of the String, and makes a reference to the specified const string
		/// </summary>
		String(const STRInfoW &si);
		/// <summary>
		/// Initializes a new instance of the String, and makes a reference to the specified const string
		/// </summary>
		String(const wchar_t *lpwstr);
		/// <summary>
		/// Initializes a new instance of the String, and copys the specified length of characters
		/// </summary>
		String(const wchar_t *lpwstr, intptr_t length);
		/// <summary>
		/// Initializes a new instance of the String from a existing String
		/// </summary>
		String(const String &str); 
		/// <summary>
		/// Initializes a new instance of the String from a existing rvalue String
		/// </summary>
		String(String &&tmpstr);
		/// <summary>
		/// Releases string resources
		/// </summary>
		~String();
		RLIB_DECLARE_DYNCREATE;

	public:
		/// <summary>
		/// Preallocates storage space for holding the specified length of characters,
		/// copies the original string and returns new instance of result string
		/// </summary>
		String Reserve(intptr_t length);
		/// <summary>
		/// Preallocates storage space for holding the specified length of characters,
		/// and determines whether to keep the original string or not
		/// </summary>
		String &reserve(intptr_t length, bool keep = true);

	public:
		/// <summary>
		/// @see GetData()
		/// </summary>
		operator TCHAR *();    
		/// <summary>
		/// @see GetConstData()
		/// </summary>
		RLIB_INLINE operator const TCHAR *() {
			return this->GetConstData();
		}
		/// <summary>
		/// @see GetConstData()
		/// </summary>
		RLIB_INLINE operator const TCHAR *() const {
			return this->GetConstData();
		}
		/// <summary>
		/// 实现拓展语法( String = Nothing ), 调用析构函数释放内存
		/// 此时 IsNull() 返回 true
		/// </summary>
		String &operator = (STRNull);  
		/// <summary>
		/// 将字符串赋给当前 String 实例, 该方法将立即进行拷贝操作
		/// </summary>
		String &operator = (TCHAR *lptstr);
		/// <summary>
		/// 以指定的字符串结构对当前 String 实例进行赋值
		/// 方法行为取决于具体编译环境, 实例生命期内必须确保对应字符串不会被修改
		/// </summary>
		String &operator = (const STRInfoA &);
		/// <summary>
		/// 以指定的字符串结构对当前 String 实例进行赋值
		/// 方法行为取决于具体编译环境, 实例生命期内必须确保对应字符串不会被修改
		/// </summary>
		String &operator = (const STRInfoW &);
		/// <summary>
		/// 以指定的字符串对当前 String 实例进行赋值
		/// 方法行为取决于具体编译环境, 实例生命期内必须确保对应字符串不会被修改
		/// </summary>
		String &operator = (const char *lpstr);
		/// <summary>
		/// 以指定的字符串对当前 String 实例进行赋值
		/// 方法行为取决于具体编译环境, 实例生命期内必须确保对应字符串不会被修改
		/// </summary>
		String &operator = (const wchar_t *lpwstr);
		/// <summary>
		/// 以指定的 String 实例 对当前 String 实例进行赋值
		/// </summary>
		String &operator = (const String &str); 
		/// <summary>
		/// 以指定的 String 实例 对当前 String 实例进行赋值
		/// </summary>
		String &operator = (String &&tmpstr);

	public:
		/// <summary>
		/// 获取字符串缓冲区大小(包括 \0 结尾符大小, in bytes)
		/// </summary>
		RLIB_PROPERTY_GET(const intptr_t Size, GetSize);
		/// <summary>
		/// 获取字符串长度(in TCHARs, 不包括结尾'\0')
		/// </summary>
		RLIB_PROPERTY_GET(const intptr_t Length, GetLength);
		/// <summary>
		/// 获取字符串可读取大小(不包含结尾'\0'的大小, in bytes)
		/// </summary>
		RLIB_PROPERTY_GET(const intptr_t CanReadSize, GetCanReadSize);
		/// <summary>
		/// 返回字符串缓冲区大小(包括 \0 结尾符大小, in bytes)
		/// </summary>
		intptr_t GetSize() const;  
		/// <summary>
		/// 返回字符串长度(in TCHARs, 不包括结尾'\0')
		/// </summary>
		intptr_t GetLength() const;   
		/// <summary>
		/// 返回字符串可读取大小, in bytes
		/// </summary>
		intptr_t GetCanReadSize() const; 
		/// <summary>
		/// 方法返回内部字符串指针并保证可读写, 对其修改将影响到 String 实例
		/// @warning 该缓冲区具有固定大小并可由 GetSize() 方法获得
		/// </summary>
		LPTSTR GetData();
		/// <summary>
		/// 返回只读字符串指针, 不应该修改该方法返回的字符串内容
		/// 返回值为可安全用于显示(如MsgBox)的只读字符串指针
		/// </summary>
		LPCTSTR GetConstData() const;
		/// <summary>
		/// 获取对象类型
		/// </summary>
		TCHAR *GetType() const { return nullptr; }
		/// <summary>
		/// 创建一个与指定的 String 具有相同值的可读 TCHAR * 副本,
		/// 该副本必须手动调用 Collect() 方法才能正确释放
		/// </summary>
		TCHAR *c_str() const;              
		/// <summary>
		/// 获取当前 String 对象中位于指定字符位置(从0开始)的字符
		/// @wearing 方法不进行参数检查, 请保证不会溢出
		/// </summary>
		TCHAR GetAt(intptr_t) const; 
		/// <summary>
		/// 设置当前 string 对象中位于指定字符位置(从0开始)的字符
		/// </summary>
		void SetAt(intptr_t, TCHAR);

	public:
		/// <summary>
		/// 尝试将指定数目的字符复制到当前 String 实例, 该方法不会分配新的内存, 空间不足返回false
		/// </summary>
		bool tryCopy(LPCTSTR pstr, intptr_t len);
		/// <summary>
		/// 将指定数目的字符复制到当前 String 实例
		/// </summary>
		String &copy(const char *pstr, intptr_t len = -1);
		/// <summary>
		/// 将指定数目的字符复制到当前 String 实例
		/// </summary>
		String &copy(const wchar_t *pstr, intptr_t len = -1);
		/// <summary>
		/// 将指定 String 复制到当前 String 实例
		/// </summary>
		String &copy(const String &str, intptr_t len = -1);
		/// <summary>
		/// 将指定的 String 中的每个格式项替换为相应对象的值的文本等效项
		/// </summary>
		String &__cdecl copyf(_In_ _Printf_format_string_ LPCTSTR pstrFormat, ...);
		/// <summary>
		/// 追加指定字符到当前 String 实例
		/// </summary>
		String &append(const TCHAR c);
		/// <summary>
		/// 追加指定长度字符串到当前 String 实例
		/// </summary>
		String &append(const TCHAR *pstr, intptr_t len = -1);
		/// <summary>
		/// 追加指定长度 String 到当前实例
		/// </summary>
		String &append(const String &str, intptr_t len = -1);
		/// <summary>
		/// 将文本以指定格式追加到当前实例
		/// </summary>
		String &__cdecl appendf(_In_ _Printf_format_string_ LPCTSTR pstrFormat, ...);
		/// <summary>
		/// 将当期实例表示的字符串复制到指定的缓冲区
		/// </summary>
		/// <param name="pstr">字符串缓冲区</param>
		/// <param name="max_length">最大可复制字符数, 包括中止符</param>
		void CopyTo(TCHAR *pstr, intptr_t max_length_with_null);

	public:
		/// <summary>
		/// Access first character, or _T('\0')
		/// </summary>
		TCHAR front() const;
		/// <summary>
		/// Access last character, or _T('\0')
		/// </summary>
		TCHAR back() const;
		/// <summary>
		/// 确定 String 实例的开头是否与指定的字符匹配
		/// </summary>
		bool StartsWith(TCHAR) const;
		/// <summary>
		/// 确定 String 的实例的末尾是否与指定的字符匹配
		/// </summary>
		bool EndsWith(TCHAR) const;
		/// <summary>
		/// 确定 String 实例的开头是否与指定的字符串匹配
		/// </summary>
		bool StartsWith(LPCTSTR pstr, intptr_t length = -1) const;
		/// <summary>
		/// 确定 String 的实例的末尾是否与指定的字符串匹配
		/// </summary>
		bool EndsWith(LPCTSTR pstr, intptr_t length = -1) const;
		//
		// 摘要: 
		//     返回一个值，该值指示指定的 System.String 对象是否出现在此字符串中。
		//
		// 参数: 
		//   value:
		//     要搜寻的字符串。
		//
		// 返回结果: 
		//     如果 value 参数出现在此字符串中，或者 value 为空字符串 ("")，则为 true；否则为 false。
		//     注意, Nothing.Contains() 总是返回false
		//
		// 异常: 
		//   this 或者 value 为 nullptr。
		//
		bool Contains(LPCTSTR value) const;
		/// <summary>
		/// 返回一个值，该值指示指定的字符是否出现在此字符串中。
		/// </summary>
		bool Contains(TCHAR c) const;
		/// <summary>
		/// This method performs an special (case-insensitive and culture-insensitive) comparison.
		/// The search begins at the first character position of this string and continues through 
		/// the last character position
		/// </summary>
		bool ContainsNoCase(LPCTSTR value) const;
		/// <summary>
		/// 与指定的字符串进行比较(考虑其大小写), 并返回一个整数, 指示二者在排序顺序中的相对位置
		/// </summary>
		intptr_t Compare(const TCHAR *) const;
		/// <summary>
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t CompareNoCase(const TCHAR *) const;
		/// <summary>
		/// 使 String 成为空字符串
		/// </summary>
		String &Empty();
		/// <summary>
		/// 判断字符串是否为引用类型(不能直接修改字符串)
		/// </summary>
		bool IsConst() const; 
		/// <summary>
		/// 判断是否为空字符串
		/// </summary>
		bool IsEmpty() const;
		/// <summary>
		/// 判断是否为空字符串指针(nullptr)
		/// </summary>
		bool IsNull() const;
		/// <summary>
		/// 判断是否为空字符串或空字符串指针(nullptr)
		/// </summary>
		bool IsNullOrEmpty() const;
		/// <summary>
		/// 报告指定字符在此字符串中的第一个匹配项的索引,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t IndexOf(TCHAR, intptr_t begin = 0) const;     
		/// <summary>
		/// 报告指定字符在在此实例中的最后一个匹配项的索引位置,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t LastIndexOf(TCHAR) const; 
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t IndexOf(const TCHAR *, intptr_t begin = 0) const;   
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t LastIndexOf(const TCHAR *) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t LastIndexOfL(const TCHAR *, intptr_t len) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t IndexOfNoCase(const TCHAR *, intptr_t begin = 0) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t LastIndexOfNoCase(const TCHAR *) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t LastIndexOfLNoCase(const TCHAR *, intptr_t len) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,
		/// 并返回末位索引(加上指定字符串长度),如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t IndexOfR(const TCHAR *, intptr_t begin = 0) const;   
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,
		/// 并返回末位索引(加上指定字符串长度),如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t IndexOfRL(const TCHAR *, intptr_t len, intptr_t begin = 0) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,
		/// 并返回末位索引,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t LastIndexOfR(const TCHAR *) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,
		/// 并返回末位索引,如果未找到该字符串，则返回 -1
		/// </summary>
		intptr_t LastIndexOfRL(const TCHAR *, intptr_t len) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,
		/// 并返回末位索引(加上指定字符串长度),如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t IndexOfRNoCase(const TCHAR *, intptr_t begin = 0) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在此字符串中的第一个匹配项的索引,
		/// 并返回末位索引(加上指定字符串长度),如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t IndexOfRLNoCase(const TCHAR *, intptr_t len, intptr_t begin = 0) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,
		/// 并返回末位索引,如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t LastIndexOfRNoCase(const TCHAR *) const;
		/// <summary>
		/// 报告 string 或一个或多个字符在在此实例中的最后一个匹配项的索引位置,
		/// 并返回末位索引,如果未找到该字符串，则返回 -1
		/// Compare strings ignoring the case of the strings being compared
		/// </summary>
		intptr_t LastIndexOfRLNoCase(const TCHAR *, intptr_t len) const;
		/// <summary>
		/// 计算指定文本出现的次数
		/// </summary>
		intptr_t CountOf(const TCHAR *, intptr_t begin = 0) const;   
		/// <summary>
		/// 将指定长度 字符串 拼到当前 String 末尾并返回新实例
		/// 方法不会影响当前实例
		/// </summary>
		String Concat(const TCHAR *, intptr_t len = -1) const;  
		/// <summary>
		/// 将指定长度 String 拼到当前 String 末尾并返回新实例
		/// 方法不会影响当前实例
		/// </summary>
		String Concat(const String &, intptr_t len = -1) const; 
		/// <summary>
		/// 返回此 String 颠倒字符次序后的副本
		/// </summary>
		String Reverse() const;
		/// <summary>
		/// 将当前 String 颠倒字符次序
		/// </summary>
		String &reverse();
		/// <summary>
		/// 返回此 String 转换为小写形式的副本
		/// </summary>
		String ToLower() const;
		/// <summary>
		/// 将当前 String 转换为小写形式
		/// </summary>
		String &toLower();
		/// <summary>
		/// 返回此 String 转换为大写形式的副本
		/// </summary>
		String ToUpper() const;
		/// <summary>
		/// 将当前 String 转换为大写形式
		/// </summary>
		String &toUpper();
		/// <summary>
		/// 返回当前 String 对象移除所有前导指定字符和尾部指定字符后的副本
		/// </summary>
		String Trim(TCHAR c = 0) const;
		/// <summary>
		/// 从当前 String 对象移除所有前导指定字符和尾部指定字符
		/// </summary>
		String &trim(TCHAR c = 0);
		/// <summary>
		/// 从当前 String 对象移除数组中指定的一组字符的所有前导指定字符后的副本
		/// </summary>
		String TrimStart(TCHAR c = 0) const;  
		/// <summary>
		/// 从当前 String 对象移除数组中指定的一组字符的所有前导指定字符
		/// </summary>
		String &trimStart(TCHAR c = 0);
		/// <summary>
		/// 从当前 String 对象移除数组中指定的一组字符的所有尾部指定字符后的副本
		/// </summary>
		String TrimEnd(TCHAR c = 0) const;      
		/// <summary>
		/// 从当前 String 对象移除数组中指定的一组字符的所有尾部指定字符
		/// </summary>
		String &trimEnd(TCHAR c = 0);
		/// <summary>
		/// Returns a new string that left-aligns the characters in this string
		/// by padding them on the right with a specified character, for a specified total length
		/// </summary>
		String PadRight(intptr_t totalWidth, TCHAR paddingChar = _T(' ')) const;
		/// <summary>
		/// Left-aligns the characters in this string
		/// by padding them on the right with a specified character, for a specified total length
		/// </summary>
		String &padRight(intptr_t totalWidth, TCHAR paddingChar = _T(' '));
		/// <summary>
		/// Returns a new string that right-aligns the characters in this instance
		/// by padding them on the left with a specified character, for a specified total length
		/// </summary>
		String PadLeft(intptr_t totalWidth, TCHAR paddingChar = _T(' ')) const;
		/// <summary>
		/// Right-aligns the characters in this instance
		/// by padding them on the left with a specified character, for a specified total length
		/// </summary>
		String &padLeft(intptr_t totalWidth, TCHAR paddingChar = _T(' '));
		/// <summary>
		/// 返回一个新字符串, 其中当前实例中出现的 n 个指定字符串都替换为另一个指定的字符串,
		/// 若 n 为 0 则替换全部项
		/// </summary>
		String Replace(const TCHAR *pstrFrom, const TCHAR *pstrTo, intptr_t begin = 0, intptr_t n = 0,
					   intptr_t *replace_count = nullptr) const; 
		/// <summary>
		/// 返回一个新字符串, 其中当前实例中出现的 n 个指定字符串都替换为另一个指定的字符串并忽略大小写,
		/// 若 n 为 0 则替换全部项
		/// </summary>
		String ReplaceNoCase(const TCHAR *pstrFrom, const TCHAR *pstrTo, intptr_t begin = 0, intptr_t n = 0,
							 intptr_t *replace_count = nullptr) const;
		/// <summary>
		/// 将当前实例中出现的 n 个指定字符串都替换为另一个指定的字符串,
		/// 若 n 为 0 则替换全部项
		/// </summary>
		String &replace(const TCHAR *pstrFrom, const TCHAR *pstrTo, intptr_t begin = 0, intptr_t n = 0,
						intptr_t *replace_count = nullptr);
		/// <summary>
		/// 将当前实例中出现的 n 个指定字符串都替换为另一个指定的字符串并忽略大小写,
		/// 若 n 为 0 则替换全部项
		/// </summary>
		String &replaceNoCase(const TCHAR *pstrFrom, const TCHAR *pstrTo, intptr_t begin = 0, intptr_t n = 0,
							  intptr_t *replace_count = nullptr);
		/// <summary>
		/// 从此实例检索并返回新的子字符串, 子字符串从指定的字符位置(从0开始)开始且具有指定的长度
		/// </summary>
		String Substring(intptr_t index, intptr_t length = -1) const;
		/// <summary>
		/// 从此实例检索子字符串, 子字符串从指定的字符位置(从0开始)开始且具有指定的长度
		/// </summary>
		String &substring(intptr_t index, intptr_t length = -1);
		/// <summary>
		/// 返回的字符串数组包含此实例中的子字符串（由指定字符串的元素分隔）
		/// </summary>
		class StringArray *Split(const String &strSeparator, bool removeEmptyEntries = false) const;
		/// <summary>
		/// 返回的字符串数组包含此实例中的子字符串（由指定字符串的元素分隔）
		/// </summary>
		class StringArray *Split(const TCHAR *strSeparator, intptr_t lengthOfSeparator,
								 intptr_t averageItemLength, bool removeEmptyEntries = false) const;
		/// <summary>
		/// 快速拆分此实例中的子字符串（由指定字符串的元素分隔）, 并返回字符串引用数组(不进行任何拷贝)
		/// @warning 方法会影响当前实例字符串值
		/// </summary>
		Collections::Generic::Array<LPTSTR> *FastSplit(const TCHAR *strSeparator, intptr_t lengthOfSeparator,
													   intptr_t averageItemLength);

	public: // efficient string matching, not regular expression
		typedef String(*__match_callback)(_In_ LPCTSTR lpbegin, _In_ LPCTSTR lpend);
		typedef __match_callback MatchCallback;
		/// <summary>
		/// Performs a lazy/non-greedy text match and returns the first matched
		/// </summary>
		String Match(const TCHAR *, const TCHAR *, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match and replaces the first matched
		/// </summary>
		String MatchReplace(const TCHAR *, const TCHAR *, const TCHAR *replaceTo, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match and replaces all matched using a callback
		/// </summary>
		String MatchReplace(const TCHAR *, const TCHAR *, MatchCallback callback, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match and returns all matched as StringArray
		/// </summary>
		class StringArray *MatchAll(const TCHAR *, const TCHAR *, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match(case insensitive) and returns the first matched
		/// </summary>
		String MatchNoCase(const TCHAR *, const TCHAR *, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match(case insensitive) and replaces the first matched
		/// </summary>
		String MatchReplaceNoCase(const TCHAR *, const TCHAR *, const TCHAR *replaceTo, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match(case insensitive) and replaces using a callback
		/// </summary>
		String MatchReplaceNoCase(const TCHAR *, const TCHAR *, MatchCallback callback, intptr_t begin = 0) const;
		/// <summary>
		/// Performs a lazy/non-greedy text match(case insensitive) and returns all matched as StringArray
		/// </summary>
		StringArray *MatchAllNoCase(const TCHAR *, const TCHAR *, intptr_t begin = 0) const;

	public:
		TCHAR &operator [] (intptr_t); 
		const TCHAR &operator [] (intptr_t) const; 
		String &operator += (const TCHAR c);
		String &operator += (const TCHAR *);
		String &operator += (const String &);
		RLIB_INLINE String &operator += (const STRInfo &si) {
			return this->append(si.lpstr, si.length);
		}
		bool operator == (LPCTSTR) const;
		bool operator == (const String &) const;
		bool operator != (LPCTSTR) const;
		bool operator != (const String &) const;
		bool operator <= (LPCTSTR) const;
		bool operator <= (const String &) const;
		bool operator <  (LPCTSTR) const;
		bool operator <  (const String &) const;
		bool operator >= (LPCTSTR) const;
		bool operator >= (const String &) const;
		bool operator >  (LPCTSTR) const;
		bool operator >  (const String &) const;

	public:
		template <typename T> String &appendCompose(const T &t) {
			return this->append(t);
		}
		template <typename T, typename ... Args> String &appendCompose(const T &t, const Args& ... args) {
//			auto count = sizeof...(args) + 1;
			return this->append(t).appendCompose(args...);
		}

	public:
		/// <summary>
		/// Performs optimized string copy
		/// </summary>
		static void FastStringCopy(TCHAR *__restrict destmem, const TCHAR *__restrict srcmem, intptr_t Count);
		/// <summary>
		/// Releases string with reference count
		/// </summary>
		static void Collect(LPVOID);

	public: // iteration
		TCHAR *begin() /* noexcept */ {
			return this->Length > 0 ? this->m_pstr : nullptr;
		}
		const TCHAR *begin() const  /* noexcept */ {
			return this->Length > 0 ? this->m_pstr : nullptr;
		}
		TCHAR *end() /* noexcept */ {
			return this->Length > 0 ? this->m_pstr + this->Length : nullptr;
		}
		const TCHAR *end() const /* noexcept */ {
			return this->Length > 0 ? this->m_pstr + this->Length : nullptr;
		}
	};
};

//-------------------------------------------------------------------------

typedef System::String string;
/*
 *	Composes strings
 */
template <typename T> static string StringCompose(const T &t) {
	return t;
}
/*
 *	Composes strings
 */
template <typename T, typename ... Args> static string StringCompose(const T &t, const Args& ... args) {
	return t + StringCompose(args...);
}

//-------------------------------------------------------------------------

extern RLIB_API System::String operator + (const System::String &, const TCHAR *);
extern RLIB_API System::String operator + (const System::String &, const System::STRInfo &);
extern RLIB_API System::String operator + (const System::String &, const System::String &);

//-------------------------------------------------------------------------

/*
 *	Takes advantage of compiler's format specifiers checking
 */
#if defined(_DEBUG) && !defined(RLIB_BUILDING) && !defined(RLIB_NOCRT)
# define copyf(f,...)   copyf((_sntprintf_s(NULL, 0, 0, f, __VA_ARGS__), f), __VA_ARGS__)
# define appendf(f,...) appendf((_sntprintf_s(NULL, 0, 0, f, __VA_ARGS__), f), __VA_ARGS__)
# pragma warning(disable:4774) // format string expected in argument 4 is not a string literal 
#endif // _DEBUG && RLIB_BUILDING

#endif