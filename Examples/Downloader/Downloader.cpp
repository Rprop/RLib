/********************************************************************
	Created:	2016/07/20  14:45
	Filename: 	Downloader.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdlib.h>

//-------------------------------------------------------------------------

int __stdcall main()
{
	/*
	 * a simple example shows how to download internet resources 
	 * while keeping the directory structure.
	 */
	String root    = _R("GitHub/"); // root dir to save files
	String files[] = {
		_R("https://github-windows.s3.amazonaws.com/Application%20Files/GitHub_3_1_1_4/zh-Hans/Microsoft.Expression.Interactions.resources.dll.deploy"),  
		_R("https://github-windows.s3.amazonaws.com/Application%20Files/GitHub_3_1_1_4/zh-Hant/Microsoft.Expression.Interactions.resources.dll.deploy") 
	}; // files to be download
	Directory::Exist(root) || Directory::Create(root);

	for (auto &fn : files)
	{
		String path = Uri(fn).PathAndQuery;
		ManagedObject<StringArray> dirs = path.Substring(1, path.LastIndexOf(_T("/")) - 1).Split(RLIB_STR_LEN(_T("/")), 16);
		String dir = root;
		foreachp(lpdir, dirs)
		{
			dir += HttpUtility::UrlDecode(*lpdir);
			Directory::Exist(dir) || Directory::Create(dir);
		}

		String filename = path.Substring(path.LastIndexOfR(_T("/"))); // assumes that there is not query part in URL 
		bool result = WebClient::DownloadFile(fn, dir + filename);

		printf("%s %s\n", RT2A(filename).toGBK(), result ? "succeed" : "failed");
	}
}