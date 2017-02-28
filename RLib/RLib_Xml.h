/********************************************************************
	Created:	2012/04/03  8:28
	Filename: 	RLib_Xml.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
	Used:       TinyXML 2.6.2
*********************************************************************/
#include "RLib_String.h"
#include "RLib_Exception.h"

#if !(defined _USE_XML) && !(defined _DISABLE_XML)
#define _USE_XML
#include "support/tinyxml.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 包含用于处理 XML 的类型, 为处理 XML 提供支持
	/// </summary>
	namespace Xml
	{
		class RLIB_API XmlDocument;
		class RLIB_API XmlElement;
		class RLIB_API XmlComment;
		class RLIB_API XmlUnknown;
		class RLIB_API XmlAttribute;
		class RLIB_API XmlText;
		class RLIB_API XmlDeclaration;
		class RLIB_API XmlParsingData;

		/// <summary>
		/// Internal structure for tracking location of items in the XML file
		/// </summary>
		struct RLIB_API XmlCursor
		{
			XmlCursor()  { Clear(); }
			void Clear() { row = col = -1; }

			int row;	// 0 based.
			int col;	// 0 based.
		};

		/// <summary>
		/// Implements the interface to the "Visitor pattern" (see the Accept() method.)
		/// If you call the Accept() method, it requires being passed a XmlVisitor
		/// class to handle callbacks. For nodes that contain other nodes (Document, Element)
		/// you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
		/// are simply called with Visit().
		/// 
		/// If you return T('true') from a Visit method, recursive parsing will continue. If you return
		/// false, <b>no children of this node or its sibilings</b> will be Visited.
		/// 
		/// All flavors of Visit methods have a default implementation that returns T('true') (continue 
		/// visiting). You need to only override methods that are interesting to you.
		/// 
		/// Generally Accept() is called on the XmlDocument, although all nodes suppert Visiting.
		/// 
		/// You should never change the document from a callback.
		/// 
		/// @sa XmlNode::Accept()
		/// </summary>
		class RLIB_API XmlVisitor
		{
		public:
			virtual ~XmlVisitor() {}

			/// Visit a document.
			virtual bool VisitEnter( const XmlDocument& /*doc*/ )			{ return true; }
			/// Visit a document.
			virtual bool VisitExit( const XmlDocument& /*doc*/ )			{ return true; }

			/// Visit an element.
			virtual bool VisitEnter( const XmlElement& /*element*/, const XmlAttribute* /*firstAttribute*/ )	{ return true; }
			/// Visit an element.
			virtual bool VisitExit( const XmlElement& /*element*/ )		{ return true; }

			/// Visit a declaration
			virtual bool Visit( const XmlDeclaration& /*declaration*/ )	{ return true; }
			/// Visit a text node
			virtual bool Visit( const XmlText& /*text*/ )					{ return true; }
			/// Visit a comment node
			virtual bool Visit( const XmlComment& /*comment*/ )			{ return true; }
			/// Visit an unknown node
			virtual bool Visit( const XmlUnknown& /*unknown*/ )			{ return true; }
		};
		/// <summary>
		/// XmlBase is a base class for every class in TinyXml.
		/// It does little except to establish that TinyXml classes
		/// can be printed and provide some utility functions.
		/// 
		/// In XML, the document and elements can contain
		/// other elements and other types of nodes.
		/// 
		/// @verbatim
		/// A Document can contain:	Element	(container or leaf)
		/// Comment (leaf)
		/// Unknown (leaf)
		/// Declaration( leaf )
		/// 
		/// An Element can contain:	Element (container or leaf)
		/// Text	(leaf)
		/// Attributes (not on tree)
		/// Comment (leaf)
		/// Unknown (leaf)
		/// 
		/// A Decleration contains: Attributes (not on tree)
		/// @endverbatim
		/// </summary>
		class RLIB_API XmlBase
		{
			friend class RLIB_API XmlNode;
			friend class RLIB_API XmlElement;
			friend class RLIB_API XmlDocument;

		public:
			XmlBase()	:	userData(0)		{}
			virtual ~XmlBase()			{}

			virtual void Print(IO::Stream* cfile, int depth ) const = 0;


			static void SetCondenseWhiteSpace( bool condense )		{ condenseWhiteSpace = condense; }

			/// Return the current white space setting.
			static bool IsWhiteSpaceCondensed()						{ return condenseWhiteSpace; }


			int Row() const			{ return location.row + 1; }
			int Column() const		{ return location.col + 1; }	///< See Row()

			void  SetUserData( void* user )			{ userData = user; }	///< Set a pointer to arbitrary user data.
			void* GetUserData()						{ return userData; }	///< Get a pointer to arbitrary user data.
			const void* GetUserData() const 		{ return userData; }	///< Get a pointer to arbitrary user data.

			virtual const TCHAR *Parse(	const TCHAR *p, 
				XmlParsingData* data) = 0;


			static void EncodeString( const String& str, String* out );

			enum
			{
				RLIBXML_NO_ERROR = 0,
				RLIBXML_ERROR,
				RLIBXML_ERROR_OPENING_FILE,
				RLIBXML_ERROR_READING_STREAM,
				RLIBXML_ERROR_PARSING_ELEMENT,
				RLIBXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
				RLIBXML_ERROR_READING_ELEMENT_VALUE,
				RLIBXML_ERROR_READING_ATTRIBUTES,
				RLIBXML_ERROR_PARSING_EMPTY,
				RLIBXML_ERROR_READING_END_TAG,
				RLIBXML_ERROR_PARSING_UNKNOWN,
				RLIBXML_ERROR_PARSING_COMMENT,
				RLIBXML_ERROR_PARSING_DECLARATION,
				RLIBXML_ERROR_DOCUMENT_EMPTY,
				RLIBXML_ERROR_EMBEDDED_NULL,
				RLIBXML_ERROR_PARSING_CDATA,
				RLIBXML_ERROR_DOCUMENT_TOP_ONLY,
				RLIBXML_ERROR_MEMORY_NOT_ENOUGH,

				RLIBXML_ERROR_STRING_COUNT
			};
		protected:

			static const TCHAR *SkipWhiteSpace( const TCHAR *);

			RLIB_INLINE static bool IsWhiteSpace(TCHAR c )		
			{ 
				return ( _istspace(static_cast<unsigned int>(c)) || c == _T('\n') || c == _T('\r') );
			}
			RLIB_INLINE static bool IsWhiteSpace( int c )
			{
				if ( c < 256 )
					return IsWhiteSpace( (TCHAR) c );
				return false;	// Again, only truly correct for English/Latin...but usually works.
			}

			static const TCHAR *ReadName( const TCHAR *p, String* name);

			static const TCHAR *ReadText(	const TCHAR *in,				// where to start
				String* text,			// the string read
				bool ignoreWhiteSpace,  // whether to keep the white space
				const TCHAR *endTag,	// what ends this text
				bool ignoreCase			// whether to ignore case in the end tag
				);

			// If an entity has been found, transform it into a character.
			static const TCHAR *GetEntity( const TCHAR *in, TCHAR *value, int* length);

			// Get a character, while interpreting entities.
			// The length can be from 0 to 4 bytes.
			RLIB_INLINE static const TCHAR *GetChar( const TCHAR *p, TCHAR *_value, int* length)
			{
				assert( p );

				*length = 1;

				if ( *length == 1 )
				{
					if ( *p == _T('&') )
						return GetEntity( p, _value, length);
					*_value = *p;
					return p+1;
				}
				else if ( *length )
				{
					//strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
					// and the null terminator isn't needed
					for( int i=0; p[i] && i<*length; ++i ) {
						_value[i] = p[i];
					}
					return p + (*length);
				}
				else
				{
					// Not valid text.
					return 0;
				}
			}

			// Return true if the next characters in the stream are any of the endTag sequences.
			// Ignore case only works for english, and should only be relied on when comparing
			// to English words: StringEqual( p, "version", true ) is fine.
			static bool StringEqual(const TCHAR *p,
									const TCHAR *endTag,
									bool ignoreCase);

			XmlCursor location;

			/// Field containing a generic user pointer
			void*			userData;

			// None of these methods are reliable for any language except English.
			// Good for approximation, not great for accuracy.
			static int IsAlpha(TCHAR anyByte);
			static int IsAlphaNum(TCHAR anyByte);
			RLIB_INLINE static int ToLower(int v)
			{
				return tolower(v);
			}

		private:
			XmlBase( const XmlBase& );				// not implemented.
			void operator=( const XmlBase& base );	// not allowed.

			struct Entity
			{
				const TCHAR *    str;
				unsigned int	strLength;
				TCHAR		    chr;
			};
			enum
			{
				NUM_ENTITY = 5,
				MAX_ENTITY_LENGTH = 6

			};
			static Entity entity[ NUM_ENTITY ];
			static bool condenseWhiteSpace;
		};
		/// <summary>
		/// The parent class for everything in the Document Object Model.
		/// (Except for attributes).
		/// Nodes have siblings, a parent, and children. A node can be
		/// in a document, or stand on its own. The type of a XmlNode
		/// can be queried, and it can be cast to its more defined type.
		/// </summary>
		class RLIB_API XmlNode : public XmlBase
		{
			friend class RLIB_API XmlDocument;
			friend class RLIB_API XmlElement;

		public:
			RLIB_DECLARE_DYNCREATE;

			enum NodeType
			{
				TINYXML_DOCUMENT,
				TINYXML_ELEMENT,
				TINYXML_COMMENT,
				TINYXML_UNKNOWN,
				TINYXML_TEXT,
				TINYXML_DECLARATION,
				TINYXML_TYPECOUNT
			};

			virtual ~XmlNode();

			const String &Value() const { return value; }

			//void SetValue(const TCHAR *_value) { value = const_cast<TCHAR *>(_value);}

			void SetValue(const String &_value) { value = _value; }

			/// Delete all the children of this node. Does not affect _T('this').
			void Clear();

			/// One step up the DOM.
			XmlNode* Parent()							{ return parent; }
			const XmlNode* Parent() const				{ return parent; }

			const XmlNode* FirstChild()	const		{ return firstChild; }	///< The first child of this node. Will be null if there are no children.
			XmlNode* FirstChild()						{ return firstChild; }
			const XmlNode* FirstChild( const TCHAR * value ) const;	 /// The first child of this node with the matching T('value'). Will be null if none found.
			/// The first child of this node with the matching T('value'). Will be null if none found.
			XmlNode* FirstChild( const TCHAR * _value ) {
				// Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
				// call the method, cast the return back to non-const.
				return const_cast< XmlNode* > ((const_cast< const XmlNode* >(this))->FirstChild( _value ));
			}
			const XmlNode* LastChild() const	{ return lastChild; }		/// The last child of this node. Will be null if there are no children.
			XmlNode* LastChild()	{ return lastChild; }

			const XmlNode* LastChild( const TCHAR * value ) const;			/// The last child of this node matching T('value'). Will be null if there are no children.
			XmlNode* LastChild( const TCHAR * _value ) {
				return const_cast< XmlNode* > ((const_cast< const XmlNode* >(this))->LastChild( _value ));
			}

			const XmlNode* IterateChildren( const XmlNode* previous ) const;
			XmlNode* IterateChildren( const XmlNode* previous ) {
				return const_cast< XmlNode* >( (const_cast< const XmlNode* >(this))->IterateChildren( previous ) );
			}

			/// This flavor of IterateChildren searches for children with a particular T('value')
			const XmlNode* IterateChildren( const TCHAR * value, const XmlNode* previous ) const;
			XmlNode* IterateChildren( const TCHAR * _value, const XmlNode* previous ) {
				return const_cast< XmlNode* >( (const_cast< const XmlNode* >(this))->IterateChildren( _value, previous ) );
			}

			XmlNode* InsertEndChild( const XmlNode& addThis );

			XmlNode* LinkEndChild( XmlNode* addThis );

			XmlNode* InsertBeforeChild( XmlNode* beforeThis, const XmlNode& addThis );

			XmlNode* InsertAfterChild(  XmlNode* afterThis, const XmlNode& addThis );


			XmlNode* ReplaceChild( XmlNode* replaceThis, const XmlNode& withThis );

			/// Delete a child of this node.
			bool RemoveChild( XmlNode* removeThis );

			/// Navigate to a sibling node.
			const XmlNode* PreviousSibling() const			{ return prev; }
			XmlNode* PreviousSibling()						{ return prev; }

			/// Navigate to a sibling node.
			const XmlNode* PreviousSibling( const TCHAR * ) const;
			XmlNode* PreviousSibling( const TCHAR *_prev ) {
				return const_cast< XmlNode* >( (const_cast< const XmlNode* >(this))->PreviousSibling( _prev ) );
			}

			/// Navigate to a sibling node.
			const XmlNode* NextSibling() const				{ return next; }
			XmlNode* NextSibling()							{ return next; }

			/// Navigate to a sibling node with the given T('value').
			const XmlNode* NextSibling( const TCHAR * ) const;
			XmlNode* NextSibling( const TCHAR *_next ) {
				return const_cast< XmlNode* >( (const_cast< const XmlNode* >(this))->NextSibling( _next ) );
			}


			const XmlElement* NextSiblingElement() const;
			XmlElement* NextSiblingElement() {
				return const_cast< XmlElement* >( (const_cast< const XmlNode* >(this))->NextSiblingElement() );
			}


			const XmlElement* NextSiblingElement( const TCHAR * ) const;
			XmlElement* NextSiblingElement( const TCHAR *_next ) {
				return const_cast< XmlElement* >( (const_cast< const XmlNode* >(this))->NextSiblingElement( _next ) );
			}

			/// Convenience function to get through elements.
			const XmlElement* FirstChildElement()	const;
			XmlElement* FirstChildElement() {
				return const_cast< XmlElement* >( (const_cast< const XmlNode* >(this))->FirstChildElement() );
			}

			/// Convenience function to get through elements.
			const XmlElement* FirstChildElement( const TCHAR * _value ) const;
			XmlElement* FirstChildElement( const TCHAR * _value ) {
				return const_cast< XmlElement* >( (const_cast< const XmlNode* >(this))->FirstChildElement( _value ) );
			}

			int Type() const	{ return type; }

			const XmlDocument* GetDocument() const;
			XmlDocument* GetDocument() {
				return const_cast< XmlDocument* >( (const_cast< const XmlNode* >(this))->GetDocument() );
			}

			/// Returns true if this node has no children.
			bool NoChildren() const						{ return !firstChild; }

			virtual const XmlDocument*    ToDocument()    const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual const XmlElement*     ToElement()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual const XmlComment*     ToComment()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual const XmlUnknown*     ToUnknown()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual const XmlText*        ToText()        const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual const XmlDeclaration* ToDeclaration() const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

			virtual XmlDocument*          ToDocument()    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual XmlElement*           ToElement()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual XmlComment*           ToComment()     { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual XmlUnknown*           ToUnknown()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual XmlText*	            ToText()        { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
			virtual XmlDeclaration*       ToDeclaration() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.


			virtual XmlNode* Clone() const = 0;


			virtual bool Accept( XmlVisitor* visitor ) const = 0;

		protected:
			XmlNode( NodeType _type );

			// Copy to the allocated object. Shared functionality between Clone, Copy constructor,
			// and the assignment operator.
			void CopyTo( XmlNode* target ) const;

			// Figure out what is at *p, and parse it. Returns null if it is not an xml node.
			XmlNode* Identify( const TCHAR *start);

			XmlNode*		parent;
			NodeType		type;

			XmlNode*		firstChild;
			XmlNode*		lastChild;

			String	value;

			XmlNode*		prev;
			XmlNode*		next;

		private:
			XmlNode( const XmlNode& );				// not implemented.
			void operator=( const XmlNode& base );	// not allowed.
		};
		/// <summary>
		/// An attribute is a name-value pair. Elements have an arbitrary
		/// number of attributes, each with a unique name.
		/// 
		/// @note The attributes are not XmlNodes, since they are not
		/// part of the tinyXML document object model. There are other
		/// suggested ways to look at this problem.
		/// </summary>
		class RLIB_API XmlAttribute : public XmlBase
		{
			friend class RLIB_API XmlAttributeSet;

		public:
			RLIB_DECLARE_DYNCREATE;

			/// Construct an empty attribute.
			XmlAttribute() : XmlBase()
			{
				document = 0;
				prev = next = 0;
			}

			/// Construct an attribute with a name and value.
			XmlAttribute( const TCHAR * _name, const TCHAR * _value )
			{
				name = _name;
				value = _value;
				document = 0;
				prev = next = 0;
			}

			const String &Name()  const { return name; } /// Return the name of this attribute.
			const String &Value() const { return value; } /// Return the value of this attribute.

			int				IntValue() const;							 /// Return the value of this attribute, converted to an integer.
			double			DoubleValue() const;						 /// Return the value of this attribute, converted to a double.

			int QueryIntValue( int* _value ) const;
			/// QueryDoubleValue examines the value string. See QueryIntValue().
			int QueryDoubleValue( double* _value ) const;

			void SetName( const String &_name )	{ name = _name; }		 /// Set the name of this attribute.
			void SetValue( const String &_value )	{ value = _value; }		 /// Set the value.

			void SetIntValue( int _value );								 /// Set the value from an integer.
			void SetDoubleValue( double _value );						 /// Set the value from a double.

			/// Get the next sibling attribute in the DOM. Returns null at end.
			const XmlAttribute* Next() const;
			XmlAttribute* Next() {
				return const_cast< XmlAttribute* >( (const_cast< const XmlAttribute* >(this))->Next() ); 
			}

			/// Get the previous sibling attribute in the DOM. Returns null at beginning.
			const XmlAttribute* Previous() const;
			XmlAttribute* Previous() {
				return const_cast< XmlAttribute* >( (const_cast< const XmlAttribute* >(this))->Previous() ); 
			}

			bool operator==( const XmlAttribute& rhs ) const { return rhs.name == name; }
			bool operator<( const XmlAttribute& rhs )	 const { return name < rhs.name; }
			bool operator>( const XmlAttribute& rhs )  const { return name > rhs.name; }


			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			// Prints this Attribute to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth ) const {
				Print( cfile, depth, 0 );
			}
			void Print( IO::Stream* cfile, int depth, String* str OUT ) const;

			// [internal use]
			// Set the document pointer so the attribute can report errors.
			void SetDocument( XmlDocument* doc )	{ document = doc; }

		private:
			XmlAttribute( const XmlAttribute& );				// not implemented.
			void operator=( const XmlAttribute& base );	// not allowed.

			XmlDocument*	document;	// A pointer back to a document, for error reporting.
			String name;
			String value;
			XmlAttribute*	prev;
			XmlAttribute*	next;
		};
		/// <summary>
		/// A class used to manage a group of attributes.
		/// It is only used internally, both by the ELEMENT and the DECLARATION.
		/// The set can be changed transparent to the Element and Declaration
		/// classes that use it, but NOT transparent to the Attribute
		/// which has to implement a next() and previous() method. Which makes
		/// it a bit problematic and prevents the use of STL.
		/// This version is implemented with circular lists because:
		/// - I like circular lists
		/// - it demonstrates some independence from the (typical) doubly linked list.
		/// </summary>
		class RLIB_API XmlAttributeSet
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			XmlAttributeSet();
			~XmlAttributeSet();

			void Add( XmlAttribute* attribute );
			void Remove( XmlAttribute* attribute );

			const XmlAttribute* First()	const	{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
			XmlAttribute* First()					{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
			const XmlAttribute* Last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
			XmlAttribute* Last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

			XmlAttribute*	Find( const TCHAR *_name ) const;
			XmlAttribute* FindOrCreate( const TCHAR *_name );

		private:
			//Because of hidden/disabled copy-construktor in XmlAttribute (sentinel-element),
			//this class export must be also use a hidden/disabled copy-constructor !!!
			XmlAttributeSet( const XmlAttributeSet& );	// not allowed
			void operator=( const XmlAttributeSet& );	// not allowed (as XmlAttribute)

			XmlAttribute sentinel;
		};
		/// <summary>
		/// The element is a container class. It has a value, the element name,
		/// and can contain other elements, text, comments, and unknowns.
		/// Elements also contain an arbitrary number of attributes.
		/// </summary>
		class RLIB_API XmlElement : public XmlNode
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			/// Construct an element.
			XmlElement (const TCHAR * in_value);

			XmlElement( const XmlElement& );

			XmlElement& operator=( const XmlElement& base );

			virtual ~XmlElement();

			const String Attribute( const TCHAR *name ) const;
			const String Attribute( const TCHAR *name, int* i ) const;
			const String Attribute( const TCHAR *name, double* d ) const;


			int QueryIntAttribute( const TCHAR *name, int* _value ) const;
			/// QueryUnsignedAttribute examines the attribute - see QueryIntAttribute().
			int QueryUnsignedAttribute( const TCHAR *name, unsigned* _value ) const;

			int QueryBoolAttribute( const TCHAR *name, bool* _value ) const;
			/// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
			int QueryDoubleAttribute( const TCHAR *name, double* _value ) const;
			/// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
			int QueryFloatAttribute( const TCHAR *name, float* _value ) const {
				double d;
				int result = QueryDoubleAttribute( name, &d );
				if ( result == RLIBXML_SUCCESS ) {
					*_value = (float)d;
				}
				return result;
			}

			void SetAttribute( const TCHAR *name, const TCHAR * _value );

			void SetAttribute( const TCHAR * name, int value );


			void SetDoubleAttribute( const TCHAR * name, double value );


			void RemoveAttribute( const TCHAR * name );

			const XmlAttribute* FirstAttribute() const	{ return attributeSet.First(); } /// Access the first attribute in this element.
			XmlAttribute* FirstAttribute() 				{ return attributeSet.First(); }
			const XmlAttribute* LastAttribute()	const 	{ return attributeSet.Last(); } /// Access the last attribute in this element.
			XmlAttribute* LastAttribute()					{ return attributeSet.Last(); }


			const String GetText() const;

			/// Creates a new Element and returns it - the returned element is a copy.
			virtual XmlNode* Clone() const;
			// Print the Element to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth ) const;


			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			virtual const XmlElement*     ToElement()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual XmlElement*           ToElement()	          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* visitor ) const;

		protected:

			void CopyTo( XmlElement* target ) const;
			void ClearThis();	// like clear, but initializes T('this') object as well

			// Used to be public [internal use]

			const TCHAR *ReadValue( const TCHAR *in, XmlParsingData* prevData);

		private:
			XmlAttributeSet attributeSet;
		};
		/// <summary>
		/// 表示XML注释
		/// </summary>
		class RLIB_API XmlComment : public XmlNode
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			/// Constructs an empty comment.
			XmlComment() : XmlNode( XmlNode::TINYXML_COMMENT ) {}
			/// Construct a comment from text.
			XmlComment( const String &_value ) : XmlNode( XmlNode::TINYXML_COMMENT ) {
				SetValue( _value );
			}
			XmlComment( const XmlComment& );
			XmlComment& operator=( const XmlComment& base );

			virtual ~XmlComment()	{}

			/// Returns a copy of this Comment.
			virtual XmlNode* Clone() const;
			// Write this Comment to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth ) const;


			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			virtual const XmlComment*  ToComment() const	{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual		  XmlComment*  ToComment()		{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* visitor ) const;

		protected:
			void CopyTo( XmlComment* target ) const;
		private:

		};
		/// <summary>
		/// XML text. A text node can have 2 ways to output the next. "normal" output 
		/// and CDATA. It will default to the mode it was parsed from the XML file and
		/// you generally want to leave it alone, but you can change the output mode with 
		/// SetCDATA() and query it with CDATA().
		/// </summary>
		class RLIB_API XmlText : public XmlNode
		{
			friend class RLIB_API XmlElement;
		public:
			RLIB_DECLARE_DYNCREATE;

			XmlText (const String &initValue ) : XmlNode (XmlNode::TINYXML_TEXT)
			{
				SetValue( initValue );
				cdata = false;
			}
			virtual ~XmlText() {}

			XmlText( const XmlText& copy ) : XmlNode( XmlNode::TINYXML_TEXT )	{ copy.CopyTo( this ); }
			XmlText& operator=( const XmlText& base )							 	{ base.CopyTo( this ); return *this; }

			// Write this text object to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth ) const;

			/// Queries whether this represents text using a CDATA section.
			bool CDATA() const				{ return cdata; }
			/// Turns on or off a CDATA representation of text.
			void SetCDATA( bool _cdata )	{ cdata = _cdata; }

			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			virtual const XmlText* ToText() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual XmlText*       ToText()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* content ) const;

		protected :
			///  [internal use] Creates a new Element and returns it.
			virtual XmlNode* Clone() const;
			void CopyTo( XmlText* target ) const;

			bool Blank() const;	// returns true if all white space and new lines
			// [internal use]
		private:
			bool cdata;			// true if this should be input and output as a CDATA style text element
		};

		/// <summary>
		/// In correct XML the declaration is the first entry in the file.
		/// @verbatim
		/// <?xml version="1.0" standalone="yes"?>
		/// @endverbatim
		/// 
		/// TinyXml will happily read or write files without a declaration,
		/// however. There are 3 possible attributes to the declaration:
		/// version, encoding, and standalone.
		/// 
		/// Note: In this version of the code, the attributes are
		/// handled as special cases, not generic attributes, simply
		/// because there can only be at most 3 and they are always the same.
		/// </summary>
		class RLIB_API XmlDeclaration : public XmlNode
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			/// Construct an empty declaration.
			XmlDeclaration() : XmlNode(XmlNode::TINYXML_DECLARATION) {}

			/// Construct.
			XmlDeclaration(const String &_version,
						   const String &_encoding = Nothing,
						   const String &_standalone = Nothing);

			XmlDeclaration( const XmlDeclaration& copy );
			XmlDeclaration& operator=( const XmlDeclaration& copy );

			virtual ~XmlDeclaration()	{}

			/// Creates a copy of this Declaration and returns it.
			virtual XmlNode* Clone() const;
			// Print this declaration to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth, String* str ) const;
			virtual void Print( IO::Stream* cfile, int depth ) const {
				Print( cfile, depth, 0 );
			}

			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			virtual const XmlDeclaration* ToDeclaration() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual XmlDeclaration*       ToDeclaration()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* visitor ) const;

		protected:
			void CopyTo( XmlDeclaration* target ) const;
			// used to be public

		public:
			/// Version. Will return an empty string if none was found.
			String version;
			/// Encoding. Will return an empty string if none was found.
			String encoding;
			/// Is this a standalone document?
			String standalone;
		};
		/// <summary>
		/// Any tag that tinyXml doesn't recognize is saved as an
		/// unknown. It is a tag of text, but should not be modified.
		/// It will be written back to the XML, unchanged, when the file
		/// is saved.
		/// 
		/// DTD tags get thrown into XmlUnknowns
		/// </summary>
		class RLIB_API XmlUnknown : public XmlNode
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			XmlUnknown() : XmlNode( XmlNode::TINYXML_UNKNOWN )	{}
			virtual ~XmlUnknown() {}

			XmlUnknown( const XmlUnknown& copy ) : XmlNode( XmlNode::TINYXML_UNKNOWN )		{ copy.CopyTo( this ); }
			XmlUnknown& operator=( const XmlUnknown& copy )										{ copy.CopyTo( this ); return *this; }

			/// Creates a copy of this Unknown and returns it.
			virtual XmlNode* Clone() const;
			// Print this Unknown to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth ) const;

			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data);

			virtual const XmlUnknown*     ToUnknown()     const	{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual XmlUnknown*           ToUnknown()				{ return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* content ) const;

		protected:
			void CopyTo( XmlUnknown* target ) const;

		private:

		};

		/// <summary>
		/// 表示XML异常
		/// </summary>
		RLIB_INTERNAL_EXCEPTION(XmlException, Exception);

		/// <summary>
		/// Always the top level node. A document binds together all the
		/// XML pieces. It can be saved, loaded, and printed to the screen.
		/// The T('value') of a document node is the xml file name.
		/// </summary>
		class RLIB_API XmlDocument : public XmlNode
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			/// Create an empty document, that has no name.
			XmlDocument();
			/// Create a document with a name. The name of the document is also the filename of the xml.
			XmlDocument(const String &documentName);

			XmlDocument( const XmlDocument& copy );
			XmlDocument& operator=( const XmlDocument& copy );

			virtual ~XmlDocument() {}

			/// Load a file using the current document value. Returns true if successful.
			bool LoadFile();
			/// Load a file using the given filename. Returns true if successful.
			bool LoadFile(const String &filename);
			/// Load from stream. Returns true if successful.
			bool LoadFromStream(System::IO::Stream *, Text::Encoding codepage = Text::UnknownEncoding);

			/// Save a file using the current document value. Returns true if successful.
			bool SaveFile(Text::Encoding codepage = Text::UnknownEncoding) const;
			/// Save a file using the given filename. Returns true if successful.
			bool SaveFile(const String &filename, Text::Encoding codepage = Text::UnknownEncoding) const;
			/// Save a file using the given IO::Stream *. Returns true if successful.
			bool SaveFile( System::IO::Stream *, Text::Encoding codepage = Text::UnknownEncoding ) const;

			virtual const TCHAR *Parse( const TCHAR *p, XmlParsingData* data = 0);


			const XmlElement* RootElement() const		{ return FirstChildElement(); }
			XmlElement* RootElement()					{ return FirstChildElement(); }

			/// <summary>
			/// 获取XmlDocument发生的异常信息
			/// </summary>
			XmlException *GetLastException() { return &this->errorException; } ; 


			int ErrorRow() const	{ return errorLocation.row+1; }
			int ErrorCol() const	{ return errorLocation.col+1; }	///< The column where the error occured. See ErrorRow()


			void SetTabSize( int _tabsize )		{ tabsize = _tabsize; }

			int TabSize() const	{ return tabsize; }


			void ClearError()						
			{	
				errorException.HResult = STATUS_SUCCESS;
				errorLocation.row      = errorLocation.col = 0; //errorLocation.last = 0; 
			}

			/// Print this Document to a IO::Stream stream.
			virtual void Print( IO::Stream* cfile, int depth = 0 ) const;
			// [internal use]
			void SetError( int err, const TCHAR *errorLocation, XmlParsingData* prevData);

			virtual const XmlDocument*    ToDocument()    const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
			virtual XmlDocument*          ToDocument()          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.


			virtual bool Accept( XmlVisitor* content ) const;

		protected :
			// [internal use]
			virtual XmlNode* Clone() const;

		private:
			void CopyTo( XmlDocument* target ) const;

			int tabsize;

			XmlCursor    errorLocation;
			mutable XmlException errorException;
		};

		class RLIB_API XmlHandle
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			/// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
			XmlHandle( XmlNode* _node )					{ this->node = _node; }
			/// Copy constructor
			XmlHandle( const XmlHandle& ref )			{ this->node = ref.node; }
			XmlHandle operator=( const XmlHandle& ref ) { if ( &ref != this ) this->node = ref.node; return *this; }

			/// Return a handle to the first child node.
			XmlHandle FirstChild() const;
			/// Return a handle to the first child node with the given name.
			XmlHandle FirstChild( const TCHAR * value ) const;
			/// Return a handle to the first child element.
			XmlHandle FirstChildElement() const;
			/// Return a handle to the first child element with the given name.
			XmlHandle FirstChildElement( const TCHAR * value ) const;


			XmlHandle Child( const TCHAR *value, int index ) const;

			XmlHandle Child( int index ) const;

			XmlHandle ChildElement( const TCHAR *value, int index ) const;

			XmlHandle ChildElement( int index ) const;

			XmlNode* ToNode() const			{ return node; } 

			XmlElement* ToElement() const		{ return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }

			XmlText* ToText() const			{ return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }

			XmlUnknown* ToUnknown() const		{ return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }


			XmlNode* Node() const			{ return ToNode(); } 

			XmlElement* Element() const	{ return ToElement(); }

			XmlText* Text() const			{ return ToText(); }

			XmlUnknown* Unknown() const	{ return ToUnknown(); }

		private:
			XmlNode* node;
		};

		class RLIB_API XmlPrinter : public XmlVisitor
		{
		public:
			RLIB_DECLARE_DYNCREATE;

			XmlPrinter()
				: depth(0), simpleTextPrint(false), buffer(), indent(_T("    ")), lineBreak(_T("\n")) {}

			virtual bool VisitEnter( const XmlDocument& doc );
			virtual bool VisitExit( const XmlDocument& doc );

			virtual bool VisitEnter( const XmlElement& element, const XmlAttribute* firstAttribute );
			virtual bool VisitExit( const XmlElement& element );

			virtual bool Visit( const XmlDeclaration& declaration );
			virtual bool Visit( const XmlText& text );
			virtual bool Visit( const XmlComment& comment );
			virtual bool Visit( const XmlUnknown& unknown );


			void SetIndent(const String &_indent) {
				indent = _indent;
			}
			/// Query the indention string.
			const String &Indent() {
				return indent;
			}

			void SetLineBreak( const String &_lineBreak )		
			{ 
				lineBreak = _lineBreak;
			}
			/// Query the current line breaking string.
			const String &LineBreak() {
				return lineBreak;
			}

			void SetStreamPrinting() {
				indent.Empty();
				lineBreak.Empty();
			}
			/// Return the result.
			const String &CStr() {
				return buffer;
			}
			/// Return the length of the result string.
			intptr_t Size() { 
				return buffer.Length;
			}

		private:
			void DoIndent()	{
				for( int i=0; i<depth; ++i )
					buffer += indent;
			}
			void DoLineBreak() {
				buffer += lineBreak;
			}

			int depth;
			bool simpleTextPrint;
			String buffer;
			String indent;
			String lineBreak;
		};
	}
}
#endif //_USE_XML