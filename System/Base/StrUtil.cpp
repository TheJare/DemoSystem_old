//----------------------------------------------------------------------------
//  Nombre:    StrUtil.cpp
//
//  Contenido: Utilidades de manejo de cadenas.
//----------------------------------------------------------------------------

#include "StrUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --------------------------------------------------------------------------
// Function:      CleanLine
// Purpose:       Remove a string's trailing and leading blanks. It also
//                removes CTRL chars and C++-style '//' comments.
//                'src' can be the same as 'dest' if desired.
// --------------------------------------------------------------------------
char *StrUtil::CleanLine(char *pszDest, const char *pszSrc)
{
  char *pszStore;
  bool bAlgo = false;

  if (pszSrc == NULL)
  {
    *pszDest='\0';
    return NULL;
  }

  while ( *pszSrc && (iscntrl(*pszSrc) || isspace(*pszSrc)) )
    pszSrc++;
  if (!*pszSrc)
  {
    *pszDest = '\0';
    return pszDest;
  }
  pszStore = pszDest;

  int  inComment = 0;
  bool bInLineComment = false;

  while (*pszSrc)
  {
    char c = *pszSrc;
    if (!bInLineComment && !inComment)
    {
      if (!iscntrl(c))
      {
        if (c == '/' && pszSrc[1] == '/')
        {
          bInLineComment = true;
          if (bAlgo)
            *pszStore++ = ' ';
          pszSrc++;
        }
        else if (c == '/' && pszSrc[1] == '*')
        {
          inComment++;
          if (bAlgo)
            *pszStore++ = ' ';
          pszSrc++;
        }
        else if (bAlgo || !isspace(c))
        {
          bAlgo = true;
          *pszStore++ = c;       // Aquí está seguro de que hay algo.
        }
      }
      else if (bAlgo)
        *pszStore++ = ' ';        // Sustituye ctrls por espacio.
    }
    if (c == '*' && pszSrc[1] == '/')
    {
      inComment--;
      pszSrc++;
    }
    if (bInLineComment && c == '\n')
      bInLineComment = false;
    pszSrc++;
  }
  if (!bAlgo || pszStore == pszDest)
  {
    *pszDest = '\0';
    return pszDest;
  }

  do
    pszStore--;
  while (pszStore >= pszDest && isspace(*pszStore));
  pszStore[1] = '\0';
  return pszDest;
}

// --------------------------------------------------------------------------
// Function:      SplitLine
// Purpose:       Turns a string into a series of tokens, argv-style
// --------------------------------------------------------------------------
int StrUtil::SplitLine(char *ppc[], int nstr, char *pszSrc)
{
  char *p = pszSrc;
  int  n = 0;

  if (pszSrc == NULL || ppc == NULL || nstr == 0)
    return 0;
  do
  {
    while (*p == ' ' || *p == '\t')
      p++;
    if (*p == '\0')
      return n;
    if (*p == '\"' || *p == '\'')
    {
      char del = *p++;
      *ppc++ = p;
      n++;
      if ( (p = strchr(p, del)) == NULL)
        return n;
      if (*p == '\0')
        return n;
       *p++ = '\0';
    }
    else
    {
      *ppc++ = p;
      while (*p != ' ' && *p != '\t' && *p != '\0')
          p++;
      n++;
      if (*p == '\0')
        return n;
      *p++ = '\0';
    }
  }
  while (*p != '\0' && n < nstr);
  if (*p != '\0')
    *ppc = p;
  return n;
}

