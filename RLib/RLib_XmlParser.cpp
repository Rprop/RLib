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
#include "RLib_StringHelper.h"

#ifdef _USE_XML

#pragma warning(default:4710)

using namespace System::Xml;

//#define DEBUG_PARSER
#if defined( DEBUG_PARSER )
#if defined( DEBUG ) && defined( _MSC_VER )
#include <windows.h>
#define RLIBXML_LOG OutputDebugString
#else 
#define RLIBXML_LOG printf
#endif 
#endif 

// Note tha T("PutString") hardcodes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break putstring.	
XmlBase::Entity XmlBase::entity[XmlBase::NUM_ENTITY] =
{
	{
		_T("&amp;"), 5, _T('&')
	}
	,
	{
		_T("&lt;"), 4, _T('<')
	}
	,
	{
		_T("&gt;"), 4, _T('>')
	}
	,
	{
		_T("&quot;"), 6, _T('\"')
	}
	,
	{
		_T("&apos;"), 6, _T('\'')
	}
};

/*static*/int XmlBase::IsAlpha(TCHAR anyByte)
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

	//	if ( encoding == RLIBXML_ENCODING_UTF8 )
	//	{
	if (anyByte < 127)
	{
		return isalpha(anyByte);
	}
	else
	{
		return 1;
	}
	// What else to do? The unicode set is huge...get the english ones right.
	//	}
	//	else
	//	{
	//		return isalpha( anyByte );
	//	}
}


/*static*/int XmlBase::IsAlphaNum(TCHAR anyByte)
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

	//	if ( encoding == RLIBXML_ENCODING_UTF8 )
	//	{
	if (anyByte < 127)
	{
		return isalnum(anyByte);
	}
	else
	{
		return 1;
	}
	// What else to do? The unicode set is huge...get the english ones right.
	//	}
	//	else
	//	{
	//		return isalnum( anyByte );
	//	}
}

//-------------------------------------------------------------------------

class System::Xml::XmlParsingData
{
	friend class XmlDocument;
public:
	void Stamp(const TCHAR *now);

	const XmlCursor &Cursor()const
	{
		return cursor;
	}

private:
	// Only used by the document!
	XmlParsingData(const TCHAR *start, int _tabsize, int row, int col)
	{
		assert(start);
		stamp = start;
		tabsize = _tabsize;
		cursor.row = row;
		cursor.col = col;
	}

	XmlCursor cursor;
	const TCHAR *stamp;
	int tabsize;
};

void XmlParsingData::Stamp(const TCHAR *now)
{
	assert(now);

	// Do nothing if the tabsize is 0.
	if (tabsize < 1)
	{
		return;
	}

	// Get the current row, column.
	int row = cursor.row;
	int col = cursor.col;
	const TCHAR *p = stamp;
	assert(p);

	while (p < now)
	{
		// Treat p as unsigned, so we have a happy compiler.
		const TCHAR *pU = p;

		// Code contributed by Fletcher Dunn: (modified by lee)
		switch (*pU)
		{
			case 0:
				// We *should* never get here, but in case we do, don't
				// advance past the terminating null character, ever
				return;

			case _T('\r'):
				// bump down to the next line
				++row;
				col = 0;
				// Eat the character
				++p;

				// Check for \r\n sequence, and treat this as a single character
				if (*p == _T('\n')) {
					++p;
				} //if
				break;

			case _T('\n'):
				// bump down to the next line
				++row;
				col = 0;

				// Eat the character
				++p;

				// Check for \n\r sequence, and treat this as a single
				// character.  (Yes, this bizarre thing does occur still
				// on some arcane platforms...)
				if (*p == _T('\r')) {
					++p;
				} //if
				break;

			case _T('\t'):
				// Eat the character
				++p;

				// Skip to next tab stop
				col = (col / tabsize + 1) *tabsize;
				break;

			default:
				++p;
				++col;
				break;
		}
	}
	cursor.row = row;
	cursor.col = col;
	assert(cursor.row >= -1);
	assert(cursor.col >= -1);
	stamp = p;
	assert(stamp);
}

