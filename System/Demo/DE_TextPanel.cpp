//----------------------------------------------------------------------------
//  Nombre:    DE_TextPanel.cpp
//
//  Contenido: Efecto para pintar paneles de texto
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DE_TextPanel.h"
#include "DemoEffectManager.h"
#include "DisplayFont.h"
#include "FontManager.h"

#include "StrUtil.h"

EFFECTCLASS_FNDEFINE(TextPanel)

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TError CDE_TextPanel::Init(CDemoEffectManager *pManager, const char *pszName, const char *pszArg)
{
  End();

  char font[1000];
  char text[1000];

  TError ret = RET_FAIL;

  if (StrUtil::GetToken(font, sizeof(font), pszArg))
    if (StrUtil::GetToken(text, sizeof(text), pszArg))
    {
      m_color   = (dword)StrUtil::GetInt(pszArg);
      m_shColor = (dword)StrUtil::GetInt(pszArg);
      ret = RET_OK;
    }

  if (RET_OK == ret)
  {
    m_pFont   = NULL;
    m_pszText = NULL;
    ret = inherited::Init(pManager, pszName, pszArg);
  }
  if (RET_OK == ret)
  {
    m_pFont = m_pManager->GetFontManager()->InitFont(font, 0);
    if (!m_pFont)
      ret = RET_FAIL;
    else
    {
      int l = strlen(text);
      m_pszText = NEW_ARRAY(char, l+1);
      strcpy(m_pszText, text);
    }
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
void CDE_TextPanel::End()
{
  if (IsOk())
  {
    DISPOSE(m_pszText);
    m_pManager->GetFontManager()->ReleaseFont(m_pFont);
    inherited::End();
  }
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
/*
TError CDE_TextPanel::Command(const char *pszCommand, const char *pszArg)
{
  return inherited::Command(pszCommand, pszArg);
}
*/

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_TextPanel::Draw(CRenderContext &rc, float x, float y, float sx, float sy, dword c) const
{
  if (Color_A(c) == 0)
    return;

  dword ct = MakeColor(Color_R(c)*Color_R(m_color)/255,
                       Color_G(c)*Color_G(m_color)/255,
                       Color_B(c)*Color_B(m_color)/255,
                       Color_A(c)*Color_A(m_color)/255);
  dword cs = MakeColor(Color_R(c)*Color_R(m_shColor)/255,
                       Color_G(c)*Color_G(m_shColor)/255,
                       Color_B(c)*Color_B(m_shColor)/255,
                       Color_A(c)*Color_A(m_shColor)/255);

  m_pFont->Puts(rc, x, y, sx, sy, ct, cs, m_pszText);
}
