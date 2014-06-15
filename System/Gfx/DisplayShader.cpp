// -------------------------------------------------------------------------------------
// File:        DisplayShader.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"
#include "DisplayShader.h"
#include "DisplayTexture.h"
#include "StrUtil.h"

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayShader::Init      (const char *pszName)
{
  End();

  StrUtil::SafeStrncpy(m_szName, pszName, sizeof(m_szName));
  m_nPasses = 1;

  for (int i = 0; i < MAX_PASSES; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      m_aPasses[i].Stages[j].ColorArg1 = D3DTA_CURRENT;
      m_aPasses[i].Stages[j].ColorArg2 = D3DTA_TEXTURE;
      m_aPasses[i].Stages[j].ColorOp   = (j == 0)? D3DTOP_MODULATE : D3DTOP_DISABLE;
      m_aPasses[i].Stages[j].AlphaArg1 = D3DTA_CURRENT;
      m_aPasses[i].Stages[j].AlphaArg2 = D3DTA_TEXTURE;
      m_aPasses[i].Stages[j].AlphaOp   = (j == 0)? D3DTOP_MODULATE : D3DTOP_DISABLE;
      m_aPasses[i].Stages[j].bClamp    = false;

      m_aPasses[i].Stages[j].TexGenMode = j| D3DTSS_TCI_PASSTHRU;
      m_aPasses[i].Stages[j].pTex = NULL;
    }

    m_aPasses[i].FillMode   = D3DFILL_SOLID;
    m_aPasses[i].SrcBlend   = D3DBLEND_ONE;
    m_aPasses[i].DstBlend   = D3DBLEND_ZERO;
    m_aPasses[i].ZFunc      = D3DCMP_LESSEQUAL;
    m_aPasses[i].ZWrite     = TRUE;
    m_aPasses[i].AlphaFunc  = D3DCMP_ALWAYS;
    m_aPasses[i].AlphaRef   = 128;
    m_aPasses[i].CullMode   = D3DCULL_CCW;
    m_aPasses[i].Lighting   = FALSE;
    m_aPasses[i].ShadeMode  = D3DSHADE_GOURAUD;
    m_aPasses[i].TFactor    = 0xFFFFFFFF;
  }

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
static D3DMATRIX s_TextureMatrix =
{
  {
    .5f,   0,   0,   0,
      0,-.5f,   0,   0,
      0,   0,   0,   0,
    .0f, .5f,   0,   0,
  }
};

void CDisplayShader::Set(CDisplayDevice *pDev, int nPass) const
{
  const TPass &pass = m_aPasses[nPass];
  int i;
  for (i = 0; i < 2; i++)
  {
    const TStage &st = pass.Stages[i];
    pDev->SetTSS(i, D3DTSS_COLORARG1, st.ColorArg1);
    pDev->SetTSS(i, D3DTSS_COLORARG2, st.ColorArg2);
    pDev->SetTSS(i, D3DTSS_ALPHAARG1, st.AlphaArg1);
    pDev->SetTSS(i, D3DTSS_ALPHAARG2, st.AlphaArg2);
    pDev->SetTSS(i, D3DTSS_COLOROP,   st.ColorOp);
    pDev->SetTSS(i, D3DTSS_ALPHAOP,   st.AlphaOp);
    pDev->SetTSS(i, D3DTSS_TEXCOORDINDEX,         st.TexGenMode);
    pDev->SetSS(i, D3DSAMP_ADDRESSU,         st.bClamp? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
    pDev->SetSS(i, D3DSAMP_ADDRESSV,         st.bClamp? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);

    // Esto esta algo chapucero aqui, pero que se le va a hacer...
    if ((st.TexGenMode & ~7) == D3DTSS_TCI_CAMERASPACENORMAL || (st.TexGenMode & ~7) == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
    {
      if (pDev->GetCachedTSS(i, D3DTSS_TEXTURETRANSFORMFLAGS) != D3DTTFF_COUNT2)
      {
        pDev->SetTSS(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        pDev->GetDirect3DDevice()->SetTransform(D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+i), &s_TextureMatrix);
      }
    }
    else
      pDev->SetTSS(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

    pDev->SetTexture(i, st.pTex? st.pTex->GetTexture() : NULL);
  }
  pDev->SetTSS(i, D3DTSS_COLOROP,   D3DTOP_DISABLE);
  pDev->SetTSS(i, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

  pDev->SetRS(D3DRS_FILLMODE,         pass.FillMode);

  pDev->SetRS(D3DRS_ALPHABLENDENABLE, (pass.SrcBlend != D3DBLEND_ONE || pass.DstBlend != D3DBLEND_ZERO));
  pDev->SetRS(D3DRS_SRCBLEND,         pass.SrcBlend);
  pDev->SetRS(D3DRS_DESTBLEND,        pass.DstBlend);

  pDev->SetRS(D3DRS_ZFUNC,            pass.ZFunc);
  pDev->SetRS(D3DRS_ZWRITEENABLE,     pass.ZWrite);

  pDev->SetRS(D3DRS_ALPHATESTENABLE,  pass.AlphaFunc != D3DCMP_ALWAYS);
  pDev->SetRS(D3DRS_ALPHAFUNC,        pass.AlphaFunc);
  pDev->SetRS(D3DRS_ALPHAREF,         pass.AlphaRef);

  pDev->SetRS(D3DRS_CULLMODE,         pass.CullMode);
  pDev->SetRS(D3DRS_LIGHTING,         pass.Lighting);
  pDev->SetRS(D3DRS_SHADEMODE,        pass.ShadeMode);
  pDev->SetRS(D3DRS_TEXTUREFACTOR,    pass.TFactor);
}