//-------------------------------------------------------------------------

const TCHAR *XmlBase::SkipWhiteSpace(const TCHAR *p)
{
	if (!p || !*p)
	{
		return 0;
	}
	//rrrfff changed: in bytes
	while (*p && IsWhiteSpace(*p))
	{
		++p;
	}
	return p;
}

// One of TinyXML's more performance demanding functions. Try to keep the memory overhead down. The
// T("assign") optimization removes over 10% of the execution time.
//
const TCHAR *XmlBase::ReadName(const TCHAR *p, System::String *name)
{
	// Oddly, not supported on some comilers,
	//name->clear();
	// So use this:
	name->Empty();
	assert(p);

	// Names start with letters or underscores.
	// Of course, in unicode, tinyxml has no idea what a letter *is*. The
	// algorithm is generous.
	//
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (p && *p && (IsAlpha(*p) || *p == _T('_')))
	{
		const TCHAR *start = p;
		while (p && *p && (IsAlphaNum(*p) || *p == _T('_') || *p == _T('-') || *p == _T('.') || *p == _T(':')))
		{
			//(*name) += *p; // expensive
			++p;
		}
		if (p - start > 0)
		{
			name->copy(start, p - start);
		}
		return p;
	}
	return 0;
}

//-------------------------------------------------------------------------

