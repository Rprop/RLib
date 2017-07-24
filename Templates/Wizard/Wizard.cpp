/********************************************************************
	Created:	2016/07/26  22:33
	Filename: 	Wizard.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdio.h>
#include <locale.h>
#pragma warning(push)
#pragma warning(disable:4917)
#include <ShlObj.h>
#pragma warning(pop)

//-------------------------------------------------------------------------

ManagedObject<Ini::IniFile> config;

//-------------------------------------------------------------------------

extern void gen_project(const String &dir, const String &project, const String &rlib);

//-------------------------------------------------------------------------

const String &temporary_path()
{
	static String path(RLIB_MAX_PATH);
	if (path.IsEmpty()) {
		GetTempPath(RLIB_MAX_PATH, path);
		path.trim(_T('/')).trim(_T('\\')).Append(_T('\\'));
	} //if
	return path;
}

//-------------------------------------------------------------------------

static const String &config_path()
{
	static String cfg_path;
	if (cfg_path.IsNull()) {
		cfg_path = temporary_path() + TSTR(_T(RLIB_TARGETNAME) _T(".cfg"));
	} //if
	return cfg_path;
}

//-------------------------------------------------------------------------

static String RLIB_VECTORCALL select_path(LPCTSTR lpszTitle)
{
	String szBuffer(RLIB_MAX_PATH);
	BROWSEINFO bi     = { 0 };
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle      = lpszTitle;
	bi.ulFlags        = BIF_RETURNFSANCESTORS;
	bi.lpfn           = [](HWND hwnd, UINT uMsg, LPARAM/* lParam*/, LPARAM/* lpData*/)->int
	{   
		if (uMsg == BFFM_INITIALIZED) {
			String last_path = config->Read(_R("Paths"), _R("Last"), AppBase::GetStartupPath());
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(last_path.GetConstData()));
		} //if
		return TRUE;
	};
	LPITEMIDLIST lpidlist = SHBrowseForFolder(&bi);
	if (lpidlist != nullptr) {
		SHGetPathFromIDList(lpidlist, szBuffer);
		SHFree(lpidlist);

		szBuffer.trim(_T('/')).trim(_T('\\')).Append(_T('\\'));
		(*config)[_R("Paths")][_R("Last")] = szBuffer;
		return szBuffer;
	} //if
	
	return Nothing;
}

//-------------------------------------------------------------------------

static String &RLIB_VECTORCALL probe_rlib_path(String explicit_root = Nothing)
{
	static String rlib_path;
	if (!rlib_path.IsNull()) return rlib_path;

	String root = explicit_root.IsNull() ? AppBase::GetStartupPath() : explicit_root.trim(_T('/')).trim(_T('\\')) + _R("\\");
	String probabilities[] = { 
		root + _R("RLib\\props\\"), root + _R("\\props\\"), root + _R("..\\RLib\\props\\"),
		root + _R("RLib\\RLib\\props\\"), root + _R("..\\RLib\\RLib\\props\\"), root + _R("..\\..\\props\\"),
		config->Read(_R("Paths"), _R("RLib"), _R("D:\\RLib\\RLib\\props\\"))
	};
	for (auto &prob : probabilities)
	{
		if (File::Exist(prob + _R("RLib.props"))) {
			rlib_path = Path(prob).GetDosPath();
			(*config)[_R("Paths")][_R("RLib")] = rlib_path;
			break;
		} //if
	} //for

	return rlib_path;
}

//-------------------------------------------------------------------------

static void core_prepare()
{
	_tprintf(_T("Probing RLib project location ...") RLIB_NEWLINE);
	if (probe_rlib_path().IsNull()) {
		_tprintf(_T("\tnot found") RLIB_NEWLINE);
		String library_dir;
		do 
		{
			library_dir = select_path(_T("Selects RLib project location:"));
			if (library_dir.IsNull()) return;		
		} while (probe_rlib_path(library_dir).IsNull());
	} //if
	_tprintf(_T("\tfound at: %s") RLIB_NEWLINE, probe_rlib_path().GetConstData());

	_tprintf(_T("Selects new project location:") RLIB_NEWLINE);
	String project_dir = select_path(_T("Selects project location(new folder will be created here):"));
	if (project_dir.IsNull()) return;
	_tprintf(_T("\t%s") RLIB_NEWLINE, project_dir.GetConstData());

	TCHAR project_name[RLIB_DEFAULT_LENGTH];
__retype_name:
	_tprintf(_T("Types project name you want to create(at least 2 words):") RLIB_NEWLINE _T("\t"));
	_tscanf_s(_T("%c"), project_name, RLIB_DEFAULT_LENGTH);
	if (project_name[0] == _T('\n')) return;
	_tscanf_s(_T("%s"), project_name + 1, RLIB_DEFAULT_LENGTH - 1);

	if (Directory::Exist(project_dir + project_name)) {
		_tprintf(_T("\tproject %s already exists, overwrite?[Y/N]") RLIB_NEWLINE _T("\t"), project_name);
		getchar();
		TCHAR control;
		_tscanf_s(_T("%c"), &control, 1);
		getchar();
		if (control != _T('Y') && control != _T('y')) goto __retype_name;
	} //if

	_tprintf(_T("Creating project %s at %s ...") RLIB_NEWLINE, project_name, project_dir.GetConstData());
	gen_project(project_dir, project_name, probe_rlib_path());

	// fix rlib path if in need
	String &&rlib_root = Path::ToDosPath(probe_rlib_path() + _R("../../"));
	_tprintf(_T("Fixing rlib path to %s ...") RLIB_NEWLINE, rlib_root.GetConstData());
	if (File::Exist(probe_rlib_path() + _R("RLib Path.props"))) {
		String &&contents = File::ReadAllText(probe_rlib_path() + _R("RLib Path.props"));
		String &&root = contents.Match(_T("<RLibDir>"), _T("</RLibDir>"));
		if (!root.IsEmpty() && rlib_root != root) {
			contents = contents.Substring(0, contents.IndexOfR(_T("<RLibDir>"))) + rlib_root + contents.Substring(contents.IndexOf(_T("</RLibDir>")));
			File::WriteAllText(probe_rlib_path() + _R("RLib Path.props"), contents, UTF8Encoding);
		} //if	
	} //if
	_tprintf(_T("\t%s") RLIB_NEWLINE, _T("succeed"));
}

//-------------------------------------------------------------------------

int __stdcall main()
{
	_tsetlocale(LC_CTYPE, _T("chs"));
	_tprintf(_T("%s"),
			 _T("VS2015 (R) C/C++ Project Wizard Version 1.00.00") RLIB_NUMBER_TO_STRING(VSVER)
			 _T(" for RLib v") RLIB_NUMBER_TO_STRING(RLIB_VER) _T("_") _T(RLIB_TARGET) RLIB_NEWLINE
			 _T("Copyright (C) RLib Open Source Project.  All rights reserved.") RLIB_NEWLINE RLIB_NEWLINE);
	
	config = new Ini::IniFile(config_path());
	core_prepare();
	config->SaveFile();

	_tprintf(_T("%s"),
			 RLIB_NEWLINE _T("For more information on RLib, see http://rlib.cf/") RLIB_NEWLINE RLIB_NEWLINE);
	return STATUS_SUCCESS;
}