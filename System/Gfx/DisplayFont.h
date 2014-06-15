//----------------------------------------------------------------------------
//  Nombre:    DisplayFont.h
//
//  Contenido: Fuente de letras
//----------------------------------------------------------------------------

#ifndef _DISPLAY_FONT_H_
#define _DISPLAY_FONT_H_

#include "DisplayShader.h"

//----------------------------------------------------------------------------

class CDisplayDevice;
class CDisplayTexture;
class CRenderContext;
class CDynamicVB;
class CFile;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CDisplayFont
{
  enum { MAX_CHARS = 256 };
  public:
    CDisplayFont      (): m_nChars(NULL)         { }
    ~CDisplayFont     ()                         { End(); }

    TError    Init        (const char *pszName, CDynamicVB *pVB, CDisplayDevice *pDev = NULL);  // Device for immediate Load.
    void      End         ();
    bool      IsOk        ()                    const { return m_nChars > 0; }

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

    TError      LoadTextures  (CDisplayDevice *pDev, CFile *pFile);

    struct TCharInfo
    {
      int nTex;
      int x, y, w, h;
      int offsX, offsY;
      int logWidth;
    };

    struct TTexture;

    char           *m_pszName;
    CDynamicVB     *m_pVB;                  // TIENE que ser TTLVertexUV
    int             m_aCharMap[MAX_CHARS];
    int             m_nChars;
    int             m_fontHeight;
    TCharInfo      *m_paChars;
    int             m_nTextures;
    TTexture       *m_paTextures;
    
    // Lo tengo aqui para no inicializarlo cada vez que dibujo,
    // pero no afecta al constness.
    mutable CDisplayShader  m_Shader;
};

#endif // _DISPLAY_FONT_H_
