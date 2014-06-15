//-----------------------------------------------------------------------------------------
//  Nombre:     ProjectionViewport.cpp
//
//  Contenido:  Proyecciones 3D
//-----------------------------------------------------------------------------------------

#include "GfxPCH.h"
#include "ProjectionViewport.h"
#include <math.h>

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void TProjectionViewport::Set(int x, int y, int w, int h,
                              float _ivpw, float _ivph,
                              float _zn, float _zf)
{
  if (_ivpw == 0.f)
    _ivpw = _ivph*h/w;
  zn   = _zn;
  zf   = _zf;
  ivpw = _ivpw;
  ivph = _ivph;
  vx = x; vy = y; vw = w; vh = h;

  float dz = 1.f / (_zf - _zn);
  float fw = float(w);
  float fh = float(h);
  Kx =  zn * fw * _ivpw;
  Ky = -zn * fh * _ivph;
  Kz = zf * dz;
  offX = float (x) + fw/2;
  offY = float (y) + fh/2;
  ZnKz = zn*Kz;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void TProjectionViewport::SetFOV  (float hFov)
{
  float _ivpw = 1.f/tanf(hFov/2.f);
  float _ivph = _ivpw/float(vh)*float(vw);
  Set(vx, vy, vw, vh, _ivpw, _ivph, zn, zf);
}
