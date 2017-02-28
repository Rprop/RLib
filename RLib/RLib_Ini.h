/********************************************************************
	Created:	2012/07/26  10:22
	Filename: 	RLib_Ini.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_INI
#define _USE_INI

#include "RLib_Exception.h"
#include "RLib_List.h"
#include "RLib_String.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Ini
	{
		RLIB_INTERNAL_EXCEPTION(IniException, Exception);
		/// <summary>
		/// Ini elements container
		/// </summary>
		class RLIB_API IniElementCollection : public Collections::Generic::List<class IniElement *>
		{
		public:
			RLIB_DECLARE_DYNCREATE;
		};
		/// <summary>
		/// The base class of any ini elements
		/// </summary>
		class RLIB_API IniElement
		{
		public:
			String Value;

		public:
			IniElement() = default;
			RLIB_DECLARE_DYNCREATE;

		protected:
			IniElement(const String &value) : Value(value) {}

		public:
			virtual void Finalize() = 0;
			virtual void Print(IO::Stream *) const = 0;
			virtual class IniKey *TryGetAsKey() const = 0;
			virtual class IniSection *TryGetAsSection() const = 0;
			virtual class IniComment *TryGetAsComment() const = 0;
		};
		/// <summary>
		/// Represents ini key
		/// </summary>
		class RLIB_API IniKey : public IniElement
		{		
		public:
			String Name;

		public:		
			IniKey(const String &key = Nothing, const String &value = Nothing) : Name(key.Trim()), IniElement(value) {}
			RLIB_DECLARE_DYNCREATE;

		public:
			virtual void Finalize() override {
				delete this;
			}
			virtual IniKey *TryGetAsKey() const override final {
				return const_cast<IniKey *>(reinterpret_cast<const IniKey *>(this));
			}
			virtual class IniSection *TryGetAsSection() const override final {
				return nullptr;
			}
			virtual class IniComment *TryGetAsComment() const override final {
				return nullptr;
			}
			virtual void Print(IO::Stream *cout) const override final;

		public:
			operator String &() {
				return this->Value;
			}
			operator const String &() const {
				return this->Value;
			}
			IniKey &operator = (const String &val) {
				this->Value = val;
				return *this;
			}
		};
		/// <summary>
		/// Represents ini section(key-value container)
		/// </summary>
		class RLIB_API IniSection : public IniElement
		{
		public:
			IniElementCollection Items;
		
		public:		
			IniSection(const String &value) : IniElement(value) {
				assert(!this->Value.IsNullOrEmpty());
			}
			~IniSection() {
				while (this->Items.Length > 0) {
					this->Items[0]->Finalize();
					this->Items.RemoveAt(0);
				}
			}
			RLIB_DECLARE_DYNCREATE;

		public:
			virtual void Finalize() override {
				delete this;
			}
			virtual IniKey *TryGetAsKey() const override final {
				return nullptr;
			}
			virtual IniSection *TryGetAsSection() const override final {
				return (IniSection *)this;
			}
			virtual class IniComment *TryGetAsComment() const override final {
				return nullptr;
			}
			virtual void Print(IO::Stream *cout) const;

		public:
			IniKey &operator[](const String &key_name) {
				auto lpkey = this->GetKey(key_name);
				if (lpkey == nullptr) lpkey = this->AddKey(key_name);

				return *lpkey;
			}
			IniKey *GetKey(const String &key_name) const
			{
				foreach(pItem, this->Items)
				{
					if ((*pItem)->TryGetAsKey() != nullptr) {
						String &name = static_cast<IniKey *>(*pItem)->Name;
						if (name.IsNull()) {
							if (key_name.IsNull()) {
								return static_cast<IniKey *>(*pItem);
							} //if
						} else if (name == key_name) {
							return static_cast<IniKey *>(*pItem);
						} //if
					} //if
				}
				return nullptr;
			}
			bool RemoveKey(const String &key_name)
			{
				foreachList(itemIterator, this->Items)
				{
					if ((*itemIterator)->TryGetAsKey() != nullptr) {
						String &name = static_cast<IniKey *>(*itemIterator)->Name;
						if (name.IsNull()) {
							if (key_name.IsNull()) {
								static_cast<IniKey *>(*itemIterator)->Finalize();
								this->Items.Remove(itemIterator);
								return true;
							} //if
						} else if (name == key_name) {
							static_cast<IniKey *>(*itemIterator)->Finalize();
							this->Items.Remove(itemIterator);
							return true;
						} //if
					} //if
				}

				return false;
			}
			IniKey *AddKey(const String &key_name = Nothing, const String &key_value = Nothing)
			{
				return static_cast<IniKey *>(this->Items.AddLast(new IniKey(key_name, key_value))->Node);
			}
		};
		/// <summary>
		/// Represents ini comment
		/// </summary>
		class RLIB_API IniComment : public IniElement
		{
		protected:
			String CommentPrefix;

		public:
			IniComment(const String &value = Nothing, const String &prefix = Nothing) : IniElement(value) {
				this->CommentPrefix = prefix;
			}
			RLIB_DECLARE_DYNCREATE;

		public:
			virtual void Finalize() override {
				delete this;
			}
			virtual IniKey *TryGetAsKey() const override final {
				return nullptr;
			}
			virtual IniSection *TryGetAsSection() const override final {
				return nullptr;
			}
			virtual IniComment *TryGetAsComment() const override final {
				return const_cast<IniComment *>(this);
			}
			virtual void Print(IO::Stream *cout) const;
		};
		/// <summary>
		/// Represents ini file(reader/writer/formatter)
		/// </summary>
		class RLIB_API IniFile
		{
		protected:
			String               m_file;
			String               m_data;
			mutable IniException m_error;

		public:
			IniElementCollection Items;

		public:
			IniFile() = default;
			IniFile(const String &ini_file);
			~IniFile();
			RLIB_DECLARE_DYNCREATE;

		private:
			bool parse_ini();

		public:
			void Clear();
			void RemoveAt(intptr_t index);
			bool LoadFromString(const String &ini_string);
			bool LoadFromStream(const System::IO::Stream &ini_data, intptr_t ini_data_size);
			bool LoadFromFile(const String &ini_file);
			bool SaveFile(Text::Encoding codepage = Text::UnknownEncoding) const;
			bool SaveFile(const String &filename, Text::Encoding codepage = Text::UnknownEncoding) const;
			bool SaveFile(System::IO::Stream*, Text::Encoding codepage = Text::UnknownEncoding) const;
			const IniException *GetLastException() const {
				return &this->m_error;
			}

		public: // section operation
			IniSection *GetSection(const String &section_name) const;
			bool RemoveSection(const String &section_name);
			IniSection *AddSection(const String &section_name);

		public: // key operation
			IniKey *GetKey(const String &key_name) const;
			bool RemoveKey(const String &key_name);
			IniKey *AddKey(const String &key_name, const String &val = Nothing);

		public:
			IniSection &operator [] (const String &section_name);
			String Read(const String &section_name, const String &key_name, const String &def_val = Nothing) const;
			void Write(const String &section_name, const String &key_name, const String &val);

		public:
			int  ReadInt(const String &section_name, const String &key_name, int def_val = 0) const;
			void WriteInt(const String &section_name, const String &key_name, int val);
		};
	}
}
#endif // _USE_INI