// --------------------------------------------------------------------------
// Function:      GetToken
// Purpose:       Skip whitespace, then return the next token: consecutive 
//                non-whitespaces or quote-enclosed characters.
//                It then advances the string pointer.
// --------------------------------------------------------------------------
bool StrUtil::GetToken(char *pDest, int size, const char *&pSrc)
{
  // If we weren't given an actual buffer to store, just skip the token
  if (pDest == NULL)
    size = 0;

  if (size > 0)
    *pDest = '\0';
  while (*pSrc != '\0' && isspace(*pSrc))
    pSrc++;

  char cEncloser = '\0';
  char c = *pSrc;
  bool bRet = false;

  // Handle quotes
  if (c == '\'' || c == '\"')
  {
    cEncloser = *pSrc++;
    c = *pSrc;
  }

  // If there are chars, there is a token to extract.
  if (c != '\0')
    bRet = true;

  while (c != '\0')  // NOTE: Additional exit condition inside the loop. Hmpfff..
  {
    bool bEscape = false;
    if (cEncloser != '\0')
    {
      // Quoted?
      if (c == cEncloser)
      {
        pSrc++;
        break;
      }
      if (c == '\\')    // Escape char inside a quoted string
      {
        pSrc++;
        bEscape = true;
      }
    }
    else
    {
      // Unquoted token
      if (isspace(c))
        break;
    }
    if (*pSrc != '\0')
    {
      // Always leave room for the trailiing \0.
      if (size > 1)
      {
        c = *pSrc;
        if (bEscape)
        {
          // Translate the escape code.
          if (c == 'n')
            c = '\n';
          else if (c == 't') c = '\t';
          else if (c == 'v') c = '\v';
          else if (c == 'b') c = '\b';
          else if (c == 'r') c = '\r';
          else if (c == '0')
          {
            // Octal number
            c = 0;
            for (int k = 0; k < 3; k++)
            {
              if (pSrc[1] >= '0' && pSrc[1] <= '7')
              {
                c = c*8 + (pSrc[1]-'0');
                pSrc++;
              }
            }
          }
          else
            ;// verbatim copy.
        }
        *pDest++ = c;
        size--;
      }
      pSrc++;
    }

    // Advance
    c = *pSrc;
  }
  // Skip white space after the token.
  while (*pSrc != '\0' && isspace(*pSrc))
    pSrc++;

  // Close the token.
  if (size > 0)
    *pDest = '\0';
  return bRet;
}

// --------------------------------------------------------------------------
// Function:      GetInt
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int StrUtil::GetInt(const char *&pSrc, int def)
{
  char buf[1000];
  if (StrUtil::GetToken(buf, sizeof(buf), pSrc))
    sscanf(buf, "%i", &def);
  return def;
}

// --------------------------------------------------------------------------
// Function:      GetFloat
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float StrUtil::GetFloat(const char *&pSrc, float def)
{
  char buf[1000];
  if (StrUtil::GetToken(buf, sizeof(buf), pSrc))
    return (float)atof(buf);
  return def;
}

// --------------------------------------------------------------------------
// Function:      GetBool
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool StrUtil::GetBool(const char *&pSrc, bool bDef)
{
  char buf[1000];
  if (StrUtil::GetToken(buf, sizeof(buf), pSrc))
  {
    if (stricmp(buf, "false") == 0)
      return false;
    if (stricmp(buf, "true") == 0)
      return true;
    return (0 != atoi(buf));
  }
  return bDef;
}

// --------------------------------------------------------------------------------
// Name        : SafeStrncpy
// Returns     : pDest
// Description : strncpy replacement, admits NULL parameters and will always end
//               the dest string with '\0'. If len == -1 it just acts like strcpy.
// --------------------------------------------------------------------------------
char *StrUtil::SafeStrncpy(char *pDest, const char *pSrc, int max)
{
  if (pDest && max != 0)
  {
    if (pSrc)
    {
      if (max > 0)
      {
        strncpy(pDest, pSrc, max);
        pDest[max-1] = '\0';
      }
      else if (max == -1)
        strcpy(pDest, pSrc);
    }
    else
      pDest[0] = '\0';
  }
  return pDest;
}

