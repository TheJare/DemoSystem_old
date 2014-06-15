//----------------------------------------------------------------------------
//  Nombre:    XtractedFont.cpp
//
//  Contenido: Fuente de letras
//----------------------------------------------------------------------------

#include "GfxPCH.h"
#include "XtractedFont.h"
#include "DisplayDevice.h"
#include "DisplayTexture.h"
#include "DisplayVertex.h"
#include "RenderContext.h"
#include "DynamicVB.h"

#include <windows.h>
#include <algorithm>
#include "RectPlacement.h"

#define MAX_TEXTURES   16
#define MAX_TEXTURE_W 512
#define MAX_TEXTURE_H 512

#define ROUND_DOWN(a,b) (((a)/(b))*(b))
#define ROUND_UP(a,b)   ((((a)+(b)-1)/(b))*(b))

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

struct CXtractedFont::TLetter: public CRectPlacement::TRect
{
  int n;  // ASCII

  int offsetX, offsetY;
  int logWidth;
  int nTex;
  byte *pPix;
  int prevChar;

  TLetter() { }
  TLetter(int _n, int _x, int _y, int _w, int _h, int ox, int oy, int lw, byte *_pPix, int pc = -1):
          TRect(_x, _y, _w, _h), n(_n), offsetX(ox), offsetY(oy), logWidth(lw), nTex(0), pPix(_pPix), prevChar(pc) { }

  // ASCII sorting
  static bool LessIndex(const TLetter &a, const TLetter &b) { return a.n < b.n; }
};

// --------------------------------------------------------------------------------
// Nombre      : 
// Retorno     : 
// Descripcion : 
// --------------------------------------------------------------------------------
bool CXtractedFont::CreateLetters  (CLetterArray &vecPosiciones)
{
  GLOG(("Generando bitmaps de las letras...\n"));

  // Calc values to be used throughout the font rendering.
  int imgWidth = ROUND_UP(m_fontSize*3, m_charScale);
  int imgHeight = imgWidth;
  int offsetChar = ROUND_UP(m_fontSize/2, m_charScale);
  int interChar = m_charScale;

  // Prepare to create a bitmap
  DWORD*      pBitmapBits;
  BITMAPINFO bmi;
  ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
  bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth       =  (int)imgWidth;
  bmi.bmiHeader.biHeight      = -(int)imgHeight;
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biBitCount    = 32;

  // Create a DC and a bitmap for the font
  HDC     hDC       = ::CreateCompatibleDC( NULL );
  HBITMAP hbmBitmap = ::CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                         (VOID**)&pBitmapBits, NULL, 0 );
  ::SetMapMode( hDC, MM_TEXT );

  // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
  // antialiased font, but this is not guaranteed.
  INT nHeight    = -MulDiv( m_fontSize, (INT)GetDeviceCaps(hDC, LOGPIXELSY), 72 );
  DWORD dwBold   = (m_bBold)   ? FW_BOLD : FW_NORMAL;
  DWORD dwItalic = (m_bItalic) ? TRUE    : FALSE;
  HFONT hFont    = ::CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic,
                                FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