const TCHAR *XmlBase::GetEntity(const TCHAR *p, TCHAR *value, int *length)
{
	// Presume an entity, and pull it out.
	String ent;
	int i;
	*length = 0;

	if (*(p + 1) && *(p + 1) == _T('#') && *(p + 2))
	{
		unsigned long ucs = 0;
		ptrdiff_t delta = 0;
		unsigned mult = 1;

		if (*(p + 2) == _T('x'))
		{
			// Hexadecimal.
			if (!*(p + 3))
			{
				return 0;
			}

			const TCHAR *q = p + 3;
			q = _tcschr(q, _T(';'));

			if (!q || !*q)
			{
				return 0;
			}

			delta = q - p;
			--q;

			while (*q != _T('x'))
			{
				if (*q >= _T('0') && *q <= _T('9'))
				{
					ucs += mult *(*q - _T('0'));
				}
				else if (*q >= _T('a') && *q <= _T('f'))
				{
					ucs += mult *(*q - _T('a') + 10);
				}
				else if (*q >= _T('A') && *q <= _T('F'))
				{
					ucs += mult *(*q - _T('A') + 10);
				}
				else
				{
					return 0;
				}
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if (!*(p + 2))
			{
				return 0;
			}

			const TCHAR *q = p + 2;
			q = _tcschr(q, _T(';'));

			if (!q || !*q)
			{
				return 0;
			}

			delta = q - p;
			--q;

			while (*q != _T('#'))
			{
				if (*q >= _T('0') && *q <= _T('9'))
				{
					ucs += mult *(*q - _T('0'));
				}
				else
				{
					return 0;
				}
				mult *= 10;
				--q;
			}
		}
		*value = (TCHAR)ucs;
		*length = 1;
		return p + delta + 1;
	}

	// Now try to match it.
	for (i = 0; i < NUM_ENTITY; ++i)
	{
		if (_tcsncmp(entity[i].str, p, entity[i].strLength) == 0)
		{
			assert(_tcslen(entity[i].str) == entity[i].strLength);
			*value = entity[i].chr;
			*length = 1;
			return (p + entity[i].strLength);
		}
	}

	// So it wasn't an entity, its unrecognized, or something like that.
	*value = *p; // Don't put back the last one, since we return it!
	//*length = 1;	// Leave unrecognized entities - this doesn't really work.
	// Just writes strange XML.
	return p + 1;
}

//-------------------------------------------------------------------------

bool XmlBase::StringEqual(const TCHAR *p, const TCHAR *tag, bool ignoreCase)
{
	assert(p);
	assert(tag);
	if (!p || !*p) {
		assert(0);
		return false;
	} //if

	const TCHAR *q = p;

	if (ignoreCase) {
		while (*q &&  *tag && ToLower(*q) == ToLower(*tag)) {
			++q;
			++tag;
		}

		if (*tag == 0) return true;
	} else {
		while (*q &&  *tag &&  *q == *tag) {
			++q;
			++tag;
		}

		if (*tag == 0) { // Have we found the end of the tag, and everything equal?
			return true;
		} //if
	} //if
	return false;
}

//-------------------------------------------------------------------------

const TCHAR *XmlBase::ReadText(const TCHAR *p, System::String *text, bool trimWhiteSpace, const TCHAR *endTag, bool caseInsensitive)
{
	text->Empty();
	if (!trimWhiteSpace  // certain tags always keep whitespace
		|| !condenseWhiteSpace)
		// if true, whitespace is always kept
	{
		// Keep all the white space.
		while (p &&  *p && !StringEqual(p, endTag, caseInsensitive))
		{
			int len;
			TCHAR cArr[4] = { 0, 0, 0, 0 };
			p = GetChar(p, cArr, &len);
			text->append(cArr, len);
		}
	}
	else
	{
		bool whitespace = false;

		// Remove leading white space:
		p = SkipWhiteSpace(p);
		while (p &&  *p && !StringEqual(p, endTag, caseInsensitive))
		{
			if (*p == _T('\r') || *p == _T('\n'))
			{
				whitespace = true;
				++p;
			}
			else if (IsWhiteSpace(*p))
			{
				whitespace = true;
				++p;
			}
			else
			{
				// If we've found whitespace, add it before the
				// new character. Any whitespace just becomes a space.
				if (whitespace) {
					(*text) += _T(' ');
					whitespace = false;
				} //if

				int len;
				TCHAR cArr[4] = { 0, 0, 0, 0 };
				p = GetChar(p, cArr, &len);
				if (len == 1) {
					(*text) += cArr[0];
				} else {
					text->append(cArr, len); // more efficient
				} //if
			}
		}
	}
	if (p &&  *p)
	{
		p += _tcslen(endTag);
	}
	return (p &&  *p) ? p : 0;
}

//-------------------------------------------------------------------------

const TCHAR *XmlDocument::Parse(const TCHAR *p, XmlParsingData *prevData)
{
	ClearError();

	// Parse away, at the document level. Since a document
	// contains nothing but other tags, most of what happens
	// here is skipping white space.
	if (!p || !*p) {
		SetError(RLIBXML_ERROR_DOCUMENT_EMPTY, 0, 0);
		return 0;
	} //if

	// Note that, for a document, this needs to come
	// before the while space skip, so that parsing
	// starts from the pointer we are given.
	location.Clear();
	if (prevData)
	{
		location.row = prevData->cursor.row;
		location.col = prevData->cursor.col;
	}
	else
	{
		location.row = 0;
		location.col = 0;
	}
	XmlParsingData data(p, TabSize(), location.row, location.col);
	location = data.Cursor();

	p = SkipWhiteSpace(p);
	if (!p)
	{
		SetError(RLIBXML_ERROR_DOCUMENT_EMPTY, 0, 0);
		goto ret;
	}

	while (p &&  *p)
	{
		XmlNode *node = Identify(p);
		if (node)
		{
			p = node->Parse(p, &data);
			LinkEndChild(node);
		}
		else
		{
			break;
		}
		p = SkipWhiteSpace(p);
	}

	// Was this empty?
	if (!firstChild)
	{
		SetError(RLIBXML_ERROR_DOCUMENT_EMPTY, 0, 0);
		goto ret;
	}

	// All is well.
ret: return 0;
}

//-------------------------------------------------------------------------

static const TCHAR *errorString[XmlBase::RLIBXML_ERROR_STRING_COUNT] =
{
	_T("No error"), _T("Error"), _T("Failed to open file"), _T("Failed to read stream"), 
	_T("Error parsing Element."), _T("Failed to read Element name"), _T("Error reading Element value."), 
	_T("Error reading Attributes."), _T("Error: empty tag."), _T("Error reading end tag."), _T("Error parsing Unknown."), 
	_T("Error parsing Comment."), _T("Error parsing Declaration."), _T("Error document empty."), 
	_T("Error null (0) or unexpected EOF found in input stream."), _T("Error parsing CDATA."), 
	_T("Error when XmlDocument added to document, because XmlDocument can only be at the root."),
	_T("Memory not enough")
};

void XmlDocument::SetError(int err, const TCHAR *pError, XmlParsingData *data)
{
	// The first error in a chain is more accurate - don't set again!
	if (errorException.HResult != STATUS_SUCCESS) {
		return;
	} //if

	assert(err > 0 && err < RLIBXML_ERROR_STRING_COUNT);
	RLIB_SetException(errorException, err, errorString[err]);

	errorLocation.Clear();
	if (pError && data) {
		data->Stamp(pError);
		errorLocation = data->Cursor();
	} //if
}

//-------------------------------------------------------------------------


XmlNode *XmlNode::Identify(const TCHAR *p)
{
	XmlNode *returnNode = 0;

	p = SkipWhiteSpace(p);
	if (!p || !*p || *p != _T('<')) {
		return 0;
	} //if

	p = SkipWhiteSpace(p);

	if (!p || !*p) {
		return 0;
	} //if

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const TCHAR *xmlHeader =
	{
		_T("<?xml")
	};
	const TCHAR *commentHeader =
	{
		_T("<!--")
	};
	const TCHAR *dtdHeader =
	{
		_T("<!")
	};
	const TCHAR *cdataHeader =
	{
		_T("<![CDATA[")
	};

	if (StringEqual(p, xmlHeader, true))
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing Declaration\n"));
#endif 
		returnNode = new XmlDeclaration();
	}
	else if (StringEqual(p, commentHeader, false))
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing Comment\n"));
#endif 
		returnNode = new XmlComment();
	}
	else if (StringEqual(p, cdataHeader, false))
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing CDATA\n"));
#endif 
		XmlText *text = new XmlText(_T(""));
		text->SetCDATA(true);
		returnNode = text;
	}
	else if (StringEqual(p, dtdHeader, false))
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing Unknown(1)\n"));
#endif 
		returnNode = new XmlUnknown();
	}
	else if (IsAlpha(*(p + 1)) || *(p + 1) == _T('_'))
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing Element\n"));
#endif 
		returnNode = new XmlElement(_T(""));
	}
	else
	{
#ifdef DEBUG_PARSER
		RLIBXML_LOG(_T("XML parsing Unknown(2)\n"));
#endif 
		returnNode = new XmlUnknown();
	}

	if (returnNode)
	{
		// Set the parent, so it can report errors
		returnNode->parent = this;
	}
	return returnNode;
}