// --------------------------------------------------------------------------------
// Name        : FindParameterString
// Description : Simple way to search for "-something" parameters in a command-line,
//               You are expected to pass the '-' if you want one.
//               Returns NULL if the parameter does not exist, or "" if it does.
//               If the found parameter is of the form "param:something" ('=' also works)
//               then it returns a pointer to "something".
// --------------------------------------------------------------------------------
const char *StrUtil::FindParameterString(const char *pszString, const char *pszParameter)
{
  const char *p = pszString;
  int plen = strlen(pszParameter);

  while (p && *p != '\0')
  {
    while (isspace(*p))
      p++;
    if (strnicmp(pszString, pszParameter, plen) == 0)
    {
      if (p)
      {
        p += plen;
        if (*p == ':' || *p == '=')
          ++p;
        else
          p = "";
      }
      return p;
    }
    p++;
  }
  return NULL;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// CString
//
// CString is an emulation of the std::string template.
// Only the member functions relevant to the TinyXML project have been implemented.
// The buffer allocation is made by a simplistic power of 2 like mechanism : if we increase
// a string and there's no more room, we allocate a buffer twice as big as we need.
//
//  Modified string class
//  Changes include formatting, refactoring, optimization
//  and overall clarity IMHO.
//
//  Original taken from:
//
//  www.sourceforge.net/projects/tinyxml
//  Original file by Yves Berquin.
//
//  This software is provided 'as-is', without any express or implied 
//  warranty. In no event will the authors be held liable for any 
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any 
//  purpose, including commercial applications, and to alter it and 
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must 
//  not claim that you wrote the original software. If you use this 
//  software in a product, an acknowledgment in the product documentation 
//  would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//  must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source 
//  distribution.
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace StrUtil
{
  namespace
  {
    // New size computation. It is simplistic right now : it returns twice the amount
    unsigned assign_new_size (unsigned minimum_to_allocate) { return minimum_to_allocate * 2; }
  }

  // --------------------------------------------------------------------------
  // Function:      Constructor
  // Purpose:       CString initialization constructor
  // Parameters:    
  // --------------------------------------------------------------------------
  CString::CString (const char* pszStr):
    m_len(0), m_total(0), m_pszStr(0)
  {
    if (pszStr && *pszStr)
      assign(pszStr, strlen(pszStr));
  }

  // --------------------------------------------------------------------------
  // Function:      Constructor
  // Purpose:       CString copy constructor
  // Parameters:    
  // --------------------------------------------------------------------------
  CString::CString (const CString& copy):
    m_len(0), m_total(0), m_pszStr(0)
  {
	  // Prevent copy to self!
	  if ( &copy != this && copy.m_pszStr && *copy.m_pszStr)
      assign(copy.m_pszStr, copy.m_len);
  }

  // --------------------------------------------------------------------------
  // Function:      assign
  // Purpose:       Assign a memory buffer
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::assign (const char * pszStr, unsigned lenstr)
  {
    if (!pszStr || !lenstr)
      empty_it ();
    else if (pszStr != m_pszStr)
    {
      // check if we need to expand
      if (lenstr >= m_total)
      {
        delete[] m_pszStr;
        m_pszStr = new char [lenstr+1];
        m_total = lenstr+1;
      }
      memcpy (m_pszStr, pszStr, lenstr);
      m_len = lenstr;
      m_pszStr [m_len] = '\0';
    }
  }

  // --------------------------------------------------------------------------
  // Function:      operator=
  // Purpose:       CString = operator. Safe when assign own content
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::operator = (const char *pszStr)
  {
    if (!pszStr || !*pszStr)
      empty_it ();
    else if (pszStr != m_pszStr)
      assign(pszStr, strlen (pszStr));
  }

  // --------------------------------------------------------------------------
  // Function:      append
  // Purpose:       append a const char * to an existing CString
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::append( const char *pszStr, unsigned len )
  {
    unsigned new_len = m_len + len;
    // check if we need to expand
    if (new_len >= m_total)
    {
      // compute new size & allocate new buffer
      unsigned newSize = assign_new_size (new_len+1);
      char *pszNew = new char [newSize];    

      // copy the previous m_total buffer into this one
      if (m_len)
        memcpy(pszNew, m_pszStr, m_len);
      delete[] m_pszStr;
      // update member variables
      m_pszStr = pszNew;
      m_total = newSize;
    }
    // append the suffix. It does exist, otherwise we wouldn't be expanding 
    memcpy(m_pszStr+m_len, pszStr, len);
    m_len = new_len;
    m_pszStr [m_len] = 0;
  }

  // --------------------------------------------------------------------------
  // Function:      append
  // Purpose:       Append a const char * to an existing CString
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::append( const char *pszStr)
  {
    if (pszStr && *pszStr)
      append(pszStr, strlen(pszStr));
  }

  // --------------------------------------------------------------------------
  // Function:      find
  // Purpose:       
  // Parameters:    
  // --------------------------------------------------------------------------
  unsigned CString::find (char tofind, unsigned offset) const
  {
    if (offset < m_len)
    {
      for (const char *p = m_pszStr + offset; *p; ++p)
        if (*p == tofind)
          return p - m_pszStr;
    }
    return (unsigned) notfound;
  }

  // --------------------------------------------------------------------------
  // Function:      internalCompare
  // Purpose:       
  // Parameters:    
  // --------------------------------------------------------------------------
  int CString::internalCompare(const CString & compare) const
  {
	  if ( m_total && compare.m_total )
		  return strcmp( m_pszStr, compare.m_pszStr );
	  return 0;
  }

  // --------------------------------------------------------------------------
  // Function:      reserve
  // Purpose:       
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::reserve (unsigned size)
  {
    empty_it();
    if (size)
    {
      m_total = size;
      m_pszStr = new char[size];
      m_pszStr[0] = '\0';
    }
  }

  // --------------------------------------------------------------------------
  // Function:      empty_it
  // Purpose:       
  // Parameters:    
  // --------------------------------------------------------------------------
  void CString::empty_it ()
  {
    delete[] m_pszStr;
    m_pszStr = NULL;
    m_total = 0;
    m_len = 0;
  }

} //namespace
