//----------------------------------------------------------------------------
//  Nombre:    XtractedFont.h
//
//  Contenido: Fuente de letras
//----------------------------------------------------------------------------

#ifndef _DISPLAY_FONT_H_
#define _DISPLAY_FONT_H_

#include "DisplayShader.h"
#include "DynamicVB.h"
#include "DynArray.h"

//----------------------------------------------------------------------------

class CDisplayDevice;
class CDisplayTexture;
class CRenderContext;
class CDynamicVB;
class CFile;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CXtractedFont
{
  enum { MAX_CHARS = 256 };
  public:
    CXtractedFont      (): m_pszName(NULL)        { }
    ~CXtractedFont     ()                         { End(); }

    TError    Init        (const char *pszName, int fontSize, int charScale, bool bBold, bool bItalic, CDisplayDevice *pDev = NULL);  // Device for immediate Load.
    void      End         ();
    bool      IsOk        ()                    const { return m_pszName != NULL; }

    const char *GetName       ()                const { return m_pszName; }
    int         GetNChars     ()                const { return m_nChars; }
    int         GetCharWidth  (unsigned c)      const { ASSERT(IsOk()); ASSERT(c < MAX_CHARS); int l = m_aCharMap[c]; return (l == -1)? 0 : m_paChars[l].logWidth; }
    int         GetFontHeight ()                const { ASSERT(IsOk()); return m_fontHeight; }

    bool        IsLoaded  ()                    const { return (m_paTextures != NULL); }
    TError      Load      (CDisplayDevice *pDev);
    void        Unload    ();

    void        Printf    (CRenderContext &rc, float x, float y, dword color, const char *pszFmt, ...) const;
    void        Printf    (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszFmt, ...) const;

    void        Puts      (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszStr) const;

  private:

    struct TCharInfo
    {
      int nTex;
      int x, y, w, h;
      int offsX, offsY;
      int logWidth;
    };

    struct TLetter;
    typedef CDynArray<TLetter> CLetterArray;

    bool CreateLetters  (CLetterArray &vecPosiciones);
    void CreateTextures (CLetterArray &vecPosiciones, CDisplayDevice *pDev);

    char           *m_pszName;
    int             m_fontSize;
    int             m_charScale;
    bool            m_bBold;
    bool            m_bItalic;

    int             m_aCharMap[MAX_CHARS];
    int             m_nChars;
    int             m_fontHeight;
    TCharInfo      *m_paChars;
    int             m_nTextures;
    CDisplayTexture *m_paTextures;
    
    // Lo tengo aqui para no inicializarlo cada vez que dibujo,
    // pero no afecta al constness.
    mutable CDynamicVB      m_VB;                  // TIENE que ser TTLVertexUV
    mutable CDisplayShader  m_Shader;
};

#endif // _DISPLAY_FONT_H_
