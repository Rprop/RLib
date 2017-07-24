/********************************************************************
	Created:	2016/07/27  19:05
	Filename: 	Zipres.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdio.h>
#include <support/ZLibWrapLib/ZLibWrapLib.h>
#ifdef _DEBUG
# pragma comment(lib, "SupportLib_x64d.lib")
# pragma comment(lib, "ZLibWrapLib_x64d.lib")
#else
# pragma comment(lib, "SupportLib_x64.lib")
# pragma comment(lib, "ZLibWrapLib_x64.lib")
#endif // _DEBUG


//-------------------------------------------------------------------------

static String g_project_dir, g_project_name, g_rlib_props_dir;
static struct ZIP_PARAM
{
	String         path;
	BufferedStream cache;
} g_zip_param;

//-------------------------------------------------------------------------

extern unsigned char zip_data[1708];
extern const String &temporary_path();

//-------------------------------------------------------------------------

static void  RLIB_VECTORCALL doFileEnumeration(String szPath, const String &pattern, bool bRecursion, void(_cdecl *enumCallback)(String &))
{
	szPath = Path::ToDosPath(Path::AddBackslash(szPath));

	WIN32_FIND_DATA fd;
	HANDLE hFindFile = FindFirstFile(szPath + pattern, &fd);
	if (hFindFile == INVALID_HANDLE_VALUE) {
		return;
	} //if

	bool bIsDirectory;
	bool bFinish = false;
	while (!bFinish) 
	{
		String &&tempPath = szPath + fd.cFileName;

		bIsDirectory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		// ignores . ..
		if (bIsDirectory && (_tcscmp(fd.cFileName, _T(".")) == 0 ||
							 _tcscmp(fd.cFileName, _T("..")) == 0)) {
			bFinish = FindNextFile(hFindFile, &fd) == FALSE;
			continue;
		} //if

		if (!bIsDirectory) {
			enumCallback(tempPath);
		} //if

		// sub dir
		if (bIsDirectory && bRecursion) {
			doFileEnumeration(tempPath, pattern, bRecursion, enumCallback);
		} //if

		bFinish = FindNextFile(hFindFile, &fd) == FALSE;
	}

	FindClose(hFindFile);
}

//-------------------------------------------------------------------------

void gen_project(const String &dir, const String &project, const String &rlib)
{
	String path = dir + project + _R("\\");
	Directory::CreateIf(path/*, FILE_ALL_ACCESS*/);
	
	g_project_dir    = path;
	g_project_name   = project;
	g_rlib_props_dir = rlib;

	ZIP_IO io;
	io.IO_Create = [](const String &filepath)->ZIP_HANDLE {			
		g_zip_param.path         = filepath;
		g_zip_param.cache.Length = 0;
		return static_cast<ZIP_HANDLE>(&g_zip_param);
	};
	io.IO_NativeHandle = [](ZIP_HANDLE/* zh*/)->HANDLE {
		// we don't need file time
		return INVALID_HANDLE_VALUE;
	};
	io.IO_Write = [](ZIP_HANDLE zh, const void *buffer, intptr_t count) {
		static_cast<ZIP_PARAM *>(zh)->cache.Write(buffer, count);
	};
	io.IO_Flush = [](ZIP_HANDLE zh) {
		assert(static_cast<ZIP_PARAM *>(zh) == &g_zip_param);

		g_zip_param.path.replaceNoCase(_T("MyApp."), g_project_name + _R("."));

		if (g_zip_param.path.EndsWith(_T(".vcxproj"))) {
			g_zip_param.cache.Position = 0;
			ManagedObject<BufferedStream> stream = Encoder::ToCurrentEncoding(g_zip_param.cache.CurrentPtr, g_zip_param.cache.MaxReadSize,
																			  UnknownEncoding);
			if (stream) {
				String contents(stream->Length / sizeof(TCHAR));
				stream->Read(contents, stream->Length);
				contents[stream->Length / sizeof(TCHAR)] = _T('\0');
				// GUID
				contents = contents.MatchReplaceNoCase(_T("<ProjectGuid>"), _T("</ProjectGuid>"), [](LPCTSTR/* lpbegin*/, LPCTSTR/* lpend*/)->String {
					Random rnd;
					return String(66).Format(66, _T("<ProjectGuid>{00002016-%d-%d-%d-000000000000}</ProjectGuid>"),
											 rnd.NextLong(1000, 9999), rnd.NextLong(1000, 9999), rnd.NextLong(1000, 9999));
				});
				// Project Name
				contents = contents.MatchReplaceNoCase(_T("<ProjectName>"), _T("</ProjectName>"), [](LPCTSTR/* lpbegin*/, LPCTSTR/* lpend*/)->String {
					return _R("<ProjectName>") + g_project_name + _R("</ProjectName>");
				});
				contents = contents.MatchReplaceNoCase(_T("<RootNamespace>"), _T("</RootNamespace>"), [](LPCTSTR/* lpbegin*/, LPCTSTR/* lpend*/)->String {
					return _R("<RootNamespace>") + g_project_name + _R("</RootNamespace>");
				});
				// Files
				contents = contents.MatchReplaceNoCase(_T("<ClCompile Include=\""), _T("\""), [](LPCTSTR lpbegin, LPCTSTR lpend)->String {
					String fn(lpend - lpbegin);
					fn.Copy(lpbegin, lpend - lpbegin);
					if (fn == _T("MyApp.cpp")) {
						fn = g_project_name + _R(".cpp");
					} //if
					return _R("<ClCompile Include=\"") + fn + _R("\"");
				});
				// Imports
				contents = contents.MatchReplaceNoCase(_T("<Import Project=\""), _T("\""), [](LPCTSTR lpbegin, LPCTSTR lpend)->String {
					String prj(lpend - lpbegin);
					prj.Copy(lpbegin, lpend - lpbegin);
					if (prj.ContainsNoCase(_T("RLib")) && prj.ContainsNoCase(_T(".props"))) {
						prj = Path::RelativePathTo(g_project_dir, g_rlib_props_dir) + Path(prj).FileName;
					} //if
					return _R("<Import Project=\"") + prj + _R("\"");
				});

				g_zip_param.cache.Position = 0;
				Encoder::WriteTextStream(g_zip_param.cache, contents, contents.CanReadSize, true, UTF8Encoding);
			} //if
		} //if

		ManagedObject<FileStream> file = File::Create(g_zip_param.path, FileMode::CreateAlways, FileAccess::Write);
		if (file) {
			g_zip_param.cache.Position = 0;
			file->Write(g_zip_param.cache.CurrentPtr, g_zip_param.cache.MaxReadSize);
		} //if
	};

	ManagedObject<FileStream> zipfile = File::Create(temporary_path() + _R("rlib_project_template.osx"),
													 FileMode::TruncateCreate, FileAccess::Write);
	if (zipfile) {
		g_zip_param.cache.Position = 0;
		zipfile->Write(zip_data, sizeof(zip_data));
		zipfile.Finalize();

		BOOL r = ZipExtract(temporary_path() + _R("rlib_project_template.osx"), path, io);
		_tprintf(_T("\t%s") RLIB_NEWLINE, r ? _T("succeed") : _T("failed"));
	} else {
		_tprintf(_T("\t%s %d") RLIB_NEWLINE, _T("failed, release template error"), Exception::GetLastErrorId());
	} //if
}