//-------------------------------------------------------------------------

const TCHAR *XmlElement::Parse(const TCHAR *p, XmlParsingData *data)
{
	p = SkipWhiteSpace(p);
	XmlDocument *document = GetDocument();

	if (!p || !*p)
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_PARSING_ELEMENT, 0, 0);
		}
		return 0;
	}

	if (data)
	{
		data->Stamp(p);
		location = data->Cursor();
	}

	if (*p != _T('<'))
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_PARSING_ELEMENT, p, data);
		}
		return 0;
	}

	p = SkipWhiteSpace(p + 1);

	// Read the name.
	const TCHAR *pErr = p;

	p = ReadName(p, &value);
	if (!p || !*p)
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data);
		}
		return 0;
	}

	String endTag(_R("</") + value);

	// Check for and read attributes. Also look for an empty
	// tag or an end tag.
	while (p &&  *p)
	{
		pErr = p;
		p = SkipWhiteSpace(p);
		if (!p || !*p)
		{
			if (document)
			{
				document->SetError(RLIBXML_ERROR_READING_ATTRIBUTES, pErr, data);
			}
			return 0;
		}
		if (*p == _T('/'))
		{
			++p;
			// Empty tag.
			if (*p != _T('>'))
			{
				if (document)
				{
					document->SetError(RLIBXML_ERROR_PARSING_EMPTY, p, data);
				}
				return 0;
			}
			return (p + 1);
		}
		else if (*p == _T('>'))
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue(p, data); // Note this is an Element method, and will set the error if one happens.
			if (!p || !*p)
			{
				// We were looking for the end tag, but found nothing.
				// Fix for [ 1663758 ] Failure to report error on bad XML
				if (document)
				{
					document->SetError(RLIBXML_ERROR_READING_END_TAG, p, data);
				}
				return 0;
			}

			// We should find the end tag now
			// note that:
			// </foo > and
			// </foo> 
			// are both valid end tags.
			if (StringEqual(p, endTag, false))
			{
				p += endTag.Length;
				p = SkipWhiteSpace(p);
				if (p &&  *p &&  *p == _T('>'))
				{
					++p;
					return p;
				}
				if (document)
				{
					document->SetError(RLIBXML_ERROR_READING_END_TAG, p, data);
				}
				return 0;
			}
			else
			{
				if (document)
				{
					document->SetError(RLIBXML_ERROR_READING_END_TAG, p, data);
				}
				return 0;
			}
		}
		else
		{
			// Try to read an attribute:
			XmlAttribute *attrib = new XmlAttribute();
			if (!attrib)
			{
				return 0;
			}

			attrib->SetDocument(document);
			pErr = p;
			p = attrib->Parse(p, data);

			if (!p || !*p)
			{
				if (document)
				{
					document->SetError(RLIBXML_ERROR_PARSING_ELEMENT, pErr, data);
				}
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes:
			XmlAttribute *node = attributeSet.Find(attrib->Name());
			if (node)
			{
				if (document)
				{
					document->SetError(RLIBXML_ERROR_PARSING_ELEMENT, pErr, data);
				}
				delete attrib;
				return 0;
			}

			attributeSet.Add(attrib);
		}
	}
	return p;
}

