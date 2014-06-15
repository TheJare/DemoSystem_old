// -------------------------------------------------------------------------------------
// File:        ReadChunker.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _READ_CHUNKER_H_
#define _READ_CHUNKER_H_

struct TVector3;
struct TQuaternion;
#include <string>

class CReadChunker
{
  public:
    CReadChunker(): m_pStr(NULL)    {   }
    ~CReadChunker()                 { End(); }

    CReadChunker        (const CReadChunker &ch, bool bAlreadyIn = false);

    TError  Init        (const char *pszName);
    TError  Init        (const CReadChunker &pChunker, bool bAlreadyIn = false);
    void    End         ();
    bool    IsOk        ()                          const { return m_pStr != NULL; }


    bool          Eof         ()                          { ReadToken(); return m_level < 0; }
    const char *  GetToken    ()                    const { return m_szToken; }
    const char *  ReadToken   ();
    const char *  ReadTokenDiscard()                      { const char *p = ReadToken(); m_bRead = false; return p; }
    float         ReadFloat   (float fDef = 0)            { return ReadTokenDiscard()? (float)atof(m_szToken) : fDef; }
    dword         ReadDword   (dword def = 0)             { if (ReadTokenDiscard()) sscanf(m_szToken, "%i", &def); return def; }
    int           ReadInt     (int def = 0)               { if (ReadTokenDiscard()) sscanf(m_szToken, "%i", &def); return def; }
    bool          IsToken     (const char *pszTok)        { return ReadToken()? (strcmp(m_szToken, pszTok) == 0) : false; }

    void          SkipValue   ();
    bool          ReadValue   (const char *pszTok, bool *pBool);
    bool          ReadValue   (const char *pszTok, int *pInt);
    bool          ReadValue   (const char *pszTok, float *pFloat);
    bool          ReadValue   (const char *pszTok, char *pszString, int len);
    bool          ReadValue   (const char *pszTok, std::string *s);
    bool          ReadValue   (const char *pszTok, TVector3 *pVec);
    bool          ReadValue   (const char *pszTok, TQuaternion *pQuat);
    bool          ReadValue   (const char *pszTok, dword *pDword)               { return ReadValue(pszTok, (int*)pDword); }

  private:
    struct TStrInfo
    {
      int         nRef;
      const char *pszCursor;
      char        szStr[1];
    };

    TStrInfo  *m_pStr;
    int       m_level;
    bool      m_bRead;
    char      m_szToken[500];
};

#endif //_READ_CHUNKER_H_
