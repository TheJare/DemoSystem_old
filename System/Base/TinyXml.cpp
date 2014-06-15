//----------------------------------------------------------------------------
//  Nombre:    TinyXml.cpp
//
//  Contenido: Simple Xml Reader / Writer
//----------------------------------------------------------------------------

#include "Base.h"

/*
  Modified TinyXml
  Changes include formatting, refactoring, optimization
  and overall clarity IMHO.

  Original taken from:

  www.sourceforge.net/projects/tinyxml
  Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

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

#include "tinyxml.h"
#include <ctype.h>

namespace
{
  const char* XMLHEADER = "<?xml";
  const char* COMMENTHEADER = "<!--";
  const char* COMMENTEND = "-->";

  struct Entity
  {
    const char*   str;
    unsigned int  strLength;
    char          chr;
  };
  enum
  {
    NUM_ENTITY = 5,
    MAX_ENTITY_LENGTH = 6
  };
  Entity entity[ NUM_ENTITY ] = 
  {
    { "&amp;",  5, '&' },
    { "&lt;",   4, '<' },
    { "&gt;",   4, '>' },
    { "&quot;", 6, '\"' },
    { "&apos;", 6, '\'' }
  };

  inline bool  IsWhiteSpace( int c )   { return ( isspace( c ) || c == '\n' || c == '\r' ); }

  const char* SkipWhiteSpace( const char* p )
  {
    if ( !p || !*p )
      return 0;
    while ( p && *p )
    {
      if ( IsWhiteSpace( *p ) )   // Still using old rules for white space.
        ++p;
      else
        break;
    }

    return p;
  }

}

// -----------------------------------------------------------------------------------
// TiXmlBase
// -----------------------------------------------------------------------------------
const char* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
  "No error",
  "Error",
  "Failed to open file",
  "Memory allocation failed.",
  "Error parsing Element.",
  "Failed to read Element name",
  "Error reading Element value.",
  "Error reading Attributes.",
  "Error: empty tag.",
  "Error reading end tag.",
  "Error parsing Unknown.",
  "Error parsing Comment.",
  "Error parsing Declaration.",
  "Error document empty."
};

bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::PutString( const TIXML_STRING& str, TIXML_STRING* outString )
{
  int i=0;

  while( i<(int)str.length() )
  {
    int c = str[i];

    if (    c == '&' 
         && i < ( (int)str.length() - 2 )
       && str[i+1] == '#'
       && str[i+2] == 'x' )
    {
      // Hexadecimal character reference.
      // Pass through unchanged.
      // &#xA9; -- copyright symbol, for example.
      while ( i<(int)str.length() )
      {
        outString->append( str.c_str() + i, 1 );
        ++i;
        if ( str[i] == ';' )
          break;
      }
    }
    else
    {
      // predefined entities
      bool bFound = false;
      for (int j = 0; j < NUM_ENTITY && !bFound; j++)
        if ( c == entity[j].chr )
        {
          outString->append( entity[j].str, entity[j].strLength );
          ++i;
          bFound = true;
        }
      if (!bFound)
      {
        if ( c < 32 || c > 126 )
        {
          // Easy pass at non-alpha/numeric/symbol
          // 127 is the delete key. Below 32 is symbolic.
          char buf[ 32 ];
          sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
          outString->append( buf, strlen( buf ) );
          ++i;
        }
        else
        {
          char realc = (char) c;
          outString->append( &realc, 1 );
          ++i;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------------
// TiXmlNode
// -----------------------------------------------------------------------------------

TiXmlNode::TiXmlNode( NodeType _type )
{
  parent = 0;
  type = _type;
  firstChild = 0;
  lastChild = 0;
  prev = 0;
  next = 0;
  userData = 0;
}


TiXmlNode::~TiXmlNode()
{
  TiXmlNode* node = firstChild;
  TiXmlNode* temp = 0;

  while ( node )
  {
    temp = node;
    node = node->next;
    delete temp;
  } 
}


void TiXmlNode::Clear()
{
  TiXmlNode* node = firstChild;
  TiXmlNode* temp = 0;

  while ( node )
  {
    temp = node;
    node = node->next;
    delete temp;
  } 

  firstChild = 0;
  lastChild = 0;
}


TiXmlNode* TiXmlNode::LinkEndChild( TiXmlNode* node )
{
  node->parent = this;

  node->prev = lastChild;
  node->next = 0;

  if ( lastChild )
    lastChild->next = node;
  else
    firstChild = node;      // it was an empty list.

  lastChild = node;
  return node;
}


TiXmlNode* TiXmlNode::InsertEndChild( const TiXmlNode& addThis )
{
  TiXmlNode* node = addThis.Clone();
  if ( !node )
    return 0;

  return LinkEndChild( node );
}


TiXmlNode* TiXmlNode::InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis )
{ 
  if ( !beforeThis || beforeThis->parent != this )
    return 0;

  TiXmlNode* node = addThis.Clone();
  if ( !node )
    return 0;
  node->parent = this;

  node->next = beforeThis;
  node->prev = beforeThis->prev;
  if ( beforeThis->prev )
  {
    beforeThis->prev->next = node;
  }
  else
  {
//    assert( firstChild == beforeThis );
    firstChild = node;
  }
  beforeThis->prev = node;
  return node;
}


TiXmlNode* TiXmlNode::InsertAfterChild( TiXmlNode* afterThis, const TiXmlNode& addThis )
{
  if ( !afterThis || afterThis->parent != this )
    return 0;

  TiXmlNode* node = addThis.Clone();
  if ( !node )
    return 0;
  node->parent = this;

  node->prev = afterThis;
  node->next = afterThis->next;
  if ( afterThis->next )
  {
    afterThis->next->prev = node;
  }
  else
  {
//    assert( lastChild == afterThis );
    lastChild = node;
  }
  afterThis->next = node;
  return node;
}


TiXmlNode* TiXmlNode::ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis )
{
  if ( replaceThis->parent != this )
    return 0;

  TiXmlNode* node = withThis.Clone();
  if ( !node )
    return 0;

  node->next = replaceThis->next;
  node->prev = replaceThis->prev;

  if ( replaceThis->next )
    replaceThis->next->prev = node;
  else
    lastChild = node;

  if ( replaceThis->prev )
    replaceThis->prev->next = node;
  else
    firstChild = node;

  delete replaceThis;
  node->parent = this;
  return node;
}


bool TiXmlNode::RemoveChild( TiXmlNode* removeThis )
{
  if ( removeThis->parent != this )
  { 
//    assert( 0 );
    return false;
  }

  if ( removeThis->next )
    removeThis->next->prev = removeThis->prev;
  else
    lastChild = removeThis->prev;

  if ( removeThis->prev )
    removeThis->prev->next = removeThis->next;
  else
    firstChild = removeThis->next;

  delete removeThis;
  return true;
}

TiXmlNode* TiXmlNode::FirstChild( const char * value ) const
{
  TiXmlNode* node;
  for ( node = firstChild; node; node = node->next )
  {
    if ( node->SValue() == TIXML_STRING( value ))
      return node;
  }
  return 0;
}

TiXmlNode* TiXmlNode::LastChild( const char * value ) const
{
  TiXmlNode* node;
  for ( node = lastChild; node; node = node->prev )
  {
    if ( node->SValue() == TIXML_STRING (value))
      return node;
  }
  return 0;
}

TiXmlNode* TiXmlNode::IterateChildren( TiXmlNode* previous ) const
{
  if ( !previous )
  {
    return FirstChild();
  }
  else
  {
//    assert( previous->parent == this );
    return previous->NextSibling();
  }
}

TiXmlNode* TiXmlNode::IterateChildren( const char * val, TiXmlNode* previous ) const
{
  if ( !previous )
  {
    return FirstChild( val );
  }
  else
  {
//    assert( previous->parent == this );
    return previous->NextSibling( val );
  }
}

TiXmlNode* TiXmlNode::NextSibling( const char * value ) const
{
  TiXmlNode* node;
  for ( node = next; node; node = node->next )
  {
    if ( node->SValue() == TIXML_STRING (value))
      return node;
  }
  return 0;
}


TiXmlNode* TiXmlNode::PreviousSibling( const char * value ) const
{
  TiXmlNode* node;
  for ( node = prev; node; node = node->prev )
  {
    if ( node->SValue() == TIXML_STRING (value))
      return node;
  }
  return 0;
}

void TiXmlElement::RemoveAttribute( const char * name )
{
  TiXmlAttribute* node = attributeSet.Find( name );
  if ( node )
  {
    attributeSet.Remove( node );
    delete node;
  }
}

TiXmlElement* TiXmlNode::FirstChildElement() const
{
  TiXmlNode* node;

  for ( node = FirstChild();
  node;
  node = node->NextSibling() )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}

TiXmlElement* TiXmlNode::FirstChildElement( const char * value ) const
{
  TiXmlNode* node;

  for ( node = FirstChild( value );
  node;
  node = node->NextSibling( value ) )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}


TiXmlElement* TiXmlNode::NextSiblingElement() const
{
  TiXmlNode* node;

  for ( node = NextSibling();
  node;
  node = node->NextSibling() )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}

TiXmlElement* TiXmlNode::NextSiblingElement( const char * value ) const
{
  TiXmlNode* node;

  for ( node = NextSibling( value );
  node;
  node = node->NextSibling( value ) )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}



TiXmlDocument* TiXmlNode::GetDocument() const
{
  const TiXmlNode* node;

  for( node = this; node; node = node->parent )
  {
    if ( node->ToDocument() )
      return node->ToDocument();
  }
  return 0;
}

// -----------------------------------------------------------------------------------
// TiXmlElement
// -----------------------------------------------------------------------------------

TiXmlElement::TiXmlElement (const char * _value)
: TiXmlNode( TiXmlNode::ELEMENT )
{
  firstChild = lastChild = 0;
  value = _value;
}

TiXmlElement::~TiXmlElement()
{
  while( attributeSet.First() )
  {
    TiXmlAttribute* node = attributeSet.First();
    attributeSet.Remove( node );
    delete node;
  }
}

const char * TiXmlElement::Attribute( const char * name ) const
{
  TiXmlAttribute* node = attributeSet.Find( name );

  if ( node )
    return node->Value();

  return 0;
}


const char * TiXmlElement::Attribute( const char * name, int* i ) const
{
  const char * s = Attribute( name );
  if ( i )
  {
    if ( s )
      *i = atoi( s );
    else
      *i = 0;
  }
  return s;
}


void TiXmlElement::SetAttribute( const char * name, int val )
{ 
  char buf[64];
  sprintf( buf, "%d", val );
  SetAttribute( name, buf );
}


void TiXmlElement::SetAttribute( const char * name, const char * value )
{
  TiXmlAttribute* node = attributeSet.Find( name );
  if ( node )
  {
    node->SetValue( value );
    return;
  }

  TiXmlAttribute* attrib = new TiXmlAttribute( name, value );
  if ( attrib )
  {
    attributeSet.Add( attrib );
  }
  else
  {
    TiXmlDocument* document = GetDocument();
    if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
  }
}

void TiXmlElement::Print( FILE* cfile, int depth ) const
{
  int i;
  for ( i=0; i<depth; i++ )
  {
    fprintf( cfile, "    " );
  }

  fprintf( cfile, "<%s", value.c_str() );

  TiXmlAttribute* attrib;
  for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
  {
    fprintf( cfile, " " );
    attrib->Print( cfile, depth );
  }

  // There are 3 different formatting approaches:
  // 1) An element without children is printed as a <foo /> node
  // 2) An element with only a text child is printed as <foo> text </foo>
  // 3) An element with children is printed on multiple lines.
  TiXmlNode* node;
  if ( !firstChild )
  {
    fprintf( cfile, " />" );
  }
  else if ( firstChild == lastChild && firstChild->ToText() )
  {
    fprintf( cfile, ">" );
    firstChild->Print( cfile, depth + 1 );
    fprintf( cfile, "</%s>", value.c_str() );
  }
  else
  {
    fprintf( cfile, ">" );

    for ( node = firstChild; node; node=node->NextSibling() )
    {
      if ( !node->ToText() )
        fprintf( cfile, "\n" );
      node->Print( cfile, depth+1 );
    }
    fprintf( cfile, "\n" );
    for( i=0; i<depth; ++i )
      fprintf( cfile, "    " );
    fprintf( cfile, "</%s>", value.c_str() );
  }
}

TiXmlNode* TiXmlElement::Clone() const
{
  TiXmlElement* clone = new TiXmlElement( Value() );
  if ( !clone )
    return 0;

  CopyToClone( clone );

  // Clone the attributes, then clone the children.
  TiXmlAttribute* attribute = 0;
  for(  attribute = attributeSet.First();
      attribute;
      attribute = attribute->Next() )
  {
    clone->SetAttribute( attribute->Name(), attribute->Value() );
  }

  TiXmlNode* node = 0;
  for ( node = firstChild; node; node = node->NextSibling() )
  {
    clone->LinkEndChild( node->Clone() );
  }
  return clone;
}

// -----------------------------------------------------------------------------------
// TiXmlDocument
// -----------------------------------------------------------------------------------

TiXmlDocument::TiXmlDocument() : TiXmlNode( TiXmlNode::DOCUMENT )
{
  error = false;
  //  ignoreWhiteSpace = true;
}

TiXmlDocument::TiXmlDocument( const char * documentName ) : TiXmlNode( TiXmlNode::DOCUMENT )
{
  //  ignoreWhiteSpace = true;
  value = documentName;
  error = false;
}

bool TiXmlDocument::LoadFile( const char* filename )
{
  // Delete the existing data:
  Clear();

  value = filename;

  FILE* file = fopen( value.c_str (), "r" );

  if ( file )
  {
    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek( file, 0, SEEK_END );
    length = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Strange case, but good to handle up front.
    if ( length == 0 )
    {
      fclose( file );
      return false;
    }

    // If we have a file, assume it is all one big XML file, and read it in.
    // The document parser may decide the document ends sooner than the entire file, however.
    TIXML_STRING data;
    data.reserve( length );

    const int BUF_SIZE = 2048;
    char buf[BUF_SIZE];

    while( fgets( buf, BUF_SIZE, file ) )
    {
      data += buf;
    }
    fclose( file );

    Parse( data.c_str() );
    if (  !Error() )
      return true;
  }
  SetError( TIXML_ERROR_OPENING_FILE );
  return false;
}

bool TiXmlDocument::SaveFile( const char * filename ) const
{
  // The old c stuff lives on...
  FILE* fp = fopen( filename, "w" );
  if ( fp )
  {
    Print( fp, 0 );
    fclose( fp );
    return true;
  }
  return false;
}


TiXmlNode* TiXmlDocument::Clone() const
{
  TiXmlDocument* clone = new TiXmlDocument();
  if ( !clone )
    return 0;

  CopyToClone( clone );
  clone->error = error;
  clone->errorDesc = errorDesc.c_str ();

  TiXmlNode* node = 0;
  for ( node = firstChild; node; node = node->NextSibling() )
  {
    clone->LinkEndChild( node->Clone() );
  }
  return clone;
}


void TiXmlDocument::Print( FILE* cfile, int depth ) const
{
  TiXmlNode* node;
  for ( node=FirstChild(); node; node=node->NextSibling() )
  {
    node->Print( cfile, depth );
    fprintf( cfile, "\n" );
  }
}

// -----------------------------------------------------------------------------------
// TiXmlAttribute
// -----------------------------------------------------------------------------------

TiXmlAttribute* TiXmlAttribute::Next() const
{
  // We are using knowledge of the sentinel. The sentinel
  // have a value or name.
  if ( next->value.empty() && next->name.empty() )
    return 0;
  return next;
}


TiXmlAttribute* TiXmlAttribute::Previous() const
{
  // We are using knowledge of the sentinel. The sentinel
  // have a value or name.
  if ( prev->value.empty() && prev->name.empty() )
    return 0;
  return prev;
}


void TiXmlAttribute::Print( FILE* cfile, int /*depth*/ ) const
{
  TIXML_STRING n, v;

  PutString( Name(), &n );
  PutString( Value(), &v );

  if (value.find ('\"') == TIXML_STRING::npos)
    fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
  else
    fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
}

