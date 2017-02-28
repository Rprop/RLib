/********************************************************************
	Created:	2012/07/26  10:47
	Filename: 	RLib_Ini.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Ini.h"
#include "RLib_StringHelper.h"
#include "RLib_Object.h"
#include "RLib_File.h"
#include "RLib_Fundamental.h"

using namespace System::IO;
using namespace System::Ini;
using namespace System::Collections::Generic;

//-------------------------------------------------------------------------

IniFile::IniFile(const String &ini_file)
{
	this->m_file = ini_file;
	if (File::Exist(ini_file)) this->LoadFromFile(ini_file);
}

//-------------------------------------------------------------------------

IniFile::~IniFile()
{
	this->Clear();
}

//-------------------------------------------------------------------------

void IniFile::Clear()
{
	while(this->Items.Length > 0)
	{
		this->Items[0]->Finalize();
		this->Items.RemoveAt(0);
	}
}

//-------------------------------------------------------------------------

bool IniFile::LoadFromFile(const String &ini_file)
{
	ManagedObject<FileStream> file = File::Open(ini_file, FileMode::OpenExist, FileAccess::Read, FileShare::Read);
	if (!file) {
		Exception::FormatException(&this->m_error, Exception::GetLastErrorId());
		return false;
	} //if
	
	if (this->LoadFromStream(*file, file->Length)) {
		this->m_file = ini_file;
		return true;
	} //if

	return false;
}

//-------------------------------------------------------------------------

bool IniFile::LoadFromStream(const System::IO::Stream &ini_data, intptr_t ini_data_size)
{
	ManagedObject<BufferedStream> stream = Text::Encoder::ToCurrentEncoding(Text::UnknownEncoding,
																			ini_data, ini_data_size);
	if (!stream) {
		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, _T("failed to read text from stream"));
		return false;
	} //if

	assert(stream->Position == 0);
	auto counts = stream->Length / RLIB_SIZEOF(TCHAR);
	auto lpdata = this->m_data.reserve(counts).GetData();
	stream->Read(lpdata, stream->Length);
	lpdata[counts] = _T('\0');

	return this->parse_ini();
}

//-------------------------------------------------------------------------

bool IniFile::LoadFromString(const String &ini_string)
{
	assert(!ini_string.IsNullOrEmpty());

	this->m_data = ini_string;
	return this->parse_ini();
}

//-------------------------------------------------------------------------

static LPCTSTR skip_blank_chars(LPCTSTR ptr)
{
	if (ptr != nullptr) {
		while (*ptr != _T('\0') && *ptr != _T('\n') && _istspace(static_cast<unsigned int>(*ptr))) {
			++ptr;
		}
	} //if
	return ptr;
}

//-------------------------------------------------------------------------

bool IniFile::parse_ini()
{
	this->Clear();

	auto container  = &this->Items;
	auto ptr_begin  = this->m_data.GetConstData();
	auto ptr_prev   = ptr_begin, ptr_next  = ptr_prev;
	while((ptr_next = skip_blank_chars(ptr_next)) != nullptr)
	{
		switch(*ptr_next)
		{
		case _T('\n'):
			{
				if (ptr_next == ptr_prev) {
					// just new line
					container->Add(new IniKey);
				} else {
					String   line_string(ptr_prev, ptr_next - ptr_prev);
					intptr_t offset = line_string.IndexOf(_T("="));
					if (offset == -1) {
						// value not set
						container->Add(new IniKey(line_string.trim()));
					} else {
						container->Add(new IniKey(line_string.Substring(0, offset).trim(), line_string.Substring(offset + 1).trim()));
					} //if
				} //if
			}
			ptr_prev = ++ptr_next;
			break;
		case _T('#'):
		case _T(';'):
			{
				if (ptr_next != ptr_begin && *(ptr_next - 1) != _T('\n')) {
					// ignores non-standard comments, e.g. not at the beginning of a line
					goto __parse_ignored;
				} //if

				auto ptr_line = _tcschr(ptr_next, _T('\n'));
				if (ptr_line == nullptr) {
					container->Add(new IniComment(String(StringFromCString(ptr_prev + 1)).trim(), String(ptr_next, 1)));
					goto __parse_done;
				} //if
				
				container->Add(new IniComment(String(ptr_prev + 1, ptr_line - ptr_next - 1).trim(), String(ptr_next, 1)));
				ptr_next = ptr_line;
			}
			ptr_prev = ++ptr_next;
			break;
		case _T('['):
			{
				if ((ptr_next != ptr_begin) && *(ptr_next - 1) != _T('\n')) {
					// ignores non-standard sections, e.g. not at the beginning of a line
					goto __parse_ignored;
				} //if

				auto ptr_sec_end = _tcsstr(ptr_next, _T("]"));
				if (ptr_sec_end == nullptr) {
					RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, _T("unexpected EOF, expecting ']'"));
					goto __parse_fail;
				} //if

				auto lpsec = new IniSection(String(ptr_prev + 1, ptr_sec_end - ptr_prev - 1));
				if (lpsec == nullptr) {
					RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, _T("failed to create section"));
					goto __parse_fail;
				} //if
				this->Items.Add(lpsec);
				container = &lpsec->Items;

				ptr_next = skip_blank_chars(ptr_sec_end + 1);
				if (*ptr_next != _T('\n')) {
					RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, _T("unexpected characters, expecting newline after ']'"));
					goto __parse_fail;
				} //if
			}
			ptr_prev = ++ptr_next;
			break;
		case _T('\0'):
__parse_done:
			return true;
		default:
__parse_ignored:
			++ptr_next;
		}
	}

__parse_fail:
	return false;
}

//-------------------------------------------------------------------------

bool IniFile::SaveFile(Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	if (this->m_file.IsNullOrEmpty()) {
		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID,
						  _T("output file path cannot be empty"));
		return false;
	} //if

	return this->SaveFile(this->m_file, codepage);
}

//-------------------------------------------------------------------------

bool IniFile::SaveFile(const String &filename, 
					   Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	ManagedObject<FileStream> file = File::Create(filename, FileMode::CreateNew);
	if (file.IsNotNull()) {
		return this->SaveFile(file, codepage);
	} //if

	Exception::FormatException(&this->m_error, Exception::GetLastErrorId());
	return false;
}

//-------------------------------------------------------------------------

bool IniFile::SaveFile(System::IO::Stream *fp, Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	if (codepage == Text::UnknownEncoding) {
		codepage = Text::Encoder::GetCurrentEncoding();
	} //if

	switch (codepage)
	{
	case Text::UTF16Encoding:
	case Text::ASCIIEncoding:
	case Text::UTF8Encoding:
	case Text::UTF16BEEncoding:
		break;
	default:
		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, _T("codepage not supported"));
		return false;
	}

	ManagedObject<BufferedStream> output = new IO::BufferedStream(RLIB_DEFAULT_BUFFER_SIZE);
	if (output.IsNotNull()) {
		foreach(pItem, this->Items)
		{
			(*pItem)->Print(output);
		}

		output->Position = 0;
		Text::Encoder::WriteTextStream(*fp, *output, -1, true, codepage);

		return true;
	} //if

	return false;
}

//-------------------------------------------------------------------------

void IniSection::Print(IO::Stream *cout) const
{
	RLIB_StreamWrite(cout, "[");
	if (!this->Value.IsNullOrEmpty()) {
		RLIB_StreamWriteString(cout, this->Value);
	} else {
		RLIB_StreamWrite(cout, "null");
	} //if
	RLIB_StreamWrite(cout, "]");
	RLIB_StreamWrite(cout, RLIB_NEWLINEA);

	foreach(pItem, this->Items)
	{
		(*pItem)->Print(cout);
	}
}

//-------------------------------------------------------------------------

void IniKey::Print(IO::Stream *cout) const
{
	if (!this->Name.IsNullOrEmpty()) {
		RLIB_StreamWriteString(cout, this->Name);
		if (!this->Value.IsNullOrEmpty()) {
			RLIB_StreamWrite(cout, " = ");
			RLIB_StreamWriteString(cout, this->Value);
		} //if
	} //if
	RLIB_StreamWrite(cout, RLIB_NEWLINEA);
}

//-------------------------------------------------------------------------

void IniComment::Print(IO::Stream *cout) const
{
	if (!this->CommentPrefix.IsNullOrEmpty()) {
		RLIB_StreamWriteString(cout, this->CommentPrefix);
	} else {
		RLIB_StreamWrite(cout, ";");
	} //if

	if (!this->Value.IsNullOrEmpty()) {
		RLIB_StreamWriteString(cout, this->Value);
	} //if
	RLIB_StreamWrite(cout, RLIB_NEWLINEA);
}

//-------------------------------------------------------------------------

void IniFile::RemoveAt(intptr_t index)
{
	this->Items[index]->Finalize();
	this->Items.RemoveAt(index);
}

//-------------------------------------------------------------------------

IniSection *IniFile::GetSection(const String &section_name) const
{
	foreach(lppItem, this->Items)
	{
		if ((*lppItem)->TryGetAsSection() != nullptr) {
			String &value = static_cast<IniSection *>(*lppItem)->Value;
			if (value == section_name) {
				return static_cast<IniSection *>(*lppItem);
			} //if
		} //if
	}
	return nullptr;
}

//-------------------------------------------------------------------------

bool IniFile::RemoveSection(const String &section_name)
{
	foreachList(itemIterator, this->Items)
	{
		if ((*itemIterator)->TryGetAsSection() != nullptr) {
			String &value = static_cast<IniSection *>(*itemIterator)->Value;
			if (value == section_name) {
				static_cast<IniSection *>(*itemIterator)->Finalize();
				this->Items.Remove(itemIterator);
				return true;
			} //if
		} //if
	}

	return false;
}

//-------------------------------------------------------------------------

IniSection *IniFile::AddSection(const String &section_name)
{
	auto lpsection = this->GetSection(section_name);
	if (lpsection == nullptr) {
		lpsection = static_cast<IniSection *>(this->Items.AddLast(new IniSection(section_name))->Node);
	} //if
	return lpsection;
}

//-------------------------------------------------------------------------

IniKey *IniFile::GetKey(const String &key_name) const
{
	foreach(lppItem, this->Items)
	{
		if ((*lppItem)->TryGetAsKey() != nullptr) {
			if (static_cast<IniKey *>(*lppItem)->Name == key_name) {
				return static_cast<IniKey *>(*lppItem);
			} //if
		} //if
	}
	return nullptr;
}

//-------------------------------------------------------------------------

bool IniFile::RemoveKey(const String &key_name)
{
	foreachList(itemIterator, this->Items)
	{
		if ((*itemIterator)->TryGetAsKey() != nullptr) {
			if (static_cast<IniKey *>(*itemIterator)->Name == key_name) {
				static_cast<IniKey *>(*itemIterator)->Finalize();
				this->Items.Remove(itemIterator);
				return true;
			} //if
		} //if
	}

	return false;
}

//-------------------------------------------------------------------------

IniKey *IniFile::AddKey(const String &key_name, const String &val /* = Nothing */)
{
	auto lpkey = this->GetKey(key_name);
	if (lpkey == nullptr) {
		lpkey = static_cast<IniKey *>(this->Items.AddLast(new IniKey(key_name, val))->Node);
	} //if
	lpkey->Value = val;
	return lpkey;
}