//-------------------------------------------------------------------------

const TCHAR *XmlElement::ReadValue(const TCHAR *p, XmlParsingData *data)
{
	XmlDocument *document = GetDocument();

	// Read in text and elements in any order.
	const TCHAR *pWithWhiteSpace = p;
	p = SkipWhiteSpace(p);

	while (p &&  *p)
	{
		if (*p != _T('<'))
		{
			// Take what we have, make a text element.
			XmlText *textNode = new XmlText(_T(""));

			if (!textNode)
			{
				return 0;
			}

			if (XmlBase::IsWhiteSpaceCondensed())
			{
				p = textNode->Parse(p, data);
			}
			else
			{
				// Special case: we want to keep the white space
				// so that leading spaces aren't removed.
				p = textNode->Parse(pWithWhiteSpace, data);
			}

			if (!textNode->Blank())
			{
				LinkEndChild(textNode);
			}
			else
			{
				delete textNode;
			}
		}
		else
		{
			// We hit a T('<')
			// Have we hit a new element or an end tag? This could also be
			// a XmlText in the T("CDATA") style.
			if (StringEqual(p, _T("</"), false))
			{
				return p;
			}
			else
			{
				XmlNode *node = Identify(p);
				if (node)
				{
					p = node->Parse(p, data);
					LinkEndChild(node);
				}
				else
				{
					return 0;
				}
			}
		}
		pWithWhiteSpace = p;
		p = SkipWhiteSpace(p);
	}

	if (!p)
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_READING_ELEMENT_VALUE, 0, 0);
		}
	}
	return p;
}

