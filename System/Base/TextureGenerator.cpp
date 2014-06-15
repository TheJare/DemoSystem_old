// ----------------------------------------------------------------------------------------
// File:      TextureGenerator.cpp
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#include "Base.h"
#include "TextureGenerator.h"
#include "Perlin.h"
#include <math.h>
#include "MathUtil.h"

// --------------------
// --------------------

namespace
{
  typedef byte (*FuncOp)(byte src, byte dest);

#define M_ADD(src,dest) Min(unsigned(src)+unsigned(dest), 255U)
#define M_MUL(src,dest) Min(unsigned(src)*unsigned(dest)/255, 255U)
#define M_SUB(src,dest) Max(int(src)-int(dest), 0)

  byte FuncSet(byte src, byte dest) { return src; }
  byte FuncAdd(byte src, byte dest) { return (byte)M_ADD(src,dest); }
  byte FuncMul(byte src, byte dest) { return (byte)M_MUL(src,dest); }
  byte FuncSub(byte src, byte dest) { return (byte)M_SUB(src,dest); }

  byte FuncInvSet(byte src, byte dest) { return ~src; }
  byte FuncInvAdd(byte src, byte dest) { return (byte)M_ADD(~src, dest); }
  byte FuncInvMul(byte src, byte dest) { return (byte)M_MUL(byte(~src), dest); }
  byte FuncInvSub(byte src, byte dest) { return (byte)M_SUB(~src, dest); }

  byte FuncSetInv(byte src, byte dest) { return src; }
  byte FuncAddInv(byte src, byte dest) { return (byte)M_ADD(src, ~dest); }
  byte FuncMulInv(byte src, byte dest) { return (byte)M_MUL(src, byte(~dest)); }
  byte FuncSubInv(byte src, byte dest) { return (byte)M_SUB(src, ~dest); }

  byte FuncInvSetInv(byte src, byte dest) { return ~src; }
  byte FuncInvAddInv(byte src, byte dest) { return (byte)M_ADD(~src, ~dest); }
  byte FuncInvMulInv(byte src, byte dest) { return (byte)M_MUL(byte(~src), byte(~dest)); }
  byte FuncInvSubInv(byte src, byte dest) { return (byte)M_SUB(~src, ~dest); }

  byte FuncRevSet(byte dest, byte src) { return src; }
  byte FuncRevAdd(byte dest, byte src) { return (byte)M_ADD(src,dest); }
  byte FuncRevMul(byte dest, byte src) { return (byte)M_MUL(src,dest); }
  byte FuncRevSub(byte dest, byte src) { return (byte)M_SUB(src,dest); }

  byte FuncRevInvSet(byte dest, byte src) { return ~src; }
  byte FuncRevInvAdd(byte dest, byte src) { return (byte)M_ADD(~src, dest); }
  byte FuncRevInvMul(byte dest, byte src) { return (byte)M_MUL(byte(~src), dest); }
  byte FuncRevInvSub(byte dest, byte src) { return (byte)M_SUB(~src, dest); }

  byte FuncRevSetInv(byte dest, byte src) { return src; }
  byte FuncRevAddInv(byte dest, byte src) { return (byte)M_ADD(src, ~dest); }
  byte FuncRevMulInv(byte dest, byte src) { return (byte)M_MUL(src, byte(~dest)); }
  byte FuncRevSubInv(byte dest, byte src) { return (byte)M_SUB(src, ~dest); }

  byte FuncRevInvSetInv(byte dest, byte src) { return ~src; }
  byte FuncRevInvAddInv(byte dest, byte src) { return (byte)M_ADD(~src, ~dest); }
  byte FuncRevInvMulInv(byte dest, byte src) { return (byte)M_MUL(byte(~src), byte(~dest)); }
  byte FuncRevInvSubInv(byte dest, byte src) { return (byte)M_SUB(~src, ~dest); }