void TiXmlAttribute::SetIntValue( int value )
{
  char buf [64];
  sprintf (buf, "%d", value);
  SetValue (buf);
}

void TiXmlAttribute::SetDoubleValue( double value )
{
  char buf [64];
  sprintf (buf, "%lf", value);
  SetValue (buf);
}

const int TiXmlAttribute::IntValue() const
{
  return atoi (value.c_str ());
}

const double  TiXmlAttribute::DoubleValue() const
{
  return atof (value.c_str ());
}

// -----------------------------------------------------------------------------------
// TiXmlComment
// -----------------------------------------------------------------------------------

void TiXmlComment::Print( FILE* cfile, int depth ) const
{
  for ( int i=0; i<depth; i++ )
  {
    fputs( "    ", cfile );
  }
  fprintf( cfile, "%s%s%s", COMMENTHEADER, value.c_str(), COMMENTEND );
}

TiXmlNode* TiXmlComment::Clone() const
{
  TiXmlComment* clone = new TiXmlComment();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}

// -----------------------------------------------------------------------------------
// TiXmlText
// -----------------------------------------------------------------------------------

void TiXmlText::Print( FILE* cfile, int /*depth*/ ) const
{
  TIXML_STRING buffer;
  PutString( value, &buffer );
  fprintf( cfile, "%s", buffer.c_str() );
}