//-------------------------------------------------------------------------

const TCHAR *XmlUnknown::Parse(const TCHAR *p, XmlParsingData *data)
{
	XmlDocument *document = GetDocument();
	p = SkipWhiteSpace(p);

	if (data) {
		data->Stamp(p);
		location = data->Cursor();
	} //if
	if (!p || !*p || *p != _T('<')) {
		if (document) {
			document->SetError(RLIBXML_ERROR_PARSING_UNKNOWN, p, data);
		} //if
		return 0;
	} //if
	++p;
	value.Empty();

	while (p && *p && *p != _T('>'))
	{
		value += *p;
		++p;
	}

	if (!p) {
		if (document) {
			document->SetError(RLIBXML_ERROR_PARSING_UNKNOWN, 0, 0);
		} //if
	} //if

	if (p &&  *p == _T('>')) {
		return p + 1;
	} //if
	return p;
}

//-------------------------------------------------------------------------

const TCHAR *XmlComment::Parse(const TCHAR *p, XmlParsingData *data)
{
	XmlDocument *document = GetDocument();
	value.Empty();

	p = SkipWhiteSpace(p);

	if (data) {
		data->Stamp(p);
		location = data->Cursor();
	} //if
	const TCHAR *startTag = _T("<!--");
	const TCHAR *endTag   = _T("-->");

	if (!StringEqual(p, startTag, false))
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_PARSING_COMMENT, p, data);
		}
		return 0;
	}
	p += _tcslen(startTag);

	// [ 1475201 ] TinyXML parses entities in comments
	// Oops - ReadText doesn't work, because we don't want to parse the entities.
	// p = ReadText( p, &value, false, endTag, false );
	//
	// from the XML spec:
	/*
	[Definition: Comments may appear anywhere in a document outside other markup; in addition,
	they may appear within the document type declaration at places allowed by the grammar.
	They are not part of the document's character data; an XML processor MAY, but need not,
	make it possible for an application to retrieve the text of comments. For compatibility,
	the string T("--") (double-hyphen) MUST NOT occur within comments.] Parameter entity
	references MUST NOT be recognized within comments.

	An example of a comment:

	<!-- declarations for <head> & <body> -->
	*/

	value.Empty();
	// Keep all the white space.
	while (p &&  *p && !StringEqual(p, endTag, false))
	{
		value.append(p, 1);
		++p;
	}
	if (p &&  *p)
	{
		p += _tcslen(endTag);
	}

	return p;
}

//-------------------------------------------------------------------------

const TCHAR *XmlAttribute::Parse(const TCHAR *p, XmlParsingData *data)
{
	p = SkipWhiteSpace(p);
	if (!p || !*p)
	{
		return 0;
	}

	if (data)
	{
		data->Stamp(p);
		location = data->Cursor();
	}
	// Read the name, the _T('=') and the value.
	const TCHAR *pErr = p;
	p = ReadName(p, &name);
	if (!p || !*p)
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_READING_ATTRIBUTES, pErr, data);
		}
		return 0;
	}
	p = SkipWhiteSpace(p);
	if (!p || !*p || *p != _T('='))
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_READING_ATTRIBUTES, p, data);
		}
		return 0;
	}

	++p; // skip _T('=')
	p = SkipWhiteSpace(p);
	if (!p || !*p)
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_READING_ATTRIBUTES, p, data);
		}
		return 0;
	}

	const TCHAR *end;
	const TCHAR SINGLE_QUOTE = _T('\'');
	const TCHAR DOUBLE_QUOTE = _T('\"');

	if (*p == SINGLE_QUOTE)
	{
		++p;
		end = _T("\'"); // single quote in string
		p = ReadText(p, &value, false, end, false);
	}
	else if (*p == DOUBLE_QUOTE)
	{
		++p;
		end = _T("\""); // double quote in string
		p = ReadText(p, &value, false, end, false);
	}
	else
	{
		// All attribute values should be in single or double quotes.
		// But this is such a common error that the parser will try
		// its best, even without them.
		value.Empty();
		while (p &&  *p  // existence
			   && !IsWhiteSpace(*p) // whitespace
			   && *p != _T('/') && *p != _T('>'))
			   // tag end
		{
			if (*p == SINGLE_QUOTE || *p == DOUBLE_QUOTE)
			{
				// [ 1451649 ] Attribute values with trailing quotes not handled correctly
				// We did not have an opening quote but seem to have a 
				// closing one. Give up and throw an error.
				if (document)
				{
					document->SetError(RLIBXML_ERROR_READING_ATTRIBUTES, p, data);
				}
				return 0;
			}
			value += *p;
			++p;
		}
	}
	return p;
}

