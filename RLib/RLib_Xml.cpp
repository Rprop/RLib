/*
www.sourceforge.net/projects/tinyxml
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "RLib_Xml.h"

#ifdef _USE_XML
#include "RLib_File.h"
#include "RLib_Fundamental.h"
#include "RLib_StringHelper.h"

using namespace System::Xml;

bool XmlBase::condenseWhiteSpace = true;

//-------------------------------------------------------------------------

void XmlBase::EncodeString(const String &str, String *outString)
{
	intptr_t i = 0;

	while (i < str.Length)
	{
		TCHAR c = str[i];

		if (c == _T('&') && i < (str.Length - 2) && str[i + 1] == _T('#') && str[i + 2] == _T('x'))
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no T(';').
			// There are actually 2 ways to exit this loop -
			// while fails (error case) and break (semicolon found).
			// However, there is no mechanism (currently) for
			// this function to return an error.
			while (i < str.Length - 1)
			{
				outString->append(str.c_str() + i, 1);
				++i;
				if (str[i] == _T(';')) {
					break;
				} //if
			}
		}
		else if (c == _T('&'))
		{
			outString->append(entity[0].str, static_cast<intptr_t>(entity[0].strLength));
			++i;
		}
		else if (c == _T('<'))
		{
			outString->append(entity[1].str, static_cast<intptr_t>(entity[1].strLength));
			++i;
		}
		else if (c == _T('>'))
		{
			outString->append(entity[2].str, static_cast<intptr_t>(entity[2].strLength));
			++i;
		}
		else if (c == _T('\"'))
		{
			outString->append(entity[3].str, static_cast<intptr_t>(entity[3].strLength));
			++i;
		}
		else if (c == _T('\''))
		{
			outString->append(entity[4].str, static_cast<intptr_t>(entity[4].strLength));
			++i;
		}
// 		else if ( c < 32 )
// 		{
// 			// Easy pass at non-alpha/numeric/symbol
// 			// Below 32 is symbolic.
// 			TCHAR buf[ 32 ];
// 			
// 			#if defined(RLIBXML_SNPRINTF)		
// 				RLIBXML_SNPRINTF( buf, sizeof(buf), T("&#x%02X;"), (unsigned) ( c & 0xff ) );
// 			#else
// 				_stprintf( buf, T("&#x%02X;"), (unsigned) ( c & 0xff ) );
// 			#endif		
// 
// 			//*ME:	warning C4267: convert 'size_t' to 'int'
// 			//*ME:	Int-Cast to make compiler happy ...
// 			outString->append( buf, (int)_tcslen( buf ) );
// 			++i;
// 		}
		else
		{
			*outString += c; // somewhat more efficient function call.
			++i;
		} //if
	} //if
}

//-------------------------------------------------------------------------

XmlNode::XmlNode(NodeType _type) : XmlBase()
{
	parent = 0;
	type   = _type;
	firstChild = 0;
	lastChild  = 0;
	prev = 0;
	next = 0;
}

//-------------------------------------------------------------------------

XmlNode::~XmlNode()
{
	XmlNode *node = firstChild;
	XmlNode *temp = 0;

	while (node)
	{
		temp = node;
		node = node->next;
		delete temp;
	}
}

//-------------------------------------------------------------------------

void XmlNode::CopyTo(XmlNode *target) const
{
	target->SetValue(value);
	target->userData = userData;
	target->location = location;
}

//-------------------------------------------------------------------------

void XmlNode::Clear()
{
	XmlNode *node = firstChild;
	XmlNode *temp = 0;

	while (node)
	{
		temp = node;
		node = node->next;
		delete temp;
	}

	firstChild = 0;
	lastChild  = 0;
}

//-------------------------------------------------------------------------

XmlNode *XmlNode::LinkEndChild(XmlNode *node)
{
	assert(node->parent == 0 || node->parent == this);
	assert(node->GetDocument() == 0 || node->GetDocument() == this->GetDocument());

	if (node->Type() == XmlNode::TINYXML_DOCUMENT) {
		delete node;
		if (GetDocument()) {
			GetDocument()->SetError(RLIBXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0);
		} //if
		return 0;
	} //if


	node->parent = this;

	node->prev = lastChild;
	node->next = 0;

	if (lastChild) {
		lastChild->next = node;
	} else {
		firstChild = node;
	} //if
	// it was an empty list.

	lastChild = node;
	return node;
}

//-------------------------------------------------------------------------

XmlNode *XmlNode::InsertEndChild(const XmlNode &addThis)
{
	if (addThis.Type() == XmlNode::TINYXML_DOCUMENT) {
		if (this->GetDocument()) {
			this->GetDocument()->SetError(RLIBXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0);
		} //if
		return 0;
	} //if

	XmlNode *node = addThis.Clone();
	if (!node) {
		return 0;
	} //if

	return this->LinkEndChild(node);
}

//-------------------------------------------------------------------------

XmlNode *XmlNode::InsertBeforeChild(XmlNode *beforeThis, const XmlNode &addThis)
{
	if (!beforeThis || beforeThis->parent != this) {
		return 0;
	} //if

	if (addThis.Type() == XmlNode::TINYXML_DOCUMENT) {
		if (GetDocument()) {
			GetDocument()->SetError(RLIBXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0);
		} //if
		return 0;
	} //if

	XmlNode *node = addThis.Clone();
	if (!node) {
		return 0;
	} //of
	node->parent = this;

	node->next = beforeThis;
	node->prev = beforeThis->prev;
	if (beforeThis->prev) {
		beforeThis->prev->next = node;
	} else {
		assert(firstChild == beforeThis);
		firstChild = node;
	} //if
	beforeThis->prev = node;
	return node;
}

//-------------------------------------------------------------------------

XmlNode *XmlNode::InsertAfterChild(XmlNode *afterThis, const XmlNode &addThis)
{
	if (!afterThis || afterThis->parent != this) {
		return 0;
	} //if

	if (addThis.Type() == XmlNode::TINYXML_DOCUMENT) {
		if (GetDocument()) {
			GetDocument()->SetError(RLIBXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0);
		}
		return 0;
	} //if

	XmlNode *node = addThis.Clone();
	if (!node) {
		return 0;
	} //if
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if (afterThis->next) {
		afterThis->next->prev = node;
	} else {
		assert(lastChild == afterThis);
		lastChild = node;
	} //if
	afterThis->next = node;
	return node;
}

//-------------------------------------------------------------------------

XmlNode *XmlNode::ReplaceChild(XmlNode *replaceThis, const XmlNode &withThis)
{
	if (!replaceThis) {
		return 0;
	} //if

	if (replaceThis->parent != this) {
		return 0;
	} //if

	if (withThis.ToDocument()) {
		// A document can never be a child.	Thanks to Noam.
		XmlDocument *document = GetDocument();
		if (document) {
			document->SetError(RLIBXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0);
		} //if
		return 0;
	} //if
	
	XmlNode *node = withThis.Clone();
	if (!node) {
		return 0;
	} //if

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if (replaceThis->next) {
		replaceThis->next->prev = node;
	} else {
		lastChild = node;
	} //if

	if (replaceThis->prev) {
		replaceThis->prev->next = node;
	} else {
		firstChild = node;
	} //if

	delete replaceThis;
	node->parent = this;
	return node;
}

//-------------------------------------------------------------------------

bool XmlNode::RemoveChild(XmlNode *removeThis)
{
	if (!removeThis)
	{
		return false;
	}

	if (removeThis->parent != this)
	{
		assert(0);
		return false;
	}

	if (removeThis->next)
	{
		removeThis->next->prev = removeThis->prev;
	}
	else
	{
		lastChild = removeThis->prev;
	}

	if (removeThis->prev)
	{
		removeThis->prev->next = removeThis->next;
	}
	else
	{
		firstChild = removeThis->next;
	}

	delete removeThis;
	return true;
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::FirstChild(const TCHAR *_value) const
{
	const XmlNode *node;
	for (node = firstChild; node; node = node->next)
	{
		if (_tcscmp(node->Value(), _value) == 0)
		{
			return node;
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::LastChild(const TCHAR *_value) const
{
	const XmlNode *node;
	for (node = lastChild; node; node = node->prev)
	{
		if (_tcscmp(node->Value(), _value) == 0)
		{
			return node;
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::IterateChildren(const XmlNode *previous) const
{
	if (!previous)
	{
		return FirstChild();
	}
	else
	{
		assert(previous->parent == this);
		return previous->NextSibling();
	}
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::IterateChildren(const TCHAR *val, const XmlNode *previous) const
{
	if (!previous)
	{
		return FirstChild(val);
	}
	else
	{
		assert(previous->parent == this);
		return previous->NextSibling(val);
	}
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::NextSibling(const TCHAR *_value) const
{
	const XmlNode *node;
	for (node = next; node; node = node->next)
	{
		if (_tcscmp(node->Value(), _value) == 0)
		{
			return node;
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlNode *XmlNode::PreviousSibling(const TCHAR *_value) const
{
	const XmlNode *node;
	for (node = prev; node; node = node->prev)
	{
		if (_tcscmp(node->Value(), _value) == 0)
		{
			return node;
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

void XmlElement::RemoveAttribute(const TCHAR *name)
{
	XmlAttribute *node = attributeSet.Find(name);
	if (node)
	{
		attributeSet.Remove(node);
		delete node;
	}
}

//-------------------------------------------------------------------------

const XmlElement *XmlNode::FirstChildElement() const
{
	const XmlNode *node;

	for (node = FirstChild(); node; node = node->NextSibling())
	{
		if (node->ToElement())
		{
			return node->ToElement();
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlElement *XmlNode::FirstChildElement(const TCHAR *_value) const
{
	const XmlNode *node;

	for (node = FirstChild(_value); node; node = node->NextSibling(_value))
	{
		if (node->ToElement())
		{
			return node->ToElement();
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlElement *XmlNode::NextSiblingElement() const
{
	const XmlNode *node;

	for (node = NextSibling(); node; node = node->NextSibling())
	{
		if (node->ToElement())
		{
			return node->ToElement();
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlElement *XmlNode::NextSiblingElement(const TCHAR *_value) const
{
	const XmlNode *node;

	for (node = NextSibling(_value); node; node = node->NextSibling(_value))
	{
		if (node->ToElement())
		{
			return node->ToElement();
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

const XmlDocument *XmlNode::GetDocument() const
{
	const XmlNode *node;

	for (node = this; node; node = node->parent)
	{
		if (node->ToDocument())
		{
			return node->ToDocument();
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

XmlElement::XmlElement(const TCHAR *_value) : XmlNode(XmlNode::TINYXML_ELEMENT)
{
	firstChild = lastChild = 0;
	value = _value;
}

//-------------------------------------------------------------------------

XmlElement::XmlElement(const XmlElement &copy) : XmlNode(XmlNode::TINYXML_ELEMENT)
{
	firstChild = lastChild = 0;
	copy.CopyTo(this);
}

//-------------------------------------------------------------------------

XmlElement &XmlElement::operator = (const XmlElement &base)
{
	ClearThis();
	base.CopyTo(this);
	return  *this;
}

//-------------------------------------------------------------------------

XmlElement::~XmlElement()
{
	ClearThis();
}

//-------------------------------------------------------------------------

void XmlElement::ClearThis()
{
	Clear();
	while (attributeSet.First())
	{
		XmlAttribute *node = attributeSet.First();
		attributeSet.Remove(node);
		delete node;
	}
}

//-------------------------------------------------------------------------

const String XmlElement::Attribute(const TCHAR *name) const
{
	const XmlAttribute *node = attributeSet.Find(name);
	if (node) {
		return node->Value();
	} //if

	return Nothing;
}

//-------------------------------------------------------------------------

const String XmlElement::Attribute(const TCHAR *name, int *i) const
{
	const XmlAttribute *attrib = attributeSet.Find(name);
	if (attrib) {
		const String &result = attrib->Value();
		if (i) {
			attrib->QueryIntValue(i);
		} //if
		return result;
	} //if

	return Nothing;
}

//-------------------------------------------------------------------------

const String XmlElement::Attribute(const TCHAR *name, double *d) const
{
	const XmlAttribute *attrib = attributeSet.Find(name);
	if (attrib) {
		const String &result = attrib->Value();
		if (d) {
			attrib->QueryDoubleValue(d);
		} //if
		return result;
	} //if

	return Nothing;
}

//-------------------------------------------------------------------------

int XmlElement::QueryIntAttribute(const TCHAR *name, int *ival) const
{
	const XmlAttribute *attrib = attributeSet.Find(name);
	if (!attrib)
	{
		return RLIBXML_NO_ATTRIBUTE;
	}
	return attrib->QueryIntValue(ival);
}

//-------------------------------------------------------------------------

int XmlElement::QueryUnsignedAttribute(const TCHAR *name, unsigned *lpvalue) const
{
	const XmlAttribute *node = attributeSet.Find(name);
	if (!node)
	{
		return RLIBXML_NO_ATTRIBUTE;
	}

	int ival = 0;
	int result = node->QueryIntValue(&ival);
	*lpvalue = static_cast<unsigned>(ival);
	return result;
}

//-------------------------------------------------------------------------

int XmlElement::QueryBoolAttribute(const TCHAR *name, bool *bval) const
{
	const XmlAttribute *node = attributeSet.Find(name);
	if (!node) {
		return RLIBXML_NO_ATTRIBUTE;
	} //if

	int result = RLIBXML_WRONG_TYPE;
	if (StringEqual(node->Value(), _T("true"), true) || StringEqual(node->Value(), _T("yes"), true) || StringEqual(node->Value(), _T("1"), true))
	{
		*bval = true;
		result = RLIBXML_SUCCESS;
	} else if (StringEqual(node->Value(), _T("false"), true) || StringEqual(node->Value(), _T("no"), true) || StringEqual(node->Value(), _T("0"), true)) {
		*bval = false;
		result = RLIBXML_SUCCESS;
	} //if
	return result;
}

//-------------------------------------------------------------------------

int XmlElement::QueryDoubleAttribute(const TCHAR *name, double *dval) const
{
	const XmlAttribute *attrib = attributeSet.Find(name);
	if (!attrib)
	{
		return RLIBXML_NO_ATTRIBUTE;
	}
	return attrib->QueryDoubleValue(dval);
}

//-------------------------------------------------------------------------

void XmlElement::SetAttribute(const TCHAR *name, int val)
{
	XmlAttribute *attrib = attributeSet.FindOrCreate(name);
	if (attrib)
	{
		attrib->SetIntValue(val);
	}
}

//-------------------------------------------------------------------------

void XmlElement::SetDoubleAttribute(const TCHAR *name, double val)
{
	XmlAttribute *attrib = attributeSet.FindOrCreate(name);
	if (attrib)
	{
		attrib->SetDoubleValue(val);
	}
}

//-------------------------------------------------------------------------

void XmlElement::SetAttribute(const TCHAR *cname, const TCHAR *cvalue)
{
	XmlAttribute *attrib = attributeSet.FindOrCreate(cname);
	if (attrib)
	{
		attrib->SetValue(cvalue);
	}
}

//-------------------------------------------------------------------------

void XmlElement::Print(IO::Stream *cfile, int depth) const
{
	int i;
	assert(cfile);
	for (i = 0; i < depth; i++)
	{
		RLIB_StreamWrite(cfile, "    ");
	}

	RLIB_StreamWrite(cfile, "<");
	RLIB_StreamWriteString(cfile, value);

	const XmlAttribute *attrib;
	for (attrib = attributeSet.First(); attrib; attrib = attrib->Next())
	{
		RLIB_StreamWrite(cfile, " ");
		attrib->Print(cfile, depth);
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	XmlNode *node;
	if (!firstChild)
	{
		RLIB_StreamWrite(cfile, " />");
	}
	else if (firstChild == lastChild && firstChild->ToText())
	{
		RLIB_StreamWrite(cfile, ">");
		firstChild->Print(cfile, depth + 1);
		RLIB_StreamWrite(cfile, "</");
		RLIB_StreamWriteString(cfile, value);
		RLIB_StreamWrite(cfile, ">");
	}
	else
	{
		RLIB_StreamWrite(cfile, ">");

		for (node = firstChild; node; node = node->NextSibling())
		{
			if (!node->ToText())
			{
				RLIB_StreamWrite(cfile, RLIB_NEWLINEA);
			}
			node->Print(cfile, depth + 1);
		}
		RLIB_StreamWrite(cfile, RLIB_NEWLINEA);
		for (i = 0; i < depth; ++i)
		{
			RLIB_StreamWrite(cfile, "    ");
		}
		RLIB_StreamWrite(cfile, "</");
		RLIB_StreamWriteString(cfile, value);
		RLIB_StreamWrite(cfile, ">");
	}
}

//-------------------------------------------------------------------------

void XmlElement::CopyTo(XmlElement *target) const
{
	// superclass:
	XmlNode::CopyTo(target);

	// Element class: 
	// Clone the attributes, then clone the children.
	const XmlAttribute *attribute = 0;
	for (attribute = attributeSet.First(); attribute; attribute = attribute->Next())
	{
		target->SetAttribute(attribute->Name(), attribute->Value());
	}

	XmlNode *node = 0;
	for (node = firstChild; node; node = node->NextSibling())
	{
		target->LinkEndChild(node->Clone());
	}
}

//-------------------------------------------------------------------------

bool XmlElement::Accept(XmlVisitor *visitor) const
{
	if (visitor->VisitEnter(*this, attributeSet.First()))
	{
		for (const XmlNode *node = FirstChild(); node; node = node->NextSibling())
		{
			if (!node->Accept(visitor))
			{
				break;
			}
		}
	}
	return visitor->VisitExit(*this);
}

//-------------------------------------------------------------------------

XmlNode *XmlElement::Clone() const
{
	XmlElement *clone = new XmlElement(Value());
	if (!clone)
	{
		return 0;
	}

	CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------

const String XmlElement::GetText() const
{
	const XmlNode *child = this->FirstChild();
	if (child) {
		const XmlText *childText = child->ToText();
		if (childText) {
			return childText->Value();
		} //if
	} //if
	return Nothing;
}

//-------------------------------------------------------------------------

XmlDocument::XmlDocument() : XmlNode(XmlNode::TINYXML_DOCUMENT)
{
	tabsize = 4;
	ClearError();
}

//-------------------------------------------------------------------------

XmlDocument::XmlDocument(const String &documentName) : XmlNode(XmlNode::TINYXML_DOCUMENT)
{
	tabsize = 4;
	value = documentName;
	ClearError();
}

//-------------------------------------------------------------------------

XmlDocument::XmlDocument(const XmlDocument &copy) : XmlNode(XmlNode::TINYXML_DOCUMENT)
{
	copy.CopyTo(this);
}

//-------------------------------------------------------------------------

XmlDocument &XmlDocument::operator = (const XmlDocument &copy)
{
	Clear();
	copy.CopyTo(this);
	return *this;
}

//-------------------------------------------------------------------------

bool XmlDocument::LoadFile()
{
	// reading in binary mode so that tinyxml can normalize the EOL
	System::IO::FileStream *file = System::IO::File::Open(value);
	if (file == nullptr) {
		SetError(RLIBXML_ERROR_OPENING_FILE, 0, 0);
		return false;
	} //if

	bool result = LoadFromStream(file);

	delete file;

	return result;
}

//-------------------------------------------------------------------------

bool XmlDocument::LoadFile(const String &filename)
{
	value = filename;

	return LoadFile();
}

//-------------------------------------------------------------------------

bool XmlDocument::LoadFromStream(IO::Stream *pstream, Text::Encoding codepage /* = 0 */)
{
	if (pstream == nullptr) {
		SetError(RLIBXML_ERROR_READING_STREAM, 0, 0);
		return false;
	} //if

	// Delete the existing data:
	Clear();
	location.Clear();

	// Strange case, but good to handle up front.
	if (pstream->MaxReadSize <= 4) {
		SetError(RLIBXML_ERROR_DOCUMENT_EMPTY, 0, 0);
		return false;
	} //if

	if (codepage != Text::Encoder::GetCurrentEncoding()) {
		pstream = Text::Encoder::ToCurrentEncoding(codepage, *pstream);	
		if (pstream == nullptr) {
			SetError(RLIBXML_ERROR_READING_STREAM, 0, 0);
			return false;
		} //if
		assert(pstream->Position == 0);
	} //if

	auto bufsize = pstream->MaxReadSize;

	// Allocate Buffer Memory
	auto buf = RLIB_GlobalAllocAny(TCHAR *, bufsize + sizeof(TCHAR));

	//  Read Data
	if (buf) {
		pstream->Read(buf, bufsize);
		buf[bufsize / sizeof(TCHAR)] = _T('\0');
	} //if

	if (codepage != Text::Encoder::GetCurrentEncoding()) {
		delete static_cast<IO::BufferedStream *>(pstream);
	} //if

	if (buf == nullptr) {
		SetError(RLIBXML_ERROR_MEMORY_NOT_ENOUGH, 0, 0);
		return false;
	} //if

	// Parse Data
	Parse(buf, 0);

	// Collect Buffer Memory
	RLIB_GlobalCollect(buf);

	// Return if error
	return this->errorException.HResult != STATUS_SUCCESS;
}