TiXmlNode* TiXmlText::Clone() const
{ 
  TiXmlText* clone = 0;
  clone = new TiXmlText( "" );

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}

// -----------------------------------------------------------------------------------
// TiXmlDeclaration
// -----------------------------------------------------------------------------------

TiXmlDeclaration::TiXmlDeclaration( const char * _version,
  const char * _encoding,
  const char * _standalone )
: TiXmlNode( TiXmlNode::DECLARATION )
{
  version = _version;
  encoding = _encoding;
  standalone = _standalone;
}

void TiXmlDeclaration::Print( FILE* cfile, int /*depth*/ ) const
{
  fprintf (cfile, "%s ", XMLHEADER);

  if ( !version.empty() )
    fprintf (cfile, "version=\"%s\" ", version.c_str ());
  if ( !encoding.empty() )
    fprintf (cfile, "encoding=\"%s\" ", encoding.c_str ());
  if ( !standalone.empty() )
    fprintf (cfile, "standalone=\"%s\" ", standalone.c_str ());
  fprintf (cfile, "?>");
}

TiXmlNode* TiXmlDeclaration::Clone() const
{ 
  TiXmlDeclaration* clone = new TiXmlDeclaration();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  clone->version = version;
  clone->encoding = encoding;
  clone->standalone = standalone;
  return clone;
}