//-------------------------------------------------------------------------

const TCHAR *XmlText::Parse(const TCHAR *p, XmlParsingData *data)
{
	value.Empty();
	XmlDocument *document = GetDocument();

	if (data)
	{
		data->Stamp(p);
		location = data->Cursor();
	}

	const TCHAR *const startTag = _T("<![CDATA[");
	const TCHAR *const endTag   = _T("]]>");

	if (cdata || StringEqual(p, startTag, false))
	{
		cdata = true;

		if (!StringEqual(p, startTag, false))
		{
			if (document)
			{
				document->SetError(RLIBXML_ERROR_PARSING_CDATA, p, data);
			}
			return 0;
		}
		p += _tcslen(startTag);

		// Keep all the white space, ignore the encoding, etc.
		while (p &&  *p && !StringEqual(p, endTag, false))
		{
			value += *p;
			++p;
		}

		String dummy;
		p = ReadText(p, &dummy, false, endTag, false);
		return p;
	}
	else
	{
		bool ignoreWhite = true;

		const TCHAR *end = _T("<");
		p = ReadText(p, &value, ignoreWhite, end, false);
		if (p &&  *p)
		{
			return p - 1;
		}
		// don't truncate the _T('<')
		return 0;
	}
}

//-------------------------------------------------------------------------

const TCHAR *XmlDeclaration::Parse(const TCHAR *p, XmlParsingData *data)
{
	p = SkipWhiteSpace(p);
	// Find the beginning, find the end, and look for
	// the stuff in-between.
	XmlDocument *document = GetDocument();
	if (!p || !*p || !StringEqual(p, _T("<?xml"), true))
	{
		if (document)
		{
			document->SetError(RLIBXML_ERROR_PARSING_DECLARATION, 0, 0);
		}
		return 0;
	}
	if (data)
	{
		data->Stamp(p);
		location = data->Cursor();
	}
	p += 5;

	version.Empty();
	encoding.Empty();
	standalone.Empty();

	while (p &&  *p)
	{
		if (*p == _T('>')) {
			++p;
			return p;
		} //if

		p = SkipWhiteSpace(p);
		if (StringEqual(p, _T("version"), true))
		{
			XmlAttribute attrib;
			p = attrib.Parse(p, data);
			version.copy(attrib.Value());
		}
		else if (StringEqual(p, _T("encoding"), true))
		{
			XmlAttribute attrib;
			p = attrib.Parse(p, data);
			encoding.copy(attrib.Value());
		}
		else if (StringEqual(p, _T("standalone"), true))
		{
			XmlAttribute attrib;
			p = attrib.Parse(p, data);
			standalone.copy(attrib.Value());
		}
		else
		{
			// Read over whatever it is.
			while (p &&  *p &&  *p != _T('>') && !IsWhiteSpace(*p))
			{
				++p;
			}
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

bool XmlText::Blank()const
{
	for (int i = 0; i < value.Length; i++)
		if (!IsWhiteSpace(value[i]))
		{
			return false;
		}
	return true;
}

#endif // _USE_XML