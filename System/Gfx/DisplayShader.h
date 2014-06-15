// -------------------------------------------------------------------------------------
// File:        DisplayShader.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _DISPLAY_SHADER_H_
#define _DISPLAY_SHADER_H_

#include "DisplayDevice.h"

class CDisplayTexture;

/*
CDisplayShader
{
  TPass
  {
    TStage[2]
    {
      CDisplayTexture *pTex;
      // RGB op                 // [Default] = MODULATE, DISABLE
      // Alpha op               // [Default] = MODULATE, DISABLE
      // Clamp..................// [Default] = false
      // TexGenMode                 // [Default] = NO         // Reflection stuff or Matrix Generator
    }
    // FillMode   // [Default] = SOLID
    // Blend      // [Default] = ONE:ZERO
    // ZFunc      // [Default] = LESSEQUAL       // ALWAYS == disable
    // ZWrite     // [Default] = YES
    // Alphafunc  // [Default] = ALWAYS          // ALWAYS == disable
    // Alpharef   // [Default] = 128
    // Cull       // [Default] = CCW
    // Lighting   // [Default] = No
    // TFactor    // [Default] = 0xFFFFFFFF;
  }

  Init(nPasses, blendOp);
  int GetNPasses();
  TPass GetPass (int i);
}
*/

class CDisplayShader: public IDisplayMaterial
{
  public:
    struct TStage
    {
      CDisplayTexture *pTex;

      byte  ColorArg1, ColorArg2, ColorOp;
      byte  AlphaArg1, AlphaArg2, AlphaOp;
      bool  bClamp;
      dword  TexGenMode;

//      float TexGenParms[4];
    };
    struct TPass
    {
      TStage  Stages[2];

      byte    FillMode;
      byte    SrcBlend;
      byte    DstBlend;
      byte    ZFunc;
      byte    ZWrite;
      byte    AlphaFunc;
      byte    AlphaRef;
      byte    CullMode;
      byte    Lighting;
      byte    ShadeMode;
      dword   TFactor;
    };

    enum
    {
      MAX_PASSES  = 4,
      MAX_NAME    = 32,
    };

    CDisplayShader():     m_nPasses(0)    { }
    ~CDisplayShader()                     { End(); }

    TError      Init      (const char *pszName = "");
    void        End       ()                { m_nPasses = 0; }
    bool        IsOk      ()          const { return m_nPasses != 0; }

    TPass      *GetPass   (int i)           { ASSERT(i >= 0 && i < m_nPasses); return m_aPasses + i; }
    TStage     *GetStage  (int i, int st)   { ASSERT(i >= 0 && i < m_nPasses); ASSERT(st >= 0 && st < 2); return m_aPasses[i].Stages + st; }

    void          SetNPasses(int n)           { ASSERT(n >= 1 && n <= MAX_PASSES); m_nPasses = n; }

    virtual int   GetNPasses()          const { return m_nPasses; }
    virtual void  Set       (CDisplayDevice *, int nPass = 0) const;

  private:
    char        m_szName[MAX_NAME];
    int         m_nPasses;
    TPass       m_aPasses[MAX_PASSES];
};

#endif // _DISPLAY_SHADER_H_