// -----------------------------------------------------------------------------------
// TiXmlUnknown
// -----------------------------------------------------------------------------------

void TiXmlUnknown::Print( FILE* cfile, int depth ) const
{
  for ( int i=0; i<depth; i++ )
    fprintf( cfile, "    " );
  fprintf( cfile, "%s", value.c_str() );
}

TiXmlNode* TiXmlUnknown::Clone() const
{
  TiXmlUnknown* clone = new TiXmlUnknown();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}

// -----------------------------------------------------------------------------------
// TiXmlAttributeSet
// -----------------------------------------------------------------------------------

TiXmlAttributeSet::TiXmlAttributeSet()
{
  sentinel.next = &sentinel;
  sentinel.prev = &sentinel;
}


TiXmlAttributeSet::~TiXmlAttributeSet()
{
//  assert( sentinel.next == &sentinel );
//  assert( sentinel.prev == &sentinel );
}


void TiXmlAttributeSet::Add( TiXmlAttribute* addMe )
{
//  assert( !Find( addMe->Name() ) ); // Shouldn't be multiply adding to the set.

  addMe->next = &sentinel;
  addMe->prev = sentinel.prev;

  sentinel.prev->next = addMe;
  sentinel.prev      = addMe;
}

void TiXmlAttributeSet::Remove( TiXmlAttribute* removeMe )
{
  TiXmlAttribute* node;

  for( node = sentinel.next; node != &sentinel; node = node->next )
  {
    if ( node == removeMe )
    {
      node->prev->next = node->next;
      node->next->prev = node->prev;
      node->next = 0;
      node->prev = 0;
      return;
    }
  }
//  assert( 0 );    // we tried to remove a non-linked attribute.
}

