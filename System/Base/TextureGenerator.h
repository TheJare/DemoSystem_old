// ----------------------------------------------------------------------------------------
// File:      TextureGenerator.h
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#ifndef _TEXTUREGENERATOR_H_
#define _TEXTUREGENERATOR_H_
#pragma once

#include "DynArray.h"

class CPerlin;

// --------------------

class CTextureGenerator
{
//    typedef __ Inherited;
  public:

    struct TTexture
    {
      unsigned w, h;
      byte *pPix;

      TTexture  (): pPix(NULL)    { }
      ~TTexture ()                { End(); }
      void Init (int w, int h)    { this->w = w; this->h = h; Alloc(); }
      void End  ()                { DISPOSE_ARRAY(pPix); }
      void Alloc()                { End(); pPix = NEW_ARRAY(byte , w*h); ZeroMem(pPix, w*h); }
      byte*GetPix (int y=0) const { return pPix + y*w; }
    };

    typedef unsigned EOp;
    enum
    {
      OP_SET,       // dest = src
      OP_ADD,       // dest = src + dest;
      OP_MUL,       // dest = src * dest;
      OP_SUB,       // dest = src - dest;

      OP_INV_SRC    = 0x04,  // invert 1st operand
      OP_INV_DEST   = 0x08,  // invert 2nd operand
      OP_REV        = 0x10,  // swap src & dest in operand
    };

    CTextureGenerator (): m_bOk(0)      { }
    ~CTextureGenerator()                { End(); }

    TError      Init        (unsigned numTextures = 1);
    void        End         ();
    bool        IsOk        ()                    const { return (m_bOk != 0); }

    const TTexture &
                GetTexture        (unsigned t)         const { ASSERT(t < m_aTextures.GetCount()); return m_aTextures[t]; }

    int         AddTexture        (int w, int h)        { m_aTextures.Add().Init(w, h); m_curTexture = m_aTextures.GetCount()-1; return m_curTexture; }
    int         AddTexture        (unsigned srcTex)     { ASSERT(srcTex < m_aTextures.GetCount()); m_aTextures.Add().Init(m_aTextures[srcTex].w, m_aTextures[srcTex].h); m_curTexture = m_aTextures.GetCount()-1; return m_curTexture; }
    int         AddRGBTexture     (unsigned srcTex)     { ASSERT(srcTex < m_aTextures.GetCount()); m_aTextures.Add().Init(m_aTextures[srcTex].w*4, m_aTextures[srcTex].h); m_curTexture = m_aTextures.GetCount()-1; return m_curTexture; }

    void        SetCurrentTexture (unsigned t)          { ASSERT(t < m_aTextures.GetCount()); m_curTexture = t; }
    void        SetTileOffset     (int x, int y)        { m_tileX = x; m_tileY = y; }
    void        SetOp             (EOp op)              { m_op = op; }

    void        Fill              (byte color);
    void        Perlin            (const CPerlin &Perlin, float fScale, int z = 0);
    void        Noise             (int dens, int scaleMin);
    void        Rect              (int x, int y, int w, int h, byte color);
    void        Ellipse           (int x, int y, int rx, int ry, byte color);
    void        Tile              (unsigned srcTex);
    void        Conv3             (const char mask[9], int scale, int offset);
    void        Conv5             (const char mask[25], int scale, int offset);
    void        Translate         (const byte xlat[256]);
    void        Blur              ();
    void        BlurMore          ();
    void        Emboss            ();
    void        IsoLevel          (int level, int width);
    void        BrightnessContrast(int brit, int cont);

    void        Replicate         (unsigned srcTex, unsigned mask = 0xFFFFFFFF, int sa = 0, int oa = 255, int sr = 256, int sg = 256, int sb = 256, int or = 0, int og = 0, int ob = 0);

// ------------
  private:

    byte                  Op    (byte src, byte dest) const;

    bool                  m_bOk;

    CDynArray<TTexture>   m_aTextures;

    int                   m_curTexture;
    int                   m_tileX;
    int                   m_tileY;
    EOp                   m_op;

};

#endif //_TEXTUREGENERATOR_H_