//                                CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
                                VARIABLE_PITCH, m_pszName);
  if (NULL == hFont)
      return false;

  ::SelectObject( hDC, hbmBitmap );
  ::SelectObject( hDC, hFont );

  // Set text properties
  ::SetTextColor( hDC, RGB(255,255,255) );
  ::SetBkColor  ( hDC, 0x00000000 );
  ::SetTextAlign( hDC, TA_TOP );

  // Find each letter's size (+ extra area)
  for (int i = 32; i < 256; i++)
  {
    char str[2] = { (char)i, '\0' };
    SIZE size = { 0, 0 };

    GLOG(("."));

    // Write the char
    memset(pBitmapBits, 0, imgWidth*imgHeight*sizeof(*pBitmapBits));
    ::GetTextExtentPoint32( hDC, str, 1, &size );
    ::ExtTextOut( hDC, offsetChar, offsetChar, ETO_OPAQUE, NULL, str, 1, NULL );

    // Find char extents
    int minx = imgWidth;
    int miny = imgHeight;
    int maxx = 0;
    int maxy = 0;
    int x, y;
    for (y = 0; y < imgHeight; y++)
      for (x = 0; x < imgWidth; x++)
      {
        if (pBitmapBits[x+y*imgWidth])
        {
          if (minx > x)
            minx = x;
          if (miny > y)
            miny = y;
          if (maxx < x)
            maxx = x;
          if (maxy < y)
            maxy = y;
        }
      }

    // Round char extents to charScale multiples.
    minx = ROUND_DOWN(minx, m_charScale);
    miny = ROUND_DOWN(miny, m_charScale);
    maxx = ROUND_UP  (maxx+1, m_charScale);
    maxy = ROUND_UP  (maxy+1, m_charScale);

    int sizeX = (maxx-minx)/m_charScale;
    int sizeY = (maxy-miny)/m_charScale;
    int logWidth = (size.cx+m_charScale-1)/m_charScale;

    // Scale down & store the pixmap.
    if (sizeX && sizeY)
    {
      // Generamos el bitmap final de la letra.
      byte *pPix = new byte[sizeX*sizeY];
      for (y = 0; y < sizeY; y++)
        for (x = 0; x < sizeX; x++)
        {
          int v = 0;
          for (int cy = 0; cy < m_charScale; cy++)
            for (int cx = 0; cx < m_charScale; cx++)
              v += (pBitmapBits[minx+x*m_charScale+cx + imgWidth*(miny+y*m_charScale+cy)] & 0xFF);
          v /= m_charScale*m_charScale;
          pPix[x + y*sizeX] = byte(v);
        }

      // Lo buscamos entre las letras anteriores, a ver si hay alguno igual.
      bool bFound = false;
      unsigned prevc;
      for (prevc = 0;
           !bFound && prevc < vecPosiciones.GetCount();
           ++prevc)
      {
        bFound = (vecPosiciones[prevc].w == sizeX && vecPosiciones[prevc].h == sizeY && 0 == memcmp(pPix, vecPosiciones[prevc].pPix, sizeof(DWORD)*sizeX*sizeY));
      }

      // Finalmente, lo metemos.
      vecPosiciones.Add(TLetter(i, 0, 0, sizeX, sizeY,
                                (minx-offsetChar)/m_charScale, (miny-offsetChar)/m_charScale,
                                logWidth, pPix,
                                bFound? prevc : -1));
    }
    else
      vecPosiciones.Add(TLetter(i, 0, 0, 0, 0, 0, 0, logWidth, NULL));
  }
  ::DeleteObject( hbmBitmap );
  ::DeleteDC    ( hDC );
  ::DeleteObject( hFont );

  GLOG(("\n"));
  return true;
}