TiXmlAttribute* TiXmlAttributeSet::Find( const char * name ) const
{
  TiXmlAttribute* node;

  for( node = sentinel.next; node != &sentinel; node = node->next )
  {
    if ( node->name == name )
      return node;
  }
  return 0;
}

// -------------------------------------------------------------------------------------------------------

const char* TiXmlBase::ReadName( const char* p, TIXML_STRING * name )
{
  *name = "";

  // Names start with letters or underscores.
  // After that, they can be letters, underscores, numbers,
  // hyphens, or colons. (Colons are valid ony for namespaces,
  // but tinyxml can't tell namespaces from names.)
  if (    p && *p 
     && ( isalpha( (unsigned char) *p ) || *p == '_' ) )
  {
    while(    p && *p
        &&  (   isalnum( (unsigned char ) *p ) 
             || *p == '_'
             || *p == '-'
             || *p == ':' ) )
    {
      (*name) += *p;
      ++p;
    }
    return p;
  }
  return 0;
}

const char* TiXmlBase::GetEntity( const char* p, char* value )
{
  // Presume an entity, and pull it out.
  TIXML_STRING ent;
  int i;

  // Ignore the &#x entities.
  if (    strncmp( "&#x", p, 3 ) == 0 
     && *(p+3) 
     && *(p+4) )
  {
    *value = 0;
    
    if ( isalpha( p[3])) *value += ( char(tolower( p[3] )) - 'a' + 10 ) * 16;
    else                 *value += ( p[3] - '0' ) * 16;

    if ( isalpha( p[4])) *value += ( char(tolower( p[4] )) - 'a' + 10 );
    else                 *value += ( p[4] - '0' );

    return p+6;
  }

  // Now try to match it.
  for( i=0; i<NUM_ENTITY; ++i )
  {
    if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
    {
//      assert( strlen( entity[i].str ) == entity[i].strLength );
      *value = entity[i].chr;
      return ( p + entity[i].strLength );
    }
  }

  // So it wasn't an entity, its unrecognized, or something like that.
  *value = *p;  // Don't put back the last one, since we return it!
  return p+1;
}