//-------------------------------------------------------------------------

bool XmlDocument::SaveFile(Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	return SaveFile(this->Value(), codepage);
}

//-------------------------------------------------------------------------

bool XmlDocument::SaveFile(const String &filename, 
						   Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	IO::FileStream *file = IO::File::Create(filename, IO::FileMode::CreateNew);
	if (file != nullptr) {
		bool result = SaveFile(file, codepage);
		delete file;
		return result;
	} //if

	Exception::FormatException(&this->errorException, Exception::GetLastErrorId());
	return false;
}

//-------------------------------------------------------------------------

bool XmlDocument::SaveFile(System::IO::Stream *fp, 
						   Text::Encoding codepage /* = Text::UnknownEncoding */) const
{
	if (codepage == 0)
	{
#ifdef _UNICODE
		codepage = Text::UTF16Encoding;
#else 
		codepage = Text::ASCIIEncoding;
#endif // _UNICODE
	} //if

	LPCTSTR strWebName;
#pragma warning(disable:4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
	switch (codepage)
	{
		case System::Text::UTF8Encoding: strWebName = _T("utf-8");
			break;
		case System::Text::UTF16Encoding: strWebName = _T("utf-16");
			break;
		case System::Text::ASCIIEncoding: strWebName = _T("gb2312");
			break;
		case System::Text::UTF16BEEncoding: strWebName = _T("unicodeFFFE");
			break;
		default:
			RLIB_SetException(this->errorException, RLIB_EXCEPTION_ID,
							  _T("codepage not supported"));
			trace(!"codepage not supported");
			return false;
	}
#pragma warning(default:4061)

	if (FirstChild()->ToDeclaration() != nullptr)
	{
		const_cast<XmlElement *>(static_cast<const XmlElement *>(FirstChild()))
			->ToDeclaration()->encoding.copy(strWebName);
	} //if

	IO::BufferedStream *output = new IO::BufferedStream(4096);

	Print(output);

	output->Position = 0;

	bool result = Text::Encoder::WriteTextStream(*fp, *output, -1, true, codepage);
	if (!result) {
		RLIB_SetException(this->errorException, RLIB_EXCEPTION_ID,
						  _T("failed to write output data"));
	} //if

	delete output;

	return result;
}

