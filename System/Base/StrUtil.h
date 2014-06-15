//----------------------------------------------------------------------------
//  Nombre:    StrUtil.h
//
//  Contenido: Utilidades de manejo de cadenas.
//----------------------------------------------------------------------------

#ifndef _STR_UTIL_H_
#define _STR_UTIL_H_

namespace StrUtil
{
  // --------------------------
  // Remove a string's trailing and leading blanks. It also
  // removes CTRL chars and C++-style '//' comments.
  // 'src' can be the same as 'dest' if desired.
  extern char *CleanLine(char *pszDest, const char *pszSrc);

  // --------------------------
  // Split a string in as many as nstr tokens. Tokens can be grouped
  // by " or '. Returns the number of tokens, and fills ppc with
  // pointers to the tokens. Last token is the rest of the string.
  // This works really like creating an argc-argv for a 'main' function.
  // NOTE: This modifies the src string.
  extern int SplitLine(char *ppc[], int nstr, char *pszSrc);

  // --------------------------
  // Skip whitespace, then return the next token: consecutive non-whitespaces
  // or quote-enclosed characters.
  extern bool   GetToken  (char *pDest, int size, const char *&pSrc);

  extern int    GetInt    (const char *&pSrc, int def = 0);
  extern float  GetFloat  (const char *&pSrc, float def = 0.f);
  extern bool   GetBool   (const char *&pSrc, bool bDef = false);

  // --------------------------
  // strncpy replacement, admits NULL parameters and will always end
  // the dest string with '\0'. If len == -1 it just acts like strcpy.
  extern char *SafeStrncpy(char *pDest, const char *pSrc, int max = -1);

  // ----------------------------------
  // Simple way to search for "-something" parameters in a command-line,
  // You are expected to pass the '-' if you want one.
  // Returns NULL if the parameter does not exist, or "" if it does.
  // If the found parameter is of the form "param:something" ('=' also works)
  // then it returns a pointer to "something".
  extern const char *FindParameterString(const char *pszString, const char *pszParameter);


  // String class modified from TinyXml, See .cpp for details.
  class CString
  {
    public :
      // CString empty constructor
      CString (): m_len(0), m_total(0), m_pszStr(0)     { }
      // CString constructor, based on a string
      CString (const char * pszStr);
      // CString copy constructor
      CString (const CString& copy);

      // CString destructor
      ~ CString ()                                            { empty_it (); }

      // Convert a CString into a classical char *
      const char *  c_str   () const                          { return m_total? m_pszStr : ""; }
      // Return the length of a CString
      unsigned      length  () const                          { return m_len; }
      // Checks if a CString is empty
      bool          empty   () const                          { return m_len ? false : true; }

      // CString = operator
      void operator = (const char *pszStr);

      // = operator
      void operator = (const CString & copy)                  { assign(copy.m_pszStr, copy.m_len); }

      // += operator. Maps to append
      CString& operator += (const char * pszStr)              { append (pszStr); return *this; }
      CString& operator += (char single)                      { append (single); return *this; }
      CString& operator += (CString & suffix)                 { append (suffix); return *this; }

      bool operator == (const CString & compare) const        { return internalCompare(compare) == 0; }
      bool operator <  (const CString & compare) const        { return internalCompare(compare) <  0; }
      bool operator >  (const CString & compare) const        { return internalCompare(compare) >  0; }

      // Checks if a CString contains only whitespace (same rules as isspace)
	  // Not actually used in tinyxml. Conflicts with a C macro, "isblank",
	  // which is a problem. Commenting out. -lee
  //    bool isblank () const;

      // single char extraction and [] operator 
      const char& at          (unsigned index) const          { return m_pszStr [index]; }
      const char& operator [] (unsigned index) const          { return m_pszStr [index]; }

      // find a char in a string. Return CString::notfound if not found
      unsigned find (char lookup) const                       { return find (lookup, 0); }
      // find a char in a string from an offset. Return CString::notfound if not found
      unsigned find (char tofind, unsigned offset) const;
      // Error value for find primitive 
      enum { notfound = 0xffffffff, npos = notfound };

      // Function to reserve a big amount of data when we know we'll need it.
		  // This function clears the content of the CString if any exists.
      void reserve (unsigned size);

      void assign (const char *pszStr, unsigned lenstr);
      void append (const char *pszStr, unsigned lenstr);

      void append (const char *pszStr );
      void append (const CString & suffix)                    { append(suffix.m_pszStr, suffix.m_len); }
      void append (char single)                               { append (&single, 1); }

    protected :
      // The base string
      char *    m_pszStr;
      // Number of chars m_total
      unsigned  m_total;
      // length of actual string
      unsigned  m_len;

      // Internal function that clears the content of a CString
      void empty_it ();

      int internalCompare(const CString & compare) const;
  } ;
};

#endif // _STR_UTIL_H_