bool TiXmlBase::StringEqual( const char* p,
               const char* tag,
               bool ignoreCase )
{
  if ( !p || !tag || !*p )
    return false;

  if ( tolower( *p ) == tolower( *tag ) )
  {
    const char* q = p;

    if (ignoreCase)
    {
      while ( *q && *tag && *q == *tag )
      {
        ++q;
        ++tag;
      }

      if ( *tag == 0 )    // Have we found the end of the tag, and everything equal?
        return true;
    }
    else
    {
      while ( *q && *tag && tolower( *q ) == tolower( *tag ) )
      {
        ++q;
        ++tag;
      }

      if ( *tag == 0 )
        return true;
    }
  }
  return false;
}

const char* TiXmlBase::ReadText(  const char* p, 
                  TIXML_STRING * text, 
                  bool trimWhiteSpace, 
                  const char* endTag, 
                  bool caseInsensitive )
{
    *text = "";
  if (    !trimWhiteSpace     // certain tags always keep whitespace
     || !condenseWhiteSpace ) // if true, whitespace is always kept
  {
    // Keep all the white space.
    while (    p && *p
        && !StringEqual( p, endTag, caseInsensitive )
        )
    {
      char c;
      p = GetChar( p, &c );
            (* text) += c;
    }
  }
  else
  {
    bool whitespace = false;

    // Remove leading white space:
    p = SkipWhiteSpace( p );
    while (    p && *p
        && !StringEqual( p, endTag, caseInsensitive ) )
    {
      if ( IsWhiteSpace( *p ) )
      {
        whitespace = true;
        ++p;
      }
      else
      {
        // If we've found whitespace, add it before the
        // new character. Any whitespace just becomes a space.
        if ( whitespace )
        {
         (* text) += ' ';
          whitespace = false;
        }
        char c;
        p = GetChar( p, &c );
            (* text) += c;
      }
    }
  }
  return p + strlen( endTag );
}