//-------------------------------------------------------------------------


void XmlDocument::CopyTo(XmlDocument *target) const
{
	XmlNode::CopyTo(target);

	target->errorException.Ref(this->errorException);
	target->tabsize = tabsize;
	target->errorLocation = errorLocation;

	XmlNode *node = 0;
	for (node = firstChild; node; node = node->NextSibling())
	{
		target->LinkEndChild(node->Clone());
	}
}

//-------------------------------------------------------------------------


XmlNode *XmlDocument::Clone() const
{
	XmlDocument *clone = new XmlDocument();
	if (!clone) {
		return 0;
	} //if

	this->CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------


void XmlDocument::Print(IO::Stream *cfile, int depth) const
{
	assert(cfile);
	for (const XmlNode *node = FirstChild(); node; node = node->NextSibling())
	{
		node->Print(cfile, depth);
		RLIB_StreamWrite(cfile, RLIB_NEWLINEA);
	}
}

//-------------------------------------------------------------------------


bool XmlDocument::Accept(XmlVisitor *visitor) const
{
	if (visitor->VisitEnter(*this))
	{
		for (const XmlNode *node = FirstChild(); node; node = node->NextSibling())
		{
			if (!node->Accept(visitor)) {
				break;
			} //if
		}
	}
	return visitor->VisitExit(*this);
}

//-------------------------------------------------------------------------

const XmlAttribute *XmlAttribute::Next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if (next->value.IsNullOrEmpty() && next->name.IsNullOrEmpty()) {
		return 0;
	} //if
	return next;
}