// --------------------------------------------------------------------------------
// Nombre      : 
// Retorno     : 
// Descripcion : 
// --------------------------------------------------------------------------------
void CXtractedFont::CreateTextures(CLetterArray &vecPosiciones, CDisplayDevice *pDev)
{
  GLOG(("Colocando letras en las texturas...\n"));

  // Sort the chars based on area, bigger area first.
  std::sort(vecPosiciones.begin(), vecPosiciones.end(), CRectPlacement::TRect::Greater);

  // Try to fit letters in the space provided.
  // Greedy algo, start with small bag size and grow as needed.
  int maxTextures = 0;
  CRectPlacement aTextures[MAX_TEXTURES];

  for (CLetterArray::iterator it = vecPosiciones.begin();
       it != vecPosiciones.end();
       ++it)
  {
    if (it->prevChar == -1)
    {
      CRectPlacement::TRect r(0, 0, it->w+2, it->h+2);

      // If the char has actual space 
      bool bPlaced = false;
      for (int i = 0; !bPlaced && i < maxTextures; i++)
      {
        bPlaced = aTextures[i].AddAtEmptySpotAutoGrow(&r, MAX_TEXTURE_W, MAX_TEXTURE_H);
        if (bPlaced)
          it->nTex = i;
      }

      // Try starting a new texture
      if (!bPlaced && maxTextures < MAX_TEXTURES)
      {
        aTextures[maxTextures].Init();
        bPlaced = aTextures[maxTextures].AddAtEmptySpotAutoGrow(&r, MAX_TEXTURE_W, MAX_TEXTURE_H);
        if (bPlaced)
        {
          it->nTex = maxTextures;
          ++maxTextures;
        }
        else
          GLOG(("ERROR SERIO: NO HAY SITIO EN LAS TEXTURAS PARA EL CARACTER %d ('%c')\n", it->n, it->n));
      }

      // If correctly placed in a texture, store the coords-
      if (bPlaced)
      {
        it->x = r.x;
        it->y = r.y;
      }
    }
  }

  // Chars already added, now let's create the actual bitmaps.
  // --------------------------------------------------------
  
  m_paTextures = NEW_ARRAY(CDisplayTexture, maxTextures);
  m_nTextures  = maxTextures;
  int maxHeight = 0;
  TError ret = RET_OK;

  for (int tex = 0; ret == RET_OK && tex < maxTextures; tex++)
  {
    GLOG(("Texture # %2d, size is %dx%d\n", tex, aTextures[tex].GetW(), aTextures[tex].GetH()));

    int size = aTextures[tex].GetW()*aTextures[tex].GetH();
    CDisplayTexture *pTex = m_paTextures+tex;

    ret = pTex->Init("", aTextures[tex].GetW(), aTextures[tex].GetH(), D3DFMT_A8R8G8B8);
    if (RET_OK == ret)
      ret = pTex->Load(pDev);

    if (RET_OK == ret)
    {
      byte *pPix = NULL;
      int   pitch = 0;
      if (RET_OK != pTex->Lock(&pPix, &pitch))
        ret = RET_FAIL;
      else
      {
        for (int j = 0; j < aTextures[tex].GetH(); j++)
        {
          DWORD* pDst = POINTER_ADD_T(DWORD, pPix, pitch*j);
          ZeroMem(pDst, aTextures[tex].GetW()*sizeof(pDst));
        }

        // Find each letter's size (+ extra area)
        for (CLetterArray::iterator it = vecPosiciones.begin();
             it != vecPosiciones.end();
             ++it)
        {
          const TLetter &l = *it;
          if (l.prevChar == -1 && l.nTex == tex && l.w)
          {
            if (maxHeight < l.h)
              maxHeight = l.h;

            char str[2] = { (char)l.n, '\0' };

            GLOG(("."));

            // Copy char bits to final bitmap.
            for (int y = 0; y < l.h; y++)
            {
              DWORD *pDest = POINTER_ADD_T(DWORD, pPix, sizeof(DWORD)*(l.x+1) + pitch*(l.y+y+1));
              const byte *pSrc = l.pPix + l.w*y;

              for (int x = 0; x < l.w; x++)
                *pDest++ = 0x00FFFFFF | ((*pSrc++) << 24);
            }
          }
        }
        pTex->Unlock();
      }
    }
    GLOG(("\n"));
  }

  // Sort the charmap back to ASCII order for output.
  std::sort(vecPosiciones.begin(), vecPosiciones.end(), TLetter::LessIndex);

  // Create the rest of the data structures.
  m_paChars = NEW_ARRAY(TCharInfo, vecPosiciones.GetCount());
  m_nChars = vecPosiciones.GetCount();
  m_fontHeight = maxHeight+1;
  memset(m_aCharMap, -1, sizeof(m_aCharMap));

  // Fix cross-references for chars identical to previous chars.
  {
    int baseH = vecPosiciones['F'-32].h + vecPosiciones['F'-32].offsetY;

    int i = 0;
    for (CLetterArray::iterator it = vecPosiciones.begin();
         it != vecPosiciones.end();
         ++it, ++i)
    {
      if (it->prevChar != -1)
      {
        it->x = vecPosiciones[it->prevChar].x;
        it->y = vecPosiciones[it->prevChar].y;
        it->nTex = vecPosiciones[it->prevChar].nTex;
      }
      m_aCharMap[i+32] = i;
      m_paChars[i].x = it->x;
      m_paChars[i].y = it->y;
      m_paChars[i].w = it->w+1;
      m_paChars[i].h = it->h+1;
      m_paChars[i].logWidth = it->logWidth;
      m_paChars[i].nTex = it->nTex;
      m_paChars[i].offsX = it->offsetX;
      m_paChars[i].offsY = it->offsetY-baseH;
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CXtractedFont::Init (const char *pszName, int fontSize, int charScale, bool bBold, bool bItalic, CDisplayDevice *pDev)
{
  End();

  m_paChars = NULL;
  m_paTextures = NULL;
  if (!pszName[0])
    return RET_FAIL;

  int l = strlen(pszName);
  m_pszName = NEW_ARRAY(char, l+1);
  strcpy(m_pszName, pszName);
  m_fontSize  = fontSize;
  m_charScale = charScale;
  m_bBold     = bBold;
  m_bItalic   = bItalic;

  m_paChars     = NULL;
  m_paTextures  = NULL;
  m_nTextures = 0;
  m_nChars    = 0;

  m_Shader.Init();
  m_Shader.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
  m_Shader.GetPass(0)->DstBlend = D3DBLEND_INVSRCALPHA;
  m_Shader.GetPass(0)->ZWrite = false;
  m_Shader.GetPass(0)->ZFunc = D3DCMP_ALWAYS;

  Load(pDev);

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CXtractedFont::End         ()
{
  if (IsOk())
  {
    Unload();
    DISPOSE_ARRAY(m_pszName);
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CXtractedFont::Load      (CDisplayDevice *pDev)
{
  if (!IsOk())
    return RET_FAIL;

  TError ret = RET_OK;

  if (pDev && pDev->IsDeviceReady() && !IsLoaded())
  {
    CLetterArray vecPosiciones;
    vecPosiciones.Init(256);

    if (CreateLetters(vecPosiciones))
    {
      CreateTextures(vecPosiciones, pDev);
    }
    if (RET_OK != m_VB.Init(pDev, DV_FVF_TLVERTEXUV, 600, sizeof(TTLVertexUV)))
      return false;
  }
  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CXtractedFont::Unload ()
{
  if (IsOk())
  {
    m_VB.End();
    DISPOSE_ARRAY(m_paChars);
    DISPOSE_ARRAY(m_paTextures);
    m_nTextures = 0;
    m_nChars    = 0;
  }
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------

void CXtractedFont::Printf    (CRenderContext &rc, float x, float y, dword color, const char *pszFmt, ...) const
{
  char buf[1000];
  va_list vl;
  va_start(vl, pszFmt);
  vsprintf(buf, pszFmt, vl);
  Puts(rc, x, y, 1, 1, color, (color & 0xFF000000), buf);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CXtractedFont::Printf    (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszFmt, ...) const
{
  char buf[1000];
  va_list vl;
  va_start(vl, pszFmt);
  vsprintf(buf, pszFmt, vl);
  Puts(rc, x, y, sx, sy, color, shColor, buf);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CXtractedFont::Puts (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszStr) const
{
  if (!Color_A(color | shColor) || !IsLoaded())
    return;

  int nVertsPerChar = (Color_A(color)? 6 : 0) + (Color_A(shColor)? 6 : 0);
  float initX = x;

  CDisplayTexture *pPrevTex = NULL;
  bool bPendingSetDevice = true;

  float txf;
  float tyf;

  while (*pszStr)
  {
    unsigned c = (unsigned char)*pszStr++;

    if (c == '\n')
    {
      x = initX;
      y += sy*(m_fontHeight+1);
      continue;
    }

    int l = m_aCharMap[c];

    // Get char data, and skip char if not prontable.
    if (l == -1)
      continue;
    const TCharInfo &i = m_paChars[l];
    int cw = i.logWidth;
    if (!cw)
      continue;
    ASSERT(i.nTex < m_nTextures);
    
    CDisplayTexture *pTex = &m_paTextures[i.nTex];
    if (!pTex->IsLoaded())
      continue;

    int vstart = 0;
    TTLVertexUV *pv = (TTLVertexUV *)m_VB.Lock(nVertsPerChar, &vstart);
    if (pv)
    {
      if (pPrevTex != pTex)
      {
        m_Shader.GetStage(0, 0)->pTex = pTex;
        pPrevTex = pTex;
        m_Shader.Set(rc.GetDevice());

        txf = 1.f/pTex->GetWidth ();
        tyf = 1.f/pTex->GetHeight();
      }

      float ox = x + sx*float(i.offsX)+.0f;
      float oy = y + sy*float(i.offsY)+.0f;
      float oxw = ox + sx*i.w;
      float oyh = oy + sy*i.h;
      float o2x = ox + sx*1;
      float o2y = oy + sy*1;
      float o2xw = oxw + sx*1;
      float o2yh = oyh + sy*1;

      float tx0 = txf*(.5f+i.x    );
      float tx1 = txf*(.5f+i.x+i.w);
      float ty0 = tyf*(.5f+i.y    );
      float ty1 = tyf*(.5f+i.y+i.h);

      if (Color_A(shColor))
      {
        pv->Set(o2x , o2y , 0, 1, shColor, tx0, ty0);   pv++;
        pv->Set(o2xw, o2y , 0, 1, shColor, tx1, ty0);   pv++;
        pv->Set(o2x , o2yh, 0, 1, shColor, tx0, ty1);   pv++;
        *pv = pv[-1];   pv++;
        *pv = pv[-3];   pv++;
        pv->Set(o2xw, o2yh, 0, 1, shColor, tx1, ty1);   pv++;
      }

      if (Color_A(color))
      {
        pv->Set(ox , oy , 0, 1, color, tx0, ty0);   pv++;
        pv->Set(oxw, oy , 0, 1, color, tx1, ty0);   pv++;
        pv->Set(ox , oyh, 0, 1, color, tx0, ty1);   pv++;
        *pv = pv[-1];  pv++;
        *pv = pv[-3];  pv++;
        pv->Set(oxw, oyh, 0, 1, color, tx1, ty1);   pv++;
      }

      m_VB.Unlock();

      if (bPendingSetDevice)
      {
        rc.GetDevice()->SetFVFShader(m_VB.GetFVF());
        rc.GetDevice()->GetDirect3DDevice()->SetStreamSource(0, m_VB.GetVB(), 0, sizeof(*pv));
        bPendingSetDevice  = false;
      }
      rc.GetDevice()->GetDirect3DDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, vstart, nVertsPerChar/3);

      x += sx*(cw+1);
    }
  }
}
