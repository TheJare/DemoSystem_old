//----------------------------------------------------------------------------
//  Nombre:    DynArray.h
//
//  Contenido: Template de array dinamico
//----------------------------------------------------------------------------


#ifndef _DYN_ARRAY_H_
#define _DYN_ARRAY_H_

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
template<class T> class CDynArray
{
  public:

    CDynArray     ()               : m_paEls(NULL), m_maxEls(0), m_nEls(0)  { }
    CDynArray     (unsigned maxEls): m_paEls(NULL), m_maxEls(0), m_nEls(0)  { Grow(maxEls); }
    ~CDynArray    ()                                                        { End(); }

    void    Init  (int maxEls = 0)                      { End(); Grow(maxEls); }
    void    End   ()                                    { if (m_maxEls) { for (int i = m_nEls-1; i >= 0; i--) m_paEls[i].~T(); m_nEls = 0; char *pT = (char*)m_paEls; DISPOSE_ARRAY(pT); m_paEls = NULL; m_maxEls = 0; } }

    unsigned  GetCount      ()                    const { return m_nEls; }
    unsigned  GetCapacity   ()                    const { return m_maxEls; }
    bool      IsEmpty       ()                    const { return m_nEls == 0; }
    void      Grow          (unsigned newMax)           { if (newMax > m_maxEls) Resize(newMax); }
    void      Resize        (unsigned newMax);
    void      SetCount      (unsigned newNum)           { Grow(newNum); {for (int i = m_nEls-1; i >= (int)newNum; i--) m_paEls[i].~T();} {for (unsigned i = m_nEls; i < newNum; i++) new(m_paEls+m_nEls) T;} m_nEls = newNum; }

    T&        GetElement  (unsigned i)                  { ASSERT(i < m_nEls); return m_paEls[i]; }
    const T&  GetElement  (unsigned i)            const { ASSERT(i < m_nEls); return m_paEls[i]; }

    T&        operator[]  (unsigned i)                  { ASSERT(i < m_nEls); return m_paEls[i]; }
    const T&  operator[]  (unsigned i)            const { ASSERT(i < m_nEls); return m_paEls[i]; }

    T&        Add         ()                            { if (m_nEls == m_maxEls) Resize(Max(1U,2*m_maxEls)); new(m_paEls+m_nEls) T; m_nEls++; return m_paEls[m_nEls-1]; }
    T&        Add         (const T& t)                  { if (m_nEls == m_maxEls) Resize(Max(1U,2*m_maxEls)); new(m_paEls+m_nEls) T; m_paEls[m_nEls] = t; m_nEls++; return m_paEls[m_nEls-1]; }
    T&        Insert      (unsigned i, const T& t);
    unsigned  Find        (const T& t)            const { for (unsigned i = 0; i < m_nEls; i++) if (t == m_paEls[i]) return i; return m_nEls; }
    void      Foreach     (void Func(T &t))             { for (unsigned i = 0; i < m_nEls; i++) Func(m_paEls[i]); }
    void      Foreach     (void Func(const T &t)) const { for (unsigned i = 0; i < m_nEls; i++) Func(m_paEls[i]); }

    bool      Remove      (unsigned i);
    bool      Remove      (const T& t)                  { return Remove(Find(t)); }
    bool      RemoveOrdered (unsigned i);
    bool      RemoveOrdered (const T& t)                { return RemoveOrdered(Find(t)); }

//    operator T*           ()                            { return m_paEls; }
//    operator const T*     ()                      const { return m_paEls; }

    T*        operator+   (unsigned i)                  { ASSERT(i < m_nEls); return m_paEls + i; }
    const T*  operator+   (unsigned i)            const { ASSERT(i < m_nEls); return m_paEls + i; }

//    static  void DeleteFunc(T&t)                        { DISPOSE(t); }

    // STL-like
    typedef T*       iterator;
    typedef const T* const_iterator;

    const_iterator  begin      ()                     const { return m_paEls; }
    iterator        begin      ()                           { return m_paEls; }
    const_iterator  end        ()                     const { return m_paEls + m_nEls; }
    iterator        end        ()                           { return m_paEls + m_nEls; }
    void            push_back  (const T& t)                 { Add(t); }
    void            clear      ()                           { SetCount(0); }
    iterator        insert     (iterator it, const T& t)    { return &Insert(it-m_paEls, t); }
    iterator        erase      (iterator it)                { RemoveOrdered(it-m_paEls); return it; }

  private:
    T         *m_paEls;
    unsigned  m_nEls;
    unsigned  m_maxEls;
};

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
template<class T>
void CDynArray<T>::Resize (unsigned newMax)
{
  if (newMax != m_maxEls)
  {
    T* paTemp = newMax? (T*)NEW_ARRAY(char, sizeof(T)*newMax) : 0;
    unsigned newN = Min(newMax, m_nEls);

    for (int i = (int)m_nEls-1; i >= (int)newN; i--)
      m_paEls[i].~T();
    if (newN)
      memcpy(paTemp, m_paEls, newN*sizeof(T));

    char *pT = (char*)m_paEls;
    DISPOSE_ARRAY(pT);
    m_nEls = newN;
    m_paEls = paTemp;
    m_maxEls = newMax;
  }
}

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
template<class T>
bool CDynArray<T>::Remove      (unsigned i)
{
  if (i < m_nEls)
  {
    m_paEls[i].~T();
    if (i < m_nEls-1)
      memcpy(i, m_paEls+m_nEls-1, sizeof(T));
    --m_nEls;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
template<class T>
bool CDynArray<T>::RemoveOrdered  (unsigned i)
{
  if (i < m_nEls)
  {
    m_paEls[i].~T();
    while (i < m_nEls-1)
    {
      memcpy(m_paEls+i, m_paEls+i+1, sizeof(T));
      ++i;
    }
    --m_nEls;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
template<class T>
T& CDynArray<T>::Insert (unsigned i, const T& t)
{
  if (m_nEls == m_maxEls)
    Resize(Max(1U,2*m_maxEls)); 
  if (m_nEls > 0)
  {
    unsigned j = m_nEls-1;
    while (j > i)
    {
      memcpy(m_paEls+j, m_paEls+j-1, sizeof(T));
      --j;
    }
  }
  ++m_nEls;
  new(m_paEls+i) T;
  m_paEls[i] = t;

  return m_paEls[i];
}

#endif //_DYN_ARRAY_H_