/*
XmlAttribute* XmlAttribute::Next()
{
// We are using knowledge of the sentinel. The sentinel
// have a value or name.
if ( next->value.empty() && next->name.empty() )
return 0;
return next;
}
*/

const XmlAttribute *XmlAttribute::Previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if (prev->value.IsNullOrEmpty() && prev->name.IsNullOrEmpty()) {
		return 0;
	} //if
	return prev;
}

/*
XmlAttribute* XmlAttribute::Previous()
{
// We are using knowledge of the sentinel. The sentinel
// have a value or name.
if ( prev->value.empty() && prev->name.empty() )
return 0;
return prev;
}
*/

void XmlAttribute::Print(IO::Stream *cfile, int /*depth*/, String *str) const
{
	String n, v;

	EncodeString(name, &n);
	EncodeString(value, &v);

	if (value.IndexOf(_T('\"')) == -1) {
		if (cfile) {
			RLIB_StreamWriteString(cfile, n);
			RLIB_StreamWrite(cfile, "=\"");
			RLIB_StreamWriteString(cfile, v);
			RLIB_StreamWrite(cfile, "\"");
		} //if

		if (str) {
			(*str) += n;
			(*str) += _R("=\"");
			(*str) += v;
			(*str) += _R("\"");
		} //if
	} else {
		if (cfile) {
			RLIB_StreamWriteString(cfile, n);
			RLIB_StreamWrite(cfile, "='");
			RLIB_StreamWriteString(cfile, v);
			RLIB_StreamWrite(cfile, "'");
		} //if

		if (str) {
			(*str) += n;
			(*str) += _T("='");
			(*str) += v;
			(*str) += _T("'");
		} //if
	} //if
}

