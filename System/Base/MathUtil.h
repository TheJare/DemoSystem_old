// ----------------------------------------------------------------------------------------
// File:      MathUtil.h
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#ifndef _MATHUTIL_H_
#define _MATHUTIL_H_
#pragma once

#include <math.h>

// --------------------

#define MATHUTIL_PI 3.1415926535897932384626433832795f

namespace MathUtil
{
  // Funcion S, con S(0) = 0, S(1) = 1, S'(0) = S'(1) = 0
  inline float SFunc    (float t)             { return t * t * (3.f - 2.f * t); }

  // Valor aleatorio 0..1
  inline float Randf    ()                    { return float(rand())*(1.f/RAND_MAX); }

  // Valor aleatorio -1..1
  inline float Random   ()                    { return float(rand())*(2.f/RAND_MAX) - 1.f; }

  // Valor aleatorio 0-2*PI
  inline float RandomAng()                    { return float(rand())*(MATHUTIL_PI*2/RAND_MAX); }

  // Vectores unitarios aleatorios;
  inline float Random1D ()                    { return (rand() & 1)? -1.f : 1.f; }
  inline void  Random2D (float &x, float &y)  { float a = RandomAng(); x = cosf(a); y = sinf(a); }
  inline void  Random3D (float &x, float &y, float &z)
                                              { float a = RandomAng(); float b = RandomAng()/2.f; float sb = sinf(b); x = cosf(a)*sb; y = sinf(a)*sb; z = cosf(b); }


}

#endif //_MATHUTIL_H_
