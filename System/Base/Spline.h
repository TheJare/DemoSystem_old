//----------------------------------------------------------------------------
//  Nombre:    Spline.h
//
//  Contenido: Generic spline template
//----------------------------------------------------------------------------


#ifndef _SPLINE_H_
#define _SPLINE_H_

//----------------------------------------------------------------------------
// class T must support the *= (float) & -= (T) operators,
// and the copy/assign members.
//----------------------------------------------------------------------------
template <class T> class CSpline
{
  public:

    struct TKnot
    {
      float   t;
      T       value;
    };

    CSpline():  m_nKnots(0)   { }
    ~CSpline()                { End(); }

    TError        Init      (int nKnots);
    TError        Init      (int nKnots, const TKnot *paKnots);
    void          End       ();
    bool          IsOk      () const                    { return m_nKnots != 0; }

    int           GetNKnots () const                    { return m_nKnots; }
    const TKnot  &GetKnot   (int nKnot) const           { ASSERT(unsigned(nKnot) < unsigned(m_nKnots)); return m_paKnots[nKnot]; }
          TKnot  &Knot      (int nKnot)                 { ASSERT(unsigned(nKnot) < unsigned(m_nKnots)); return m_paKnots[nKnot]; }

    const T       GetValue      (float t) const             { T val; GetValue(t, &val); return val; }
    void          GetValue      (float t, T *pVal) const;

    int           FindKnot      (float t) const;

  private:
    int       m_nKnots;
    TKnot    *m_paKnots;
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T> inline TError    CSpline<T>::Init      (int nKnots)
{
  End();
  if (nKnots <= 0)
    return RET_FAIL;
  m_paKnots = NEW_ARRAY(TKnot, nKnots);
  if (!m_paKnots)
    return RET_FAIL;
  m_nKnots = nKnots;

  return RET_OK;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T> inline TError    CSpline<T>::Init      (int nKnots, const TKnot *paKnots)
{
  if (RET_OK != Init(nKnots))
    return RET_FAIL;

  for (int i = 0; i < nKnots; i++)
  {
    if (i > 0)
      ASSERT(paKnots[i].t > paKnots[i-1].t);
    m_paKnots[i] = paKnots[i];
  }
  return RET_OK;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T> inline void      CSpline<T>::End       ()
{
  if (m_nKnots != 0)
  {
    DISPOSE_ARRAY(m_paKnots);
    m_nKnots = 0;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T> inline void      CSpline<T>::GetValue  (float t, T *pVal) const
{
  ASSERT(IsOk());

  if (m_nKnots == 1)
  {
    *pVal = m_paKnots[0].value;
    return;
  }

  int i = FindKnot(t);

  ASSERT(i == 0 || m_paKnots[i].t > m_paKnots[i-1].t);
  ASSERT(i > 0 || m_paKnots[i+1].t > m_paKnots[i].t);

  // Find knots for tangent calculations
  int i00 = (i > 0)? i-1 : i;
  int i01 = (i < m_nKnots-1)? i+1 : i;
  int i10 = i;
  int i11 = (i < m_nKnots-2)? i+2 : i+1;

  float td0 = (m_paKnots[i01].t - m_paKnots[i01-1].t);
  float td1 = (m_paKnots[i11].t - m_paKnots[i11-1].t);

  float ts0 = td/(m_paKnots[i01].t - m_paKnots[i00].t);
  float ts1 = td/(m_paKnots[i11].t - m_paKnots[i10].t);

  // t0 == t within the chosen interval, normalized to [0..1]
  // t0 may be < 0 or >= 1 if we're out of the knot range.
  float t0 = (t - m_paKnots[i].t) / td0;

/* TCB spline:
T0n = ts0*[ (1 - T )(1 - C)(1 - B) * (Pn+1 - Pn) +
            (1 - T )(1 + C)(1 + B) * (Pn - Pn-1)]

T1n = ts1*[ (1 - T )(1 + C)(1 - B) * (Pn+1 - Pn) +
            (1 - T )(1 - C)(1 + B) * (Pn - Pn-1)]
*/

  // Calc tangent values.
  T d0 = m_paKnots[i01].value;
  d0 -= m_paKnots[i00].value;
  d0 *= ts0;
  T d1 = m_paKnots[i11].value;
  d1 -= m_paKnots[i10].value;
  d1 *= ts1;

  // Powers of t0 for cubic polynomial
  float t02 = t0*t0;
  float t03 = t02*t0;

  // Hermite base
  float fv0 = 2.f*t03 - 3.f*t02 + 1;
  float fv1 = -2.f*t03 + 3.f*t02;
  float fv0p = t03 - 2.f*t02 + t0;
  float fv1p = -t03 + t02;

  // Dest = v0*fv0 + v1*fv1 + d0*fv0p + d1*fv1p
  *pVal = m_paKnots[i].value;
  *pVal *= fv0;
  T tempv = m_paKnots[i+1].value;
  tempv *= -fv1;
  *pVal -= tempv;
  d0 *= -fv0p;
  *pVal -= d0;
  d1 *= -fv1p;
  *pVal -= d1;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template <class T> inline int      CSpline<T>::FindKnot  (float t) const
{
  ASSERT(IsOk());

  int i;
  if (m_nKnots < 6)
  {
    // Simple linear search
    for (i = 0; i < m_nKnots-2 && t >= m_paKnots[i+1].t; i++)
      ;
  }
  else
  {
    // Quick binary search
    int range = m_nKnots-1;
    i = range/2;
    while (range > 1)
    {
      bool bGt = t >= m_paKnots[i].t;
      if (bGt && t < m_paKnots[i+1].t)
        return i;
      range /= 2;
      if (bGt)
        i += range;
      else
        i -= range;
    }
  }
  ASSERT(i >= 0 && i < m_nKnots-1);
  return CLAMP(i, 0, m_nKnots-1);
}


#endif // _SPLINE_H_