  static FuncOp s_aFuncs[8*4] =
  {
    FuncSet,        FuncAdd,        FuncMul,        FuncSub,
    FuncInvSet,     FuncInvAdd,     FuncInvMul,     FuncInvSub,
    FuncSetInv,     FuncAddInv,     FuncMulInv,     FuncSubInv,
    FuncInvSetInv,  FuncInvAddInv,  FuncInvMulInv,  FuncInvSubInv,
    FuncRevSet,        FuncRevAdd,        FuncRevMul,        FuncRevSub,
    FuncRevInvSet,     FuncRevInvAdd,     FuncRevInvMul,     FuncRevInvSub,
    FuncRevSetInv,     FuncRevAddInv,     FuncRevMulInv,     FuncRevSubInv,
    FuncRevInvSetInv,  FuncRevInvAddInv,  FuncRevInvMulInv,  FuncRevInvSubInv,
  };

static const char s_aBlur9[9] =
{
  1, 3, 1,
  3, 3, 3,
  1, 3, 1
};

static const char s_aSquare25[25] =
{
  5, 5, 5, 5, 5,
  5, 0, 0, 0, 5,
  5, 0, 0, 0, 5,
  5, 0, 0, 0, 5,
  5, 5, 5, 5, 5,
};

static const char s_aFlat25[25] =
{
  5, 5, 5, 5, 5,
  5, 5, 5, 5, 5,
  5, 5, 5, 5, 5,
  5, 5, 5, 5, 5,
  5, 5, 5, 5, 5,
};

static const char s_aEmboss25[25] =
{
  1, 2, 1, 1, 0,
  2, 4, 2, 0,-1,
  1, 2, 0,-2,-1,
  1, 0,-2,-4,-2,
  0,-1,-1,-2,-1,
};
enum { EMBOSS_FACTOR = 30, EMBOSS_LEVEL = 128 };

static const char s_aBlur25[25] =
{
  0, 1, 2, 1, 0,
  1, 3, 4, 3, 1,
  2, 4, 5, 4, 2,
  1, 3, 4, 3, 1,
  0, 1, 2, 1, 0,
};

};


// --------------------
// --------------------


// ------------------------------------------------------------------------
// Name:       Init
// Purpose:    Initialize the object
// Parameters: 
// Returns:    RET_OK if successful, RET_FAIL otherwise
// ------------------------------------------------------------------------
TError CTextureGenerator::Init (unsigned numTextures)
{
  End();

  m_aTextures.Init(numTextures);

  m_curTexture  = 0;
  m_tileX       = 0;
  m_tileY       = 0;
  m_op          = OP_SET;

  m_bOk = true;
  return RET_OK;
}

// ------------------------------------------------------------------------
// Name:       End
// Purpose:    Deinitialize the object
// Parameters: none
// Returns:    none
// ------------------------------------------------------------------------
void CTextureGenerator::End ()
{
  if (IsOk())
  {
    m_aTextures.End();
    m_bOk = false;
  }
}

// ------------------------------------------------------------------------
// Name:       Op
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
byte CTextureGenerator::Op  (byte src, byte dest) const
{
  return s_aFuncs[int(m_op)](src, dest);
}

// ------------------------------------------------------------------------
// Name:       Fill
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Fill    (byte color)
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];
  byte *pPix = t.pPix;
  int tlen = t.w*t.h;
  if (m_op == OP_SET)
    memset(pPix, color, tlen);
  else
  {
    byte *pEnd = pPix + tlen;
    while (pPix < pEnd)
    {
      *pPix = func(color, *pPix);
      pPix++;
    }
  }
}

// ------------------------------------------------------------------------
// Name:       Rect
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Rect    (int x, int y, int w, int h, byte color)
{
  FuncOp func = s_aFuncs[int(m_op)];
  const TTexture &t = m_aTextures[m_curTexture];

  const int c = x+w-t.w;
  int l, pitch;
  if (c > 0)
  {
    pitch = x-c;
    l = w-c;
  }
  else
  {
    pitch = x;
    l = w;
  }
  while (h-- > 0)
  {
    byte *pPix = t.pPix + t.w*(y%t.h);

    int i = c;
    while (i-- > 0)
    {
      *pPix = func(color, *pPix);
      pPix++;
    }
    pPix += pitch;
    i = l;
    while (i-- > 0)
    {
      *pPix = func(color, *pPix);
      pPix++;
    }
    y++;
  }
}