//-------------------------------------------------------------------------

int XmlAttribute::QueryIntValue(int *ival) const
{
	*ival = Int32::TryParse(value);
	auto err = errno;
	return err != EINVAL && err != ERANGE ? RLIBXML_SUCCESS : RLIBXML_WRONG_TYPE;
}

//-------------------------------------------------------------------------

int XmlAttribute::QueryDoubleValue(double *dval) const
{
	*dval = Double::TryParse(value);
	auto err = errno;
	return err != EINVAL && err != ERANGE ? RLIBXML_SUCCESS : RLIBXML_WRONG_TYPE;
}

//-------------------------------------------------------------------------

void XmlAttribute::SetIntValue( int _value )
{
	SetValue(ToInt32(_value).ToString());
}

//-------------------------------------------------------------------------

void XmlAttribute::SetDoubleValue( double _value )
{
	SetValue(Double(_value).ToString());
}

//-------------------------------------------------------------------------

int XmlAttribute::IntValue() const
{
	return _tstoi(value.GetConstData());
}

//-------------------------------------------------------------------------

double XmlAttribute::DoubleValue() const
{
	return _tstof(value.GetConstData());
}

//-------------------------------------------------------------------------

XmlComment::XmlComment(const XmlComment &copy) : XmlNode(XmlNode::TINYXML_COMMENT)
{
	copy.CopyTo(this);
}

