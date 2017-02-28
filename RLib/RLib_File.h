/********************************************************************
	Created:	2011/02/16  11:04
	Filename: 	RLib_File.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_FILE
#define _USE_FILE
#include "RLib_FileStream.h"
#include "RLib_Path.h"

//////////////////////////////////////////////////////////////////////////
namespace System
{
	namespace IO
	{
		/// <summary>
		/// 定义用于控制对文件的读访问、写访问或读/写访问的常数
		/// </summary>
		enum class FileAccess : ACCESS_MASK
		{
			All = GENERIC_ALL,
			Read = GENERIC_READ,
			Write = GENERIC_WRITE,
			ReadWrite = GENERIC_READ | GENERIC_WRITE,
			// Standard all rights
			NT_ALL = FILE_ALL_ACCESS,
			// Read data from the file
			NT_READ = FILE_READ_DATA,
			// Write data to the file
			NT_WRITE = FILE_WRITE_DATA,
			// Delete file
			NT_DELETE = DELETE,
			// Append data to the file
			NT_APPEND = FILE_APPEND_DATA,
			// Read the attributes of the file
			NT_READ_ATTRIBUTES = FILE_READ_ATTRIBUTES,
			// Read the extended attributes (EAs) of the file. This flag is irrelevant for device and intermediate drivers
			NT_READ_EA = FILE_READ_EA,
			// Write the attributes of the file
			NT_WRITE_ATTRIBUTES = FILE_WRITE_ATTRIBUTES,
			// Change the extended attributes (EAs) of the file. This flag is irrelevant for device and intermediate drivers
			NT_WRITE_EA = FILE_WRITE_EA,
			// Use system paging I/O to read data from the file into memory. This flag is irrelevant for device and intermediate drivers
			NT_EXECUTE = FILE_EXECUTE,
			// List the files in the directory
			NT_LIST_DIRECTORY = FILE_LIST_DIRECTORY,
			// Traverse the directory, in other words, include the directory in the path of a file
			NT_TRAVERSE = FILE_TRAVERSE
		};
		/// <summary>
		/// 指定操作系统打开文件的方式
		/// </summary>
		enum class FileMode
		{
			// 指定操作系统应创建文件, 如果文件存在则失败
			CreateOnly = FILE_CREATE,
			// @see CreateAlways
			CreateNew = FILE_SUPERSEDE,
			// 指定操作系统应创建新文件, 如果文件已存在则替换(删除旧文件以覆盖)
			CreateAlways = FILE_SUPERSEDE,
			// 指定操作系统应打开现有文件, 如果文件不存在则失败
			OpenExist = FILE_OPEN,
			// 指定操作系统应打开文件(如果文件存在),否则应创建新文件
			OpenOrCreate = FILE_OPEN_IF,
			// 指定操作系统应覆盖现有文件(保留文件属性), 文件不存在则失败
			Truncate = FILE_OVERWRITE,
			// 指定操作系统应覆盖现有文件(保留文件属性), 或者创建新文件
			TruncateCreate = FILE_OVERWRITE_IF,
			// 若存在文件，则打开该文件并查找到文件尾，或者创建一个新文件
			AppendAlways = 0x7FFFFFFF /* RLib extended */
		};
		/// <summary>
		/// 提供文件和目录的特性
		/// </summary>
		enum class FileAttributes
		{
			// 文件正常,没有设置其他的特性,此特性仅在单独使用时有效
			Normal = FILE_ATTRIBUTE_NORMAL,
			// 该文件或目录是加密的
			Encrypted = FILE_ATTRIBUTE_ENCRYPTED,
			// 文件是隐藏的
			Hidden = FILE_ATTRIBUTE_HIDDEN,
			// 文件的存档状态,应用程序使用此特性为文件加上备份或移除标记
			Archive = FILE_ATTRIBUTE_ARCHIVE,
			// 文件已脱机,文件数据不能立即供使用
			Offline = FILE_ATTRIBUTE_OFFLINE,
			// 文件为只读
			ReadOnly = FILE_ATTRIBUTE_READONLY,
			// 文件为系统文件
			SystemFile = FILE_ATTRIBUTE_SYSTEM,
			// 文件是临时文件,文件系统尝试将所有数据保留在内存中以便更快地访问,
			// 而不是将数据刷新回大容量存储器中.不再需要临时文件时,应用程序会立即将其删除
			Temporary = FILE_ATTRIBUTE_TEMPORARY
		};
		/// <summary>
		/// 包含用于控制其他 FileStream 对象对同一文件可以具有的访问类型的常数
		/// </summary>
		enum class FileShare
		{
			// 谢绝共享当前文件
			None = NULL,
			// 允许随后打开文件读取
			Read = FILE_SHARE_READ,
			// 允许随后打开文件写入	
			Write = FILE_SHARE_WRITE,
			// 允许随后删除文件
			Delete = FILE_SHARE_DELETE,
			// 允许随后打开文件读取和写入	
			ReadWrite = FILE_SHARE_READ | FILE_SHARE_WRITE,
			// 允许随后打开文件读取、写入和删除	
			All = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
		};
		/// <summary>
		/// 表示用于创建 FileStream 对象的附加选项
		/// </summary>
		enum class FileOptions
		{
			// 指示无其他参数
			None = NULL,
			// 指示系统应通过任何中间缓存、直接写入磁盘
			WriteThrough = FILE_WRITE_THROUGH,
			// 指示随机访问文件, 系统可将此选项用作优化文件缓存的提示
			RandomAccess = FILE_RANDOM_ACCESS,
			// 指示按从头到尾的顺序访问文件,系统可将此选项用作优化文件缓存的提示
			// 如果应用程序移动用于随机访问的文件指针,可能不发生优化缓存,但仍然保证操作的正确性
			// 指定此标志可以提高使用顺序访问读取大文件的应用程序的性能
			// 对于大多数情况下都按顺序读取大文件、但偶尔跳过小的字节范围的应用程序而言，性能提升可能更明显
			SequentialScan = FILE_SEQUENTIAL_ONLY,
			// 指示当不再使用某个文件时,自动删除该文件
			DeleteOnClose = FILE_DELETE_ON_CLOSE
		};
		/// <summary>
		/// 提供用于创建、复制、删除、移动和打开文件的静态方法,并协助创建 FileStream 对象
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE File
		{
		public:
			/// <summary>
			/// 以指定的模式和访问权限打开指定路径上的文件并返回 FileStream
			/// </summary>
			static FileStream *Open(const String &path, 
									FileMode mode = FileMode::OpenExist,
									FileAccess access = FileAccess::ReadWrite,
									FileShare share = FileShare::None, 
									FileOptions options = FileOptions::None);
			/// <summary>
			/// 创建指定文件, 打开并返回 FileStream
			/// </summary>
			static FileStream *Create(const String &path, 
									  FileMode mode = FileMode::CreateOnly,
									  FileAccess access = FileAccess::ReadWrite,
									  FileAttributes attr = FileAttributes::Normal,
									  FileShare share = FileShare::None,
									  FileOptions options = FileOptions::None);
		public:			
			/// <summary>
			/// 删除指定文件
			/// </summary>
			static bool Delete(const String &path);
			/// <summary>
			/// 尝试删除指定文件
			/// </summary>
			static bool TryDelete(const String &path);
			/// <summary>
			/// 判断指定文件或文件夹是否存在
			/// </summary>
			static bool Exist(const String &path);
			/// <summary>
			/// 读取指定文件全部内容并返回 String
			/// </summary>
			static String ReadAllText(const String &path);
			/// <summary>
			/// 读取指定文件内容, 如果文件可读长度小于 max_length, 则保证 NULL 结尾
			/// </summary>
			/// <param name="path">可读的文件路径</param>
			/// <param name="buffer">输出缓冲区</param>
			/// <param name="max_length">最大可读取长度(in TCHARs, 不包括'\0')</param>
			/// <returns>实际读取长度(in TCHARs, 不包括'\0')</returns>
			static intptr_t ReadText(const String &path, OUT LPTSTR buffer,
									 intptr_t max_length);
			/// <summary>
			/// 根据指定编码写入指定文件内容, 如果文件已存在则覆盖
			/// </summary>
			static bool WriteAllText(const String &path, const String &text,
									 Text::Encoding codepage = Text::UnknownEncoding);
			/// <summary>
			/// 根据指定编码写入指定文件内容, 如果文件已存在则覆盖
			/// </summary>
			static bool WriteText(const String &path, LPCTSTR lptext, intptr_t length,
								  Text::Encoding codepage = Text::UnknownEncoding);
			/// <summary>
			/// 根据指定编码续写指定文件内容, 如果文件不存在则新建
			/// </summary>
			static bool AppendAllText(const String &path, const String &text,
									  Text::Encoding codepage = Text::UnknownEncoding);
			/// <summary>
			/// 根据指定编码续写指定文件内容, 如果文件不存在则新建
			/// </summary>
			static bool AppendText(const String &path, LPCTSTR lptext, intptr_t length,
								   Text::Encoding codepage = Text::UnknownEncoding);
			/// <summary>
			/// 复制指定文件, 并指定如果目标路径存在是否覆盖
			/// </summary>
			static bool Copy(const String &path, const String &new_path,
							 bool replaceIfExists = true);
			/// <summary>
			/// Supplies network open information for the specified file
			/// </summary>
			static bool GetFullAttributes(IN const String &path, 
										  OUT FileFullAttributes *lpFileInformation);
		};
	};
};
#endif