// ------------------------------------------------------------------------
// Name:       Tile
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Tile (unsigned srcTex)
{
  ASSERT(srcTex < m_aTextures.GetCount());
  FuncOp func = s_aFuncs[int(m_op)];
  const TTexture &t = m_aTextures[m_curTexture];
  const TTexture &st = m_aTextures[srcTex];

  unsigned sx = unsigned(m_tileX+st.w) % st.w;
  unsigned sy = unsigned(m_tileY+st.h) % st.h;
  
  byte *pPix = t.pPix;
  unsigned tlen = t.w*t.h;
  byte *pEnd = pPix + tlen;

  if (sx == 0 && sy == 0 && tlen == st.w*st.h)
  {
    const byte *pSrc = st.pPix;
    while (pPix < pEnd)
    {
      *pPix = func(*pSrc, *pPix);
      pPix++;
      pSrc++;
    }
  }
  else
  {
    int h = t.h;
    int y = m_tileY;
    int initX = st.w-sx;

//    int initL = Min(t.w, sx);
//    int secL = t.w - initL;
    while (h-- > 0)
    {
      const byte *pSrc = st.pPix + st.w*(y%st.h);
      
      for (unsigned i = 0; i < t.w; i++)
        *pPix++ = func(pSrc[(i+initX)%st.w], *pPix);
/*
      const byte *pSrc2 = st.pPix + st.w*(y%st.h);
      const byte *pSrc  = pSrc2 + initX;
      int i = initL;
      while (i-- > 0)
      {
        *pPix = func(*pSrc, *pPix);
        pPix++;
        pSrc++;
      }
      i = secL;
      while (i-- > 0)
      {
        *pPix = func(*pSrc2, *pPix);
        pPix++;
        pSrc2++;
      }
*/
      y++;
    }
  }
}

// ------------------------------------------------------------------------
// Name:       Replicate  
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Replicate  (unsigned srcTex, unsigned mask, int sa, int oa, int sr, int sg, int sb, int or, int og, int ob)
{
  ASSERT(srcTex < m_aTextures.GetCount());
  const TTexture &t = m_aTextures[m_curTexture];
  const TTexture &st = m_aTextures[srcTex];
  int invMask = ~mask;
  ASSERT((t.w & 3) == 0);

  unsigned tw = t.w/4;
  int sx = (m_tileX+st.w) % st.w;
  int sy = (m_tileY+st.h) % st.h;
  
  int y = m_tileY;
  int initX = st.w-sx;

  int h = t.h;
  unsigned *pDest = (unsigned*)t.pPix;

  while (h-- > 0)
  {
    unsigned *pPix = pDest;
    const byte *pSrc = st.pPix + st.w*(y%st.h);
    
    for (unsigned i = 0; i < tw; i++)
    {
      unsigned c = 0;
      int v = pSrc[(i+initX)%st.w];
      if (mask & 0xFF000000)
        c |= unsigned(Clamp(sa*v/256+oa, 0, 255)) << 24;
      if (mask & 0x00FF0000)
        c |= unsigned(Clamp(sr*v/256+or, 0, 255)) << 16;
      if (mask & 0x0000FF00)
        c |= unsigned(Clamp(sg*v/256+og, 0, 255)) <<  8;
      if (mask & 0x000000FF)
        c |= unsigned(Clamp(sb*v/256+ob, 0, 255));

      if (invMask)
        *pPix = c | (*pPix & invMask);
      else
        *pPix = c;
      pPix++;
    }
    y++;
    pDest = POINTER_ADD_T(unsigned, pDest, t.w);
  }
}

// ------------------------------------------------------------------------
// Name:       Perlin
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Perlin  (const CPerlin &Perlin, float fScale, int z)
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];
  byte *pPix = t.pPix;
  float fScaleY = fScale/t.h*t.w;
  for (unsigned i = 0; i < t.h; i++)
    for (unsigned j = 0; j < t.w; j++)
    {
      int val = int(255.f/1.f*fabsf(Perlin.Fbm2(fScale*j, fScaleY*i, z, 4)*1.5f + 0.5f));
      byte v = (byte)Clamp(val, 0, 255);
      *pPix = func(v, *pPix);
      pPix++;
    }
}

// ------------------------------------------------------------------------
// Name:       Noise
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Noise  (int dens, int scaleMin)
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];
  byte *pPix = t.pPix;
  int tlen = t.w*t.h;
  byte *pEnd = pPix + tlen;
  int d2 = dens*2;
  while (pPix < pEnd)
  {
    byte v = (byte)Clamp(int((rand() % d2)-dens + scaleMin + *pPix), 0, 255);
    *pPix = func(v, *pPix);
    pPix++;
  }
}

