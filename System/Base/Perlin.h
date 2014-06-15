// -------------------------------------------------------------------------------------
// File:        Perlin.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _PERLIN_H_
#define _PERLIN_H_

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CPerlin
{
  public:
    enum
    {
      FNOISE_DIM  = 3,

      FNOISE_BITS = 8,
      FNOISE_SIZE = (1 << FNOISE_BITS),
      FNOISE_MASK = FNOISE_SIZE-1,
    };

    CPerlin       (): m_bOk(false)    { }
    ~CPerlin      ()                  { End(); }

    TError      Init        ();
    void        End         ();
    bool        IsOk        ()                              const { return m_bOk; }

    float       Noise2      (float x, float y, int z = 0) const;
    float       Noise3      (float x, float y, float z) const;

    float       Turbulence2 (float x, float y, int z,   float fOctaves, float fExpFac = .5f) const;
    float       Turbulence3 (float x, float y, float z, float fOctaves, float fExpFac = .5f) const;

    float       Fbm2        (float x, float y, int z,   float fOctaves, float fExpFac = .5f) const;
    float       Fbm3        (float x, float y, float z, float fOctaves, float fExpFac = .5f) const;

  private:

    bool          m_bOk;
    float         m_afNoise[FNOISE_SIZE][FNOISE_DIM];
    int           m_aPerm[FNOISE_SIZE*2];
};

#endif // _PERLIN_H_
