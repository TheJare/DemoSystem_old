//----------------------------------------------------------------------------
//  Nombre:    DE_BasePanel.cpp
//
//  Contenido: Base de efecto para pintar paneles (texto o img)
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DE_BasePanel.h"
#include "DemoEffectManager.h"

#include "StrUtil.h"
#include "vectors.h"

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TError CDE_BasePanel::Init(CDemoEffectManager *pManager, const char *pszName, const char *pszArg)
{
  End();

  ZeroMem(&m_actual, sizeof(m_actual));
  ZeroMem(&m_from, sizeof(m_from));
  ZeroMem(&m_to, sizeof(m_to));
  ZeroMem(&m_vibrato, sizeof(m_vibrato));
  ZeroMem(&m_speed, sizeof(m_speed));
  ZeroMem(&m_accel, sizeof(m_accel));
  m_to.sx = m_to.sy = 1;
  m_from.sx = m_from.sy = 1;
  m_actual.sx = m_actual.sy = 1;
  
  m_bMonoVib  = false;
  m_bAdd      = false;
  m_offMask   = 0;
  
  m_total = 0;
  m_pos   = 0;

  TError ret = Command("SETALL", pszArg);
  m_actual = m_to;
  if (ret == RET_OK)
    return inherited::Init(pManager, pszName, pszArg);

  // Something went wrong.
  return RET_FAIL;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_BasePanel::End()
{
  if (IsOk())
    inherited::End();
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TError CDE_BasePanel::Command(const char *pszCommand, const char *pszArg)
{
  enum
  {
    F_SETPOS = 1,
    F_SETRGB = 2,
    F_SETVIB = 4,
    F_SETMONOVIB = 8,
    F_SETADD     = 16,
    F_SETSPEED   = 32,
    F_SETACCEL   = 64,
    F_OFFMASK    = 128,
    F_SETSCALE   = 256,
    F_SETSCALETO = 512,
  };

  unsigned flags = 0;
  if (stricmp(pszCommand, "SETALL") == 0)
    flags = F_SETPOS | F_SETRGB | F_SETADD;
  else if (stricmp(pszCommand, "SETPOS") == 0)
    flags = F_SETPOS;
  else if (stricmp(pszCommand, "SETSCALETO") == 0)
    flags = F_SETSCALETO;
  else if (stricmp(pszCommand, "SETSCALE") == 0)
    flags = F_SETSCALE;
  else if (stricmp(pszCommand, "SETRGB") == 0)
    flags = F_SETRGB;
  else if (stricmp(pszCommand, "SETVIB") == 0)
    flags = F_SETVIB;
  else if (stricmp(pszCommand, "SETADD") == 0)
    flags = F_SETADD;
  else if (stricmp(pszCommand, "SETSPEED") == 0)
    flags = F_SETSPEED;
  else if (stricmp(pszCommand, "SETACCEL") == 0)
    flags = F_SETACCEL;
  else if (stricmp(pszCommand, "SETMONOVIB") == 0)
    flags = F_SETMONOVIB;
  else if (stricmp(pszCommand, "OFFMASK") == 0)
    flags = F_OFFMASK;
  else
    return RET_FAIL;

  if (flags & F_OFFMASK)
  {
    m_offMask = StrUtil::GetInt(pszArg);
  }
  if (flags & F_SETADD)
  {
    m_bAdd = (StrUtil::GetInt(pszArg) != 0);
  }
  if (flags & (F_SETPOS | F_SETRGB | F_SETSCALE))
  {
    m_from      = m_actual;
    m_total     = (unsigned)StrUtil::GetInt(pszArg);
    m_pos       = 0;
  }
  if (flags & F_SETPOS)
  {
    m_to.x = StrUtil::GetFloat(pszArg);
    m_to.y = StrUtil::GetFloat(pszArg);
  }
  if (flags & F_SETSCALE)
  {
    m_to.sx = StrUtil::GetFloat(pszArg);
    m_to.sy = StrUtil::GetFloat(pszArg);
  }
  if (flags & F_SETRGB)
  {
    m_to.r = StrUtil::GetFloat(pszArg);
    m_to.g = StrUtil::GetFloat(pszArg);
    m_to.b = StrUtil::GetFloat(pszArg);
    m_to.a = StrUtil::GetFloat(pszArg);
  }
  if (flags & (F_SETVIB | F_SETMONOVIB))
  {
    m_vibrato.x = StrUtil::GetFloat(pszArg);
    m_vibrato.y = StrUtil::GetFloat(pszArg);
    if (flags & F_SETVIB)
    {
      m_bMonoVib = false;
      m_vibrato.r = StrUtil::GetFloat(pszArg);
      m_vibrato.g = StrUtil::GetFloat(pszArg);
      m_vibrato.a = StrUtil::GetFloat(pszArg);
      m_vibrato.b = StrUtil::GetFloat(pszArg);
    }
    if (flags & F_SETMONOVIB)
    {
      m_bMonoVib = true;
      m_vibrato.r = StrUtil::GetFloat(pszArg);
    }
    m_vibrato.a = StrUtil::GetFloat(pszArg);
  }
  if (flags & F_SETSPEED)
  {
    m_speed.x = StrUtil::GetFloat(pszArg);
    m_speed.y = StrUtil::GetFloat(pszArg);
  }
  if (flags & F_SETACCEL)
  {
    m_accel.x = StrUtil::GetFloat(pszArg);
    m_accel.y = StrUtil::GetFloat(pszArg);
  }
  return RET_OK;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_BasePanel::Run(uint msThisFrame)
{
  if (!IsPlaying())
    return;
  inherited::Run(msThisFrame);

  // Calcular velocidad y aceleracion.
  // Para que sea totalmente independiente del framerate tenemos que integrar
  {
    float f = float(msThisFrame);

    float fx = m_speed.x*f;
    float fy = m_speed.y*f;
    float sx = fx + .5f*m_accel.x*f*f;
    float sy = fy + .5f*m_accel.y*f*f;
    m_speed.x += m_accel.x*f;
    m_speed.y += m_accel.y*f;

    m_to.x   += sx;
    m_from.x += sx;
    m_to.y   += sy;
    m_from.y += sy;
  }

  m_pos += msThisFrame;
  if (m_pos >= m_total)
  {
    m_actual = m_to;
    m_total = 0;
    m_pos   = 0;
  }
  else
  {
    float f = float(m_pos)/m_total;
    m_actual.x = Lerp(m_from.x, m_to.x, f);
    m_actual.y = Lerp(m_from.y, m_to.y, f);
    m_actual.sx = Lerp(m_from.sx, m_to.sx, f);
    m_actual.sy = Lerp(m_from.sy, m_to.sy, f);
    m_actual.a = Lerp(m_from.a, m_to.a, f);
    m_actual.r = Lerp(m_from.r, m_to.r, f);
    m_actual.g = Lerp(m_from.g, m_to.g, f);
    m_actual.b = Lerp(m_from.b, m_to.b, f);
  }

  m_actual.x += m_vibrato.x * float(rand() - RAND_MAX/2) / RAND_MAX;
  m_actual.y += m_vibrato.y * float(rand() - RAND_MAX/2) / RAND_MAX;
  m_actual.a += m_vibrato.a * float(rand() - RAND_MAX/2) / RAND_MAX;
  if (m_bMonoVib)
  {
    float f = m_vibrato.r * float(rand() - RAND_MAX/2) / RAND_MAX;
    m_actual.r += f;
    m_actual.g += f;
    m_actual.b += f;
  }
  else
  {
    m_actual.r += m_vibrato.r * float(rand() - RAND_MAX/2) / RAND_MAX;
    m_actual.g += m_vibrato.g * float(rand() - RAND_MAX/2) / RAND_MAX;
    m_actual.b += m_vibrato.b * float(rand() - RAND_MAX/2) / RAND_MAX;
  }

//  m_actual.x = float(int(m_actual.x));
//  m_actual.y = float(int(m_actual.y));
  m_actual.a = Clamp(m_actual.a, 0.f, 255.f);
  m_actual.r = Clamp(m_actual.r, 0.f, 255.f);
  m_actual.g = Clamp(m_actual.g, 0.f, 255.f);
  m_actual.b = Clamp(m_actual.b, 0.f, 255.f);
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDE_BasePanel::Draw(CRenderContext &rc) const
{
  if (!IsPlaying())
    return;

  if (m_offMask != 0)
  {
    int pos;
    int row = 0;
    m_pManager->GetMusicPos(&pos, &row);
    if (row & m_offMask)
      return;
  }

  dword c = MakeColor(byte(m_actual.r), byte(m_actual.g), byte(m_actual.b), byte(m_actual.a));

  Draw(rc, m_actual.x, m_actual.y, m_actual.sx, m_actual.sy, c);
}