// ------------------------------------------------------------------------
// Name:       Conv3
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Conv3  (const char mask[9], int scale, int offset)
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];

  if (scale == 0)
    for (int i = 0; i < 9; i++)
      scale += mask[i];
  if (scale == 0)
    scale = 1;

  TTexture temp;
  temp.Init(t.w+2, t.h+2);

  for (unsigned i = 0; i < temp.h; i++)
  {
    byte *pDest = temp.GetPix(i);
    const byte *pSrc = t.GetPix((i+t.w-1)%t.h);
    memcpy(pDest+1, pSrc, t.w);
    pDest[0] = pSrc[t.w-1];
    pDest[t.w+1] = pSrc[0];
  }

  for (unsigned i = 0; i < t.h; i++)
  {
    const byte *pS0 = temp.GetPix(i);
    const byte *pS1 = temp.GetPix(i+1);
    const byte *pS2 = temp.GetPix(i+2);
    byte *pPix = t.GetPix(i);
    byte *pEnd = pPix + t.w;
    
    while (pPix < pEnd)
    {
      byte v = (byte)Clamp(int((pS0[0]*mask[0] + pS0[1]*mask[1] + pS0[2]*mask[2]
                              + pS1[0]*mask[3] + pS1[1]*mask[4] + pS1[2]*mask[5]
                              + pS2[0]*mask[6] + pS2[1]*mask[7] + pS2[2]*mask[8])/scale + offset), 0, 255);
      *pPix = func(v, *pPix);
      pPix++;
      pS0++;
      pS1++;
      pS2++;
    }
  }
}

// ------------------------------------------------------------------------
// Name:       Conv5
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Conv5  (const char mask[25], int scale, int offset)
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];

  if (scale == 0)
    for (int i = 0; i < 25; i++)
      scale += mask[i];
  if (scale == 0)
    scale = 1;

  TTexture temp;
  temp.Init(t.w+4, t.h+4);

  for (unsigned i = 0; i < temp.h; i++)
  {
    byte *pDest = temp.GetPix(i);
    const byte *pSrc = t.GetPix((i+t.w-4)%t.h);
    memcpy(pDest+2, pSrc, t.w);
    pDest[0] = pSrc[t.w-2];
    pDest[1] = pSrc[t.w-1];
    pDest[t.w+2] = pSrc[0];
    pDest[t.w+3] = pSrc[1];
  }

  int k, l;
  for (unsigned i = 0; i < t.h; i++)
  {
    const byte *pS[5];
    for (k = 0; k < 5; k++)
      pS[k] = temp.GetPix(i+k);
    byte *pPix = t.GetPix(i);
    byte *pEnd = pPix + t.w;
    
    while (pPix < pEnd)
    {
      int val = 0;

      const char *pm = mask;
      for (k = 0; k < 5; k++)
        for (l = 0; l < 5; l++, pm++)
          val += int(pS[k][l])*(*pm);
      byte v = (byte)Clamp(int(val/scale+offset), 0, 255);

      *pPix = func(v, *pPix);
      pPix++;
      for (k = 0; k < 5; k++)
        pS[k]++;
    }
  }
}

// ------------------------------------------------------------------------
// Name:       Translate
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Translate (const byte xlat[256])
{
  FuncOp func = s_aFuncs[int(m_op)];

  const TTexture &t = m_aTextures[m_curTexture];
  byte *pPix = t.pPix;
  int tlen = t.w*t.h;
  byte *pEnd = pPix + tlen;
  while (pPix < pEnd)
  {
    *pPix = func(xlat[*pPix], *pPix);
    pPix++;
  }
}

// ------------------------------------------------------------------------
// Name:       Blur
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Blur  ()
{
  Conv3(s_aBlur9, 0, 0);
}

// ------------------------------------------------------------------------
// Name:       BlurMore
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::BlurMore  ()
{
  Conv5(s_aBlur25, 0, 0);
}

// ------------------------------------------------------------------------
// Name:       
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::Emboss  ()
{
  Conv5(s_aEmboss25, EMBOSS_FACTOR, EMBOSS_LEVEL);
}

// ------------------------------------------------------------------------
// Name:       IsoLevel
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::IsoLevel (int level, int width)
{
  byte xlat[256];

  for (int i = 0; i < 256; i++)
  {
    int k = Min(abs(i - level + width), width);
    float f = 1.f - float(k)/width;
    xlat[i] = byte(255.f*MathUtil::SFunc(f));
  }
  Translate(xlat);
}

// ------------------------------------------------------------------------
// Name:       IsoLevel
// Purpose:    
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void  CTextureGenerator::BrightnessContrast (int brit, int cont)
{
  byte xlat[256];

  for (int i = 0; i < 256; i++)
  {
    int k = 128+(i-128+brit)*255/cont;
    xlat[i] = byte(Clamp(k, 0, 255));
  }
  Translate(xlat);
}