const char* TiXmlDocument::Parse( const char* p )
{
  // Parse away, at the document level. Since a document
  // contains nothing but other tags, most of what happens
  // here is skipping white space.
  //
  // In this variant (as opposed to stream and Parse) we
  // read everything we can.

  if ( !p || !*p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY );
    return false;
  }

    p = SkipWhiteSpace( p );
  if ( !p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY );
    return false;
  }

  while ( p && *p )
  {
    TiXmlNode* node = Identify( p );
    if ( node )
    {
      p = node->Parse( p );
      LinkEndChild( node );
    }
    else
    {
      break;
    }
    p = SkipWhiteSpace( p );
  }
  // All is well.
  return p;
}


TiXmlNode* TiXmlNode::Identify( const char* p )
{
  TiXmlNode* returnNode = 0;

  p = SkipWhiteSpace( p );
  if( !p || !*p || *p != '<' )
    return 0;

  TiXmlDocument* doc = GetDocument();
  p = SkipWhiteSpace( p );

  if ( !p || !*p )
    return 0;

  // What is this thing? 
  // - Elements start with a letter or underscore, but xml is reserved.
  // - Comments: <!-- ,,, -->
  // - Decleration: <?xml
  // - Everthing else is unknown to tinyxml.

  if      ( StringEqual( p, XMLHEADER, true ) )
    returnNode = new TiXmlDeclaration();
  else if ( isalpha( p[1] ) || p[1] == '_' )
    returnNode = new TiXmlElement( "" );
  else if ( StringEqual( p, COMMENTHEADER, false ) )
    returnNode = new TiXmlComment();
  else
    returnNode = new TiXmlUnknown();

  if ( returnNode )
    returnNode->parent = this; // Set the parent, so it can report errors
  else
  {
    if ( doc )
      doc->SetError( TIXML_ERROR_OUT_OF_MEMORY );
  }
  return returnNode;
}

const char* TiXmlElement::Parse( const char* p )
{
  p = SkipWhiteSpace( p );
  TiXmlDocument* document = GetDocument();

  if ( !p || !*p || *p != '<' )
  {
    if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT );
    return false;
  }

  p = SkipWhiteSpace( p+1 );

  // Read the name.
  p = ReadName( p, &value );
  if ( !p || !*p )
  {
    if ( document ) document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME );
    return false;
  }

  TIXML_STRING endTag ("</");
  endTag += value;
  endTag += ">";

  // Check for and read attributes. Also look for an empty
  // tag or an end tag.
  while ( p && *p )
  {
    p = SkipWhiteSpace( p );
    if ( !p || !*p )
    {
      if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
      return 0;
    }
    if ( *p == '/' )
    {
      ++p;
      // Empty tag.
      if ( *p  != '>' )
      {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_EMPTY );    
        return 0;
      }
      return (p+1);
    }
    else if ( *p == '>' )
    {
      // Done with attributes (if there were any.)
      // Read the value -- which can include other
      // elements -- read the end tag, and return.
      ++p;
      p = ReadValue( p );   // Note this is an Element method, and will set the error if one happens.
      if ( !p || !*p )
        return 0;

      // We should find the end tag now
      if ( StringEqual( p, endTag.c_str(), false ) )
      {
        p += endTag.length();
        return p;
      }
      else
      {
        if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG );
        return 0;
      }
    }
    else
    {
      // Try to read an element:
      TiXmlAttribute attrib;
      attrib.SetDocument( document );
      p = attrib.Parse( p );

      if ( !p || !*p )
      {
        if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT );
        return 0;
      }
      SetAttribute( attrib.Name(), attrib.Value() );
    }
  }
  return p;
}

const char* TiXmlElement::ReadValue( const char* p )
{
  TiXmlDocument* document = GetDocument();

  // Read in text and elements in any order.
  p = SkipWhiteSpace( p );
  while ( p && *p )
  {
    if ( *p != '<' )
    {
      // Take what we have, make a text element.
      TiXmlText* textNode = new TiXmlText( "" );

      if ( !textNode )
      {
        if ( document ) document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
            return 0;
      }

      p = textNode->Parse( p );

      if ( !textNode->Blank() )
        LinkEndChild( textNode );
      else
        delete textNode;
    } 
    else 
    {
      // We hit a '<'
      // Have we hit a new element or an end tag?
      if ( StringEqual( p, "</", false ) )
        return p;
      else
      {
        TiXmlNode* node = Identify( p );
        if ( node )
        {
          p = node->Parse( p );
          LinkEndChild( node );
        }       
        else
          return 0;
      }
    }
    p = SkipWhiteSpace( p );
  }

  if ( !p )
  {
    if ( document ) document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE );
  } 
  return p;
}


