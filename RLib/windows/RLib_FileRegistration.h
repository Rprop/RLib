/********************************************************************
	Created:	2014/07/31  18:28
	Filename: 	RLib_FileRegistration.h
	Author:		Copyright (c) Microsoft Corporation.
	Url:	    http://code.microsoft.com/RLib
*********************************************************************/
#include <RLib_String.h>
#include <RLib_List.h>

#if !(defined _USE_FILE_REG) && !(defined _DISABLE_FILE_REG)
# define _USE_FILE_REG

//-------------------------------------------------------------------------

namespace System
{
	namespace Microsoft
	{
		class RLIB_API FileRegistration
		{
		public:
			FileRegistration(const String& progId, const String& path, const String& friendlyName,
				const String& appUserModelID, int numExtensions, LPCTSTR* extensions);
			~FileRegistration(void);
			RLIB_DECLARE_DYNCREATE;

		public:
			HRESULT RegisterFileAssociation();
			bool AreFileTypesRegistered();
			HRESULT UnregisterFileAssociation();

		private:
			String m_progId;
			String m_appPath;
			String m_friendlyName;
			String m_userModelID;
			Collections::Generic::List<String> *m_pExtensionsToRegister;

			// Methods
			HRESULT RegisterProgID();
			HRESULT UnregisterProgID();
			HRESULT RegisterVerbsForFileExtension(const String& fileExtension, bool toRegister);
			HRESULT SetRegistryValue(__in HKEY registryKey, __in LPCTSTR keyName, __in LPCTSTR keyValue, 
				__in LPCTSTR keyData);
		};
	}
}

#endif //  _USE_FILE_REG
