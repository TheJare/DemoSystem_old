//----------------------------------------------------------------------------
//  Nombre:    DE_ImagePanel.cpp
//
//  Contenido: Efecto para pintar paneles de imagen
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DE_ImagePanel.h"
#include "DemoEffectManager.h"
#include "DisplayTexture.h"
#include "TextureManager.h"
#include "RenderContext.h"

#include "StrUtil.h"

EFFECTCLASS_FNDEFINE(ImagePanel)

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TError CDE_ImagePanel::Init(CDemoEffectManager *pManager, const char *pszName, const char *pszArg)
{
  End();

  char img[1000];

  TError ret = RET_FAIL;

  if (StrUtil::GetToken(img, sizeof(img), pszArg))
  {
    m_color   = (dword)StrUtil::GetInt(pszArg);
    ret = RET_OK;
  }

  if (RET_OK == ret)
  {
    m_pImage   = NULL;
    ret = inherited::Init(pManager, pszName, pszArg);
  }
  if (RET_OK == ret)
  {
    if (img[0])
    {
      m_pImage = m_pManager->GetTextureManager()->InitMaterial(img, 0);
      if (!m_pImage)
        ret = RET_FAIL;
      else
        m_pImage->Load(m_pManager->GetDisplayDevice());
    }
  }
  if (RET_OK == ret)
  {
    m_Shader.Init();
    m_Shader.GetStage(0, 0)->pTex = m_pImage;
    m_Shader.GetStage(0, 0)->bClamp = true;
    m_Shader.GetPass(0)->ZWrite = false;
    m_Shader.GetPass(0)->ZFunc = D3DCMP_ALWAYS;
  }

  if (ret != RET_OK)
    End();

  return ret;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_ImagePanel::End()
{
  if (IsOk())
  {
    m_pManager->GetTextureManager()->ReleaseMaterial(m_pImage);
    inherited::End();
  }
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
/*
TError CDE_ImagePanel::Command(const char *pszCommand, const char *pszArg)
{
  return inherited::Command(pszCommand, pszArg);
}
*/

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_ImagePanel::Draw(CRenderContext &rc, float x, float y, float sx, float sy, dword c) const
{
  dword ct = MakeColor(Color_R(c)*Color_R(m_color)/255,
                       Color_G(c)*Color_G(m_color)/255,
                       Color_B(c)*Color_B(m_color)/255,
                       Color_A(c)*Color_A(m_color)/255);

  if (Color_A(ct) == 0)
    return;

  if (m_bAdd)
  {
    m_Shader.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
    m_Shader.GetPass(0)->DstBlend = D3DBLEND_ONE;
  }
  else if (m_pImage || Color_A(ct) < 255)
  {
    m_Shader.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
    m_Shader.GetPass(0)->DstBlend = D3DBLEND_INVSRCALPHA;
  }
  else
  {
    m_Shader.GetPass(0)->SrcBlend = D3DBLEND_ONE;
    m_Shader.GetPass(0)->DstBlend = D3DBLEND_ZERO;
  }

  if (m_pImage)
    rc.FillTexRect(x, y, sx*m_pImage->GetWidth(), sy*m_pImage->GetHeight(), ct, &m_Shader);
  else
    rc.FillTexRect(x, y, sx, sy, ct, &m_Shader);
}