//-------------------------------------------------------------------------

XmlComment &XmlComment::operator = (const XmlComment &base)
{
	this->Clear();
	base.CopyTo(this);
	return  *this;
}

//-------------------------------------------------------------------------

void XmlComment::Print(IO::Stream *cfile, int depth) const
{
	assert(cfile);
	for (int i = 0; i < depth; i++)
	{
		RLIB_StreamWrite(cfile, "    ");
	}
	RLIB_StreamWrite(cfile, "<!-- ");
	RLIB_StreamWriteString(cfile, value);
	RLIB_StreamWrite(cfile, " -->");
}

//-------------------------------------------------------------------------

void XmlComment::CopyTo(XmlComment *target) const
{
	XmlNode::CopyTo(target);
}

//-------------------------------------------------------------------------

bool XmlComment::Accept(XmlVisitor *visitor) const
{
	return visitor->Visit(*this);
}

//-------------------------------------------------------------------------

XmlNode *XmlComment::Clone() const
{
	XmlComment *clone = new XmlComment();

	if (!clone) {
		return 0;
	} //if

	CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------

void XmlText::Print(IO::Stream *cfile, int depth) const
{
	assert(cfile);
	if (cdata) {
		RLIB_StreamWrite(cfile, RLIB_NEWLINEA);
		for (int i = 0; i < depth; ++i) {
			RLIB_StreamWrite(cfile, "    ");
		}
		RLIB_StreamWrite(cfile, "<![CDATA[");
		RLIB_StreamWrite(cfile, "]]>\n");
		RLIB_StreamWriteString(cfile, value); // unformatted output
	} else {
		String buffer;
		EncodeString(value, &buffer);
		RLIB_StreamWriteString(cfile, buffer);
	} //if
}

//-------------------------------------------------------------------------

void XmlText::CopyTo(XmlText *target) const
{
	XmlNode::CopyTo(target);
	target->cdata = cdata;
}

//-------------------------------------------------------------------------

bool XmlText::Accept(XmlVisitor *visitor) const
{
	return visitor->Visit(*this);
}

//-------------------------------------------------------------------------

XmlNode *XmlText::Clone() const
{
	XmlText *clone = new XmlText(_T(""));

	if (!clone) {
		return 0;
	} //if

	CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------

XmlDeclaration::XmlDeclaration(const String &_version, const String &_encoding /* = Nothing */,
							   const String &_standalone /* = Nothing */) : XmlNode(XmlNode::TINYXML_DECLARATION)
{
	version    = _version;
	encoding   = _encoding;
	standalone = _standalone;
}

//-------------------------------------------------------------------------

XmlDeclaration::XmlDeclaration(const XmlDeclaration &copy) : XmlNode(XmlNode::TINYXML_DECLARATION)
{
	copy.CopyTo(this);
}

//-------------------------------------------------------------------------

XmlDeclaration &XmlDeclaration::operator = (const XmlDeclaration &copy)
{
	Clear();
	copy.CopyTo(this);
	return  *this;
}

//-------------------------------------------------------------------------

void XmlDeclaration::Print(IO::Stream *cfile, int /*depth*/, String *str) const
{
	if (cfile) {
		RLIB_StreamWrite(cfile, "<?xml ");
	} //if

	if (str) {
		(*str) += _T("<?xml ");
	} //if

	if (!version.IsNullOrEmpty()) {
		if (cfile) {
			RLIB_StreamWrite(cfile, "version=\"");
			RLIB_StreamWriteString(cfile, version);
			RLIB_StreamWrite(cfile, "\" ");
		} //if
		if (str) {
			(*str) += _R("version=\"");
			(*str) += version;
			(*str) += _R("\" ");
		} //if
	} //if

	if (!encoding.IsNullOrEmpty()) {
		if (cfile) {
			RLIB_StreamWrite(cfile, "encoding=\"");
			RLIB_StreamWriteString(cfile, encoding);
			RLIB_StreamWrite(cfile, "\" ");
		} //if

		if (str) {
			(*str) += _R("encoding=\"");
			(*str) += encoding;
			(*str) += _R("\" ");
		} //if
	} //if

	if (!standalone.IsNullOrEmpty()) {
		if (cfile) {
			RLIB_StreamWrite(cfile, "standalone=\"");
			RLIB_StreamWriteString(cfile, standalone);
			RLIB_StreamWrite(cfile, "\" ");
		} //if

		if (str) {
			(*str) += _R("standalone=\"");
			(*str) += standalone;
			(*str) += _R("\" ");
		} //if
	} //if

	if (cfile) {
		RLIB_StreamWrite(cfile, "?>");
	} //if

	if (str) {
		(*str) += _R("?>");
	} //if
}

//-------------------------------------------------------------------------

void XmlDeclaration::CopyTo(XmlDeclaration *target) const
{
	XmlNode::CopyTo(target);

	target->version    = version;
	target->encoding   = encoding;
	target->standalone = standalone;
}

//-------------------------------------------------------------------------

bool XmlDeclaration::Accept(XmlVisitor *visitor) const
{
	return visitor->Visit(*this);
}

//-------------------------------------------------------------------------

XmlNode *XmlDeclaration::Clone() const
{
	XmlDeclaration *clone = new XmlDeclaration();

	if (!clone) {
		return 0;
	} //if

	this->CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------

void XmlUnknown::Print(IO::Stream *cfile, int depth) const
{
	for (int i = 0; i < depth; i++)
	{
		RLIB_StreamWrite(cfile, "    ");
	}

	RLIB_StreamWrite(cfile, "<");
	RLIB_StreamWriteString(cfile, value);
	RLIB_StreamWrite(cfile, ">");
}

//-------------------------------------------------------------------------

void XmlUnknown::CopyTo(XmlUnknown *target) const
{
	XmlNode::CopyTo(target);
}

//-------------------------------------------------------------------------

bool XmlUnknown::Accept(XmlVisitor *visitor) const
{
	return visitor->Visit(*this);
}

//-------------------------------------------------------------------------

XmlNode *XmlUnknown::Clone() const
{
	XmlUnknown *clone = new XmlUnknown();

	if (!clone) {
		return 0;
	} //if

	this->CopyTo(clone);
	return clone;
}

//-------------------------------------------------------------------------

XmlAttributeSet::XmlAttributeSet()
{
	sentinel.next = &sentinel;
	sentinel.prev = &sentinel;
}

//-------------------------------------------------------------------------

XmlAttributeSet::~XmlAttributeSet()
{
	assert(sentinel.next == &sentinel);
	assert(sentinel.prev == &sentinel);
}

//-------------------------------------------------------------------------

void XmlAttributeSet::Add(XmlAttribute *addMe)
{
	assert(!Find(addMe->Name())); // Shouldn't be multiply adding to the set.

	addMe->next = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next = addMe;
	sentinel.prev = addMe;
}

//-------------------------------------------------------------------------

void XmlAttributeSet::Remove(XmlAttribute *removeMe)
{
	XmlAttribute *node;

	for (node = sentinel.next; node != &sentinel; node = node->next)
	{
		if (node == removeMe)
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			node->next = 0;
			node->prev = 0;
			return;
		}
	}
	assert(0); // we tried to remove a non-linked attribute.
}

//-------------------------------------------------------------------------

XmlAttribute *XmlAttributeSet::Find(const TCHAR *name) const
{
	for (XmlAttribute *node = sentinel.next; node != &sentinel; node = node->next)
	{
		if (name != nullptr && node->name == name)
		{
			return node;
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

XmlAttribute *XmlAttributeSet::FindOrCreate(const TCHAR *_name)
{
	XmlAttribute *attrib = Find(_name);
	if (!attrib)
	{
		attrib = new XmlAttribute();
		Add(attrib);
		attrib->SetName(_name);
	}
	return attrib;
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::FirstChild() const
{
	if (node) {
		XmlNode *child = node->FirstChild();
		if (child) {
			return XmlHandle(child);
		} //if
	} //if
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::FirstChild(const TCHAR *value) const
{
	if (node) {
		XmlNode *child = node->FirstChild(value);
		if (child) {
			return XmlHandle(child);
		} //if
	} //if
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::FirstChildElement() const
{
	if (node) {
		XmlElement *child = node->FirstChildElement();
		if (child) {
			return XmlHandle(child);
		} //if
	} //if
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::FirstChildElement(const TCHAR *value) const
{
	if (node)
	{
		XmlElement *child = node->FirstChildElement(value);
		if (child)
		{
			return XmlHandle(child);
		}
	}
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::Child(int count) const
{
	if (node) {
		int i;
		XmlNode *child = node->FirstChild();
		for (i = 0; child && i < count; child = child->NextSibling(), ++i) {
			// nothing
		}
		if (child) {
			return XmlHandle(child);
		} //if
	} //if
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::Child(const TCHAR *value, int count) const
{
	if (node)
	{
		int i;
		XmlNode *child = node->FirstChild(value);
		for (i = 0; child && i < count; child = child->NextSibling(value), ++i)
		{
			// nothing
		}
		if (child)
		{
			return XmlHandle(child);
		}
	}
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::ChildElement(int count) const
{
	if (node)
	{
		int i;
		XmlElement *child = node->FirstChildElement();
		for (i = 0; child && i < count; child = child->NextSiblingElement(), ++i)
		{
			// nothing
		}
		if (child)
		{
			return XmlHandle(child);
		}
	}
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

XmlHandle XmlHandle::ChildElement(const TCHAR *value, int count) const
{
	if (node)
	{
		int i;
		XmlElement *child = node->FirstChildElement(value);
		for (i = 0; child && i < count; child = child->NextSiblingElement(value), ++i)
		{
			// nothing
		}
		if (child)
		{
			return XmlHandle(child);
		}
	}
	return XmlHandle(0);
}

//-------------------------------------------------------------------------

bool XmlPrinter::VisitEnter(const XmlDocument &)
{
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::VisitExit(const XmlDocument &)
{
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::VisitEnter(const XmlElement &element, const XmlAttribute *firstAttribute)
{
	DoIndent();
	buffer += _T("<");
	buffer += element.Value();

	for (const XmlAttribute *attrib = firstAttribute; attrib; attrib = attrib->Next())
	{
		buffer += _T(" ");
		attrib->Print(0, 0, &buffer);
	}

	if (!element.FirstChild())
	{
		buffer += _T(" />");
		DoLineBreak();
	}
	else
	{
		buffer += _T(">");
		if (element.FirstChild()->ToText() && element.LastChild() == element.FirstChild() && element.FirstChild()->ToText()->CDATA() == false)
		{
			simpleTextPrint = true;
			// no DoLineBreak()!
		}
		else
		{
			DoLineBreak();
		}
	}
	++depth;
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::VisitExit(const XmlElement &element)
{
	--depth;
	if (!element.FirstChild())
	{
		// nothing.
	}
	else
	{
		if (simpleTextPrint)
		{
			simpleTextPrint = false;
		}
		else
		{
			DoIndent();
		}
		buffer += _T("</");
		buffer += element.Value();
		buffer += _T(">");
		DoLineBreak();
	}
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::Visit(const XmlText &text)
{
	if (text.CDATA()) {
		this->DoIndent();
		buffer += _R("<![CDATA[");
		buffer += text.Value();
		buffer += _R("]]>");
		this->DoLineBreak();
	} else {
		String str;
		XmlBase::EncodeString(text.Value(), &str);
		if (simpleTextPrint) {
			buffer += str;
		} else {
			this->DoIndent();
			buffer += str;
			this->DoLineBreak();
		}
	} //if

	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::Visit(const XmlDeclaration &declaration)
{
	this->DoIndent();
	declaration.Print(0, 0, &buffer);
	this->DoLineBreak();
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::Visit(const XmlComment &comment)
{
	this->DoIndent();
	buffer += _R("<!--");
	buffer += comment.Value();
	buffer += _R("-->");
	this->DoLineBreak();
	return true;
}

//-------------------------------------------------------------------------

bool XmlPrinter::Visit(const XmlUnknown &unknown)
{
	this->DoIndent();
	buffer += _R("<");
	buffer += unknown.Value();
	buffer += _R(">");
	this->DoLineBreak();
	return true;
}

#endif // _USE_XML