const char* TiXmlUnknown::Parse( const char* p )
{
  TiXmlDocument* document = GetDocument();
  p = SkipWhiteSpace( p );
  if ( !p || !*p || *p != '<' )
  {
    if ( document ) document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
    return 0;
  }
  ++p;
    value = "";

  while ( p && *p && *p != '>' )
  {
    value += *p;
    ++p;
  }

  if ( !p )
  {
    if ( document ) document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
  }
  if ( *p == '>' )
    return p+1;
  return p;
}

const char* TiXmlComment::Parse( const char* p )
{
  TiXmlDocument* document = GetDocument();
  value = "";

  p = SkipWhiteSpace( p );

  if ( !StringEqual( p, COMMENTHEADER, false ) )
  {
    document->SetError( TIXML_ERROR_PARSING_COMMENT );
    return 0;
  }
  p += strlen( COMMENTHEADER );
  p = ReadText( p, &value, false, COMMENTEND, false );
  return p;
}


const char* TiXmlAttribute::Parse( const char* p )
{
  p = SkipWhiteSpace( p );
  if ( !p || !*p ) return 0;

  // Read the name, the '=' and the value.
  p = ReadName( p, &name );
  if ( !p || !*p )
  {
    if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }
  p = SkipWhiteSpace( p );
  if ( !p || !*p || *p != '=' )
  {
    if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }

  ++p;  // skip '='
  p = SkipWhiteSpace( p );
  if ( !p || !*p )
  {
    if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }
  
  const char* end;

  if ( *p == '\'' )
  {
    ++p;
    end = "\'";
    p = ReadText( p, &value, false, end, false );
  }
  else if ( *p == '"' )
  {
    ++p;
    end = "\"";
    p = ReadText( p, &value, false, end, false );
  }
  else
  {
    // All attribute values should be in single or double quotes.
    // But this is such a common error that the parser will try
    // its best, even without them.
    value = "";
    while (    p && *p                    // existence
        && !IsWhiteSpace( *p )            // whitespace
        && *p != '/' && *p != '>' )           // tag end
    {
      value += *p;
      ++p;
    }
  }
  return p;
}

const char* TiXmlText::Parse( const char* p )
{
  value = "";

  //TiXmlDocument* doc = GetDocument();
  bool ignoreWhite = true;
//  if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;

  const char* end = "<";
  p = ReadText( p, &value, ignoreWhite, end, false );
  if ( p )
    return p-1; // don't truncate the '<'
  return 0;
}

const char* TiXmlDeclaration::Parse( const char* p )
{
  p = SkipWhiteSpace( p );
  // Find the beginning, find the end, and look for
  // the stuff in-between.
  TiXmlDocument* document = GetDocument();
  if ( !p || !*p || !StringEqual( p, XMLHEADER, true ) )
  {
    if ( document ) document->SetError( TIXML_ERROR_PARSING_DECLARATION );
    return 0;
  }
  p += 5;

  version = "";
  encoding = "";
  standalone = "";

  while ( p && *p )
  {
    if ( *p == '>' )
    {
      ++p;
      return p;
    }

    p = SkipWhiteSpace( p );
    if ( StringEqual( p, "version", true ) )
    {
      TiXmlAttribute attrib;
      p = attrib.Parse( p );    
      version = attrib.Value();
    }
    else if ( StringEqual( p, "encoding", true ) )
    {
      TiXmlAttribute attrib;
      p = attrib.Parse( p );    
      encoding = attrib.Value();
    }
    else if ( StringEqual( p, "standalone", true ) )
    {
      TiXmlAttribute attrib;
      p = attrib.Parse( p );    
      standalone = attrib.Value();
    }
    else
    {
      // Read over whatever it is.
      while( p && *p && *p != '>' && !IsWhiteSpace( *p ) )
        ++p;
    }
  }
  return 0;
}

bool TiXmlText::Blank() const
{
  for ( unsigned i=0; i<value.length(); i++ )
    if ( !isspace( value[i] ) )
      return false;
  return true;
}

