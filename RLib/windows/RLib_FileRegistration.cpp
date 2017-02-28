/********************************************************************
Created:	2014/07/31  18:28
Filename: 	RLib_FileRegistration.h
Author:		Copyright (c) Microsoft Corporation.
Url:	    http://code.microsoft.com/RLib
*********************************************************************/
#include "RLib_FileRegistration.h"

#ifdef _USE_FILE_REG
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "Shlwapi.lib")

using namespace System::Microsoft;

//-------------------------------------------------------------------------

FileRegistration::FileRegistration(
	const String& progId,
	const String& path,
	const String& friendlyName,
	const String& appUserModelID,
	int numExtensions,
	LPCTSTR* extensions)
{
	m_progId = progId;
	m_appPath = path;
	m_friendlyName = friendlyName;
	m_userModelID = appUserModelID;

	if (numExtensions > 0)
	{
		m_pExtensionsToRegister = new Collections::Generic::List<String>();
		for (int i = 0; i < numExtensions; ++i)
		{
			m_pExtensionsToRegister->Add(extensions[i]);
		}
	}
	else
	{
		m_pExtensionsToRegister = nullptr;
	}
}

//-------------------------------------------------------------------------

FileRegistration::~FileRegistration(void)
{
}

//-------------------------------------------------------------------------

// All ProgIDs that can handle a given file type should be listed under OpenWithProgids, even if listed
// as the default, so they can be enumerated in the Open With dialog, and so the Jump Lists can find
// the correct ProgID to use when relaunching a document with the specific application the Jump List is
// associated with.
HRESULT FileRegistration::RegisterFileAssociation()
{
	HRESULT hr = RegisterProgID();
	if (SUCCEEDED(hr))
	{
		if (this->m_pExtensionsToRegister != nullptr)
		{
			for (int i = 0; SUCCEEDED(hr) && i < m_pExtensionsToRegister->Length; ++i)
			{
				hr = RegisterVerbsForFileExtension(m_pExtensionsToRegister->Get(i), true);
			}
		} //if
		if (SUCCEEDED(hr))
		{
			// Notify that file associations have changed
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
		}
	}
	return hr;
}

//-------------------------------------------------------------------------

HRESULT FileRegistration::UnregisterFileAssociation()
{
	HRESULT hr = UnregisterProgID();
	if (SUCCEEDED(hr))
	{
		if (this->m_pExtensionsToRegister != nullptr)
		{
			for (int i = 0; SUCCEEDED(hr) && i < m_pExtensionsToRegister->Length; ++i)
			{
				// Unregister the file type
				hr = RegisterVerbsForFileExtension(m_pExtensionsToRegister->Get(i), false);
			}
		} //if
		if (SUCCEEDED(hr))
		{
			// Notify that file associations have changed
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
		}
	}
	return hr;
}

//-------------------------------------------------------------------------

bool FileRegistration::AreFileTypesRegistered()
{
	bool result = false;
	HKEY progIdKey;
	if (SUCCEEDED(HRESULT_FROM_WIN32(RegOpenKey(HKEY_CLASSES_ROOT, m_progId, &progIdKey))))
	{
		result = true;
		RegCloseKey(progIdKey);
	}
	return result;
}

//-------------------------------------------------------------------------

HRESULT FileRegistration::RegisterProgID()
{
	HKEY progIdKey;
	// Create a registry key for the progId under HKEY_CLASSES_ROOT key
	HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, m_progId, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &progIdKey, nullptr));
	// Set registry key values
	if (SUCCEEDED(hr))
	{
		hr = SetRegistryValue(progIdKey, nullptr, _T("FriendlyTypeName"), m_friendlyName);

		if (SUCCEEDED(hr))
		{
			hr = SetRegistryValue(progIdKey, nullptr, _T("AppUserModelID"), m_userModelID);
		}
		if (SUCCEEDED(hr))
		{
			hr = SetRegistryValue(progIdKey, _T("DefaultIcon"), nullptr, m_appPath);
		}
		if (SUCCEEDED(hr))
		{
			hr = SetRegistryValue(progIdKey, _T("CurVer"), nullptr, m_progId);
		}
		// Create a sub registry key under the progId key
		HKEY shellKey;
		if (SUCCEEDED(hr))
		{
			hr = HRESULT_FROM_WIN32(RegCreateKeyEx(progIdKey, _T("shell"), 0, nullptr,
				REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &shellKey, nullptr));
			// Create the command line
			String commandLine = m_appPath + _T(" %1");
			// Set value for ProgID\shell\Open\Command
			hr = SetRegistryValue(shellKey, _T("Open\\Command"), nullptr, commandLine);
			// Set "Open" as the default verb for this ProgID.
			if (SUCCEEDED(hr))
			{
				hr = SetRegistryValue(shellKey, nullptr, nullptr, _T("Open"));
			}
			RegCloseKey(shellKey);
		}
		RegCloseKey(progIdKey);
	}
	return hr;
}

//-------------------------------------------------------------------------

HRESULT FileRegistration::UnregisterProgID()
{
	// Delete the progId registry key
	long result = RegDeleteTree(HKEY_CLASSES_ROOT, m_progId);
	return (ERROR_SUCCESS == result || ERROR_FILE_NOT_FOUND == result) ? S_OK : HRESULT_FROM_WIN32(result);
}

//-------------------------------------------------------------------------

HRESULT FileRegistration::RegisterVerbsForFileExtension(const String& fileExtension, bool toRegister)
{
	// Construct the registry key name that we want to open or create
	String registryKeyName = fileExtension + _T("\\OpenWithProgids");
	HKEY openWithProgidsKey;
	// e.g. HKEY_CLASSES_ROOT\.jpeg\OpenWithProgids
	HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, registryKeyName,
		0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &openWithProgidsKey, nullptr));
	if (SUCCEEDED(hr))
	{
		if (toRegister)
		{
			hr = HRESULT_FROM_WIN32(RegSetValueEx(openWithProgidsKey, m_progId, 0, REG_NONE, nullptr, 0));
		}
		else
		{
			hr = HRESULT_FROM_WIN32(RegDeleteKeyValue(openWithProgidsKey, nullptr, m_progId));
		}
		RegCloseKey(openWithProgidsKey);
	}

	return hr;
}

//-------------------------------------------------------------------------

HRESULT FileRegistration::SetRegistryValue(HKEY registryKey, LPCTSTR keyName, LPCTSTR keyValue, LPCTSTR keyData)
{
	return HRESULT_FROM_WIN32(
		SHSetValue(
		registryKey,
		keyName,
		keyValue,
		REG_SZ,
		keyData,
		static_cast<unsigned long>((_tcslen(keyData) + 1) * sizeof(TCHAR))));
}

#endif // _USE_FILE_REG