//-------------------------------------------------------------------------

IniSection &IniFile::operator [] (const String &section_name)
{
	auto lpsec = this->AddSection(section_name);

	return *lpsec;
}

//-------------------------------------------------------------------------

String IniFile::Read(const String &section_name, const String &key_name, 
					 const String &def_val /* = Nothing */) const
{
	IniKey *lpkey = nullptr;
	if (!section_name.IsNullOrEmpty()) {
		auto lpsec = this->GetSection(section_name);
		if (lpsec != nullptr) {
			lpkey = lpsec->GetKey(key_name);		
		} //if
	} else {
		lpkey = this->GetKey(key_name);
	} //if

	return lpkey != nullptr && lpkey->Value != Nothing ? lpkey->Value : def_val;
}

//-------------------------------------------------------------------------

void IniFile::Write(const String &section_name, const String &key_name, const String &val)
{
	if (section_name.IsNullOrEmpty()) {
		this->AddKey(key_name, val);
		return;
	} //if

	auto lpsec = this->AddSection(section_name);
	auto lpkey = lpsec->GetKey(key_name);
	if (lpkey == nullptr) {
		if (val) lpsec->AddKey(key_name, val);
	} else {
		if (val) {
			lpkey->Value = val;
		} else {
			lpkey->Finalize();
		} //if
	} //if
}

//-------------------------------------------------------------------------

int IniFile::ReadInt(const String &section_name, const String &key_name, int def_val /* = 0 */) const
{
	String &&val = this->Read(section_name, key_name);
	return val.IsNullOrEmpty() ? def_val : Int32::TryParse(val);
}

//-------------------------------------------------------------------------

void IniFile::WriteInt(const String &section_name, const String &key_name, int val)
{
	this->Write(section_name, key_name, Int32(val).ToString());
}
