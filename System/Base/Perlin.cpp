 // -------------------------------------------------------------------------------------
// File:        Perlin.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "Base.h"
#include "Perlin.h"
#include "MathUtil.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object
// Parameters:    
// --------------------------------------------------------------------------
TError CPerlin::Init  ()
{
  if (!IsOk())
  {
    int i;
    for (i = 0; i < ARRAY_LEN(m_afNoise); i++)
      MathUtil::Random3D(m_afNoise[i][0], m_afNoise[i][1], m_afNoise[i][2]);
    for (i = 0; i < FNOISE_SIZE; i++)
      m_aPerm[i] = i;
    for (i = 0; i < FNOISE_SIZE; i++)
      Swap(m_aPerm[i], m_aPerm[rand()%FNOISE_SIZE]);
    for (i = 0; i < FNOISE_SIZE; i++)
      m_aPerm[i+FNOISE_SIZE] = m_aPerm[i];
  }
  return RET_OK;
}


// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Release all resources and deinitialize the object
// Parameters:    
// --------------------------------------------------------------------------
void CPerlin::End  ()
{
  m_bOk = false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Noise2  (float x, float y, int z) const
{
  int bx0, bx1, by0, by1, b00, b10, b01, b11;
  float rx0, rx1, ry0, ry1, sx, sy, a, b, t, u, v;
  const float *q;
  int i, j;

#define at2(rx,ry)	( rx * q[0] + ry * q[1] )
#define setup2(v,b0,b1,r0,r1) \
  t = v + 10000.f; \
  b0 = ((int)t) & FNOISE_MASK; \
  b1 = (b0+1) & FNOISE_MASK; \
  r0 = t - (int)t; \
  r1 = r0 - 1.f;

  setup2(x, bx0,bx1, rx0,rx1);
  setup2(y, by0,by1, ry0,ry1);
  z = z & FNOISE_MASK;

  i = m_aPerm[ bx0 ];
  j = m_aPerm[ bx1 ];

  b00 = m_aPerm[ i + by0 ];
  b10 = m_aPerm[ j + by0 ];
  b01 = m_aPerm[ i + by1 ];
  b11 = m_aPerm[ j + by1 ];

  sx = MathUtil::SFunc(rx0);
  sy = MathUtil::SFunc(ry0);

  q = m_afNoise[ b00 + z ] ; u = at2(rx0,ry0);
  q = m_afNoise[ b10 + z ] ; v = at2(rx1,ry0);
  a = Lerp(u, v, sx);

  q = m_afNoise[ b01 + z ] ; u = at2(rx0,ry1);
  q = m_afNoise[ b11 + z ] ; v = at2(rx1,ry1);
  b = Lerp(u, v, sx);

  return Lerp(a, b, sy);           // interpolate in y at lo x
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Noise3  (float x, float y, float z) const
{
  int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
  float rx0, rx1, ry0, ry1, rz0, rz1, sx, sy, sz, a, b, c, d, t, u, v;
  const float *q;
  int i, j;

#define at3(rx,ry,rz)	( rx * q[0] + ry * q[1] + rz * q[2] )
#define setup3(v,b0,b1,r0,r1) \
  t = v + 10000.f; \
  b0 = ((int)t) & FNOISE_MASK; \
  b1 = (b0+1) & FNOISE_MASK; \
  r0 = t - (int)t; \
  r1 = r0 - 1.f;

  setup3(x, bx0,bx1, rx0,rx1);
  setup3(y, by0,by1, ry0,ry1);
  setup3(z, bz0,bz1, rz0,rz1);

  i = m_aPerm[ bx0 ];
  j = m_aPerm[ bx1 ];

  b00 = m_aPerm[ i + by0 ];
  b10 = m_aPerm[ j + by0 ];
  b01 = m_aPerm[ i + by1 ];
  b11 = m_aPerm[ j + by1 ];

  sx = MathUtil::SFunc(rx0);
  sy = MathUtil::SFunc(ry0);
  sz = MathUtil::SFunc(rz0);

  q = m_afNoise[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
  q = m_afNoise[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
  a = Lerp(u, v, sx);

  q = m_afNoise[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
  q = m_afNoise[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
  b = Lerp(u, v, sx);

  c = Lerp(a, b, sy);           // interpolate in y at lo x

  q = m_afNoise[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
  q = m_afNoise[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
  a = Lerp(u, v, sx);

  q = m_afNoise[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
  q = m_afNoise[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
  b = Lerp(u, v, sx);

  d = Lerp(a, b, sy);           // interpolate in y at hi x

  return Lerp(c, d, sz);        // interpolate in z
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Turbulence2(float x, float y, int z, float fOctaves, float fExpFac) const
{
  float fValue = 0.0f;
  float fExp = 1.f;
  int iOctaves = int(fOctaves);

  for (int i = 0; i < iOctaves; i++)
  {
     fValue += fabsf(Noise2(x, y, z)) * fExp;// Sum weighted noise value
     x *= 2; y *= 2;// z *= 2;
     fExp *= fExpFac;
  }

  // Take care of the fractional part of fOctaves
  fOctaves -= float(iOctaves);
  if  (fOctaves > 0.0f)
    fValue += fOctaves * fabsf(Noise2(x, y, z)) * fExp;

  return fValue;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Turbulence3(float x, float y, float z, float fOctaves, float fExpFac) const
{
  float fValue = 0.0f;
  float fExp = 1.f;
  int iOctaves = int(fOctaves);

  for (int i = 0; i < iOctaves; i++)
  {
     fValue += fabsf(Noise3(x, y, z)) * fExp;// Sum weighted noise value
     x *= 2; y *= 2; z *= 2;
     fExp *= fExpFac;
  }

  // Take care of the fractional part of fOctaves
  fOctaves -= float(iOctaves);
  if  (fOctaves > 0.0f)
    fValue += fOctaves * fabsf(Noise3(x, y, z)) * fExp;

  return fValue;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Fbm2(float x, float y, int z, float fOctaves, float fExpFac) const
{
  float fValue = 0.0f;
  float fExp = 1.f;
  int iOctaves = int(fOctaves);

  for (int i = 0; i < iOctaves; i++)
  {
     fValue += Noise2(x, y, z) * fExp;// Sum weighted noise value
     x *= 2; y *= 2;// z *= 2;
     fExp *= fExpFac;
  }

  // Take care of the fractional part of fOctaves
  fOctaves -= float(iOctaves);
  if  (fOctaves > 0.0f)
    fValue += fOctaves * Noise2(x, y, z) * fExp;

  return fValue;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CPerlin::Fbm3(float x, float y, float z, float fOctaves, float fExpFac) const
{
  float fValue = 0.0f;
  float fExp = 1.f;
  int iOctaves = int(fOctaves);

  for (int i = 0; i < iOctaves; i++)
  {
     fValue += Noise3(x, y, z) * fExp;// Sum weighted noise value
     x *= 2; y *= 2; z *= 2;
     fExp *= fExpFac;
  }

  // Take care of the fractional part of fOctaves
  fOctaves -= float(iOctaves);
  if  (fOctaves > 0.0f)
    fValue += fOctaves * Noise3(x, y, z) * fExp;

  return fValue;
}
