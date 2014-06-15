//----------------------------------------------------------------------------
//  Nombre:    NamedList.h
//
//  Contenido: Template de lista enlazada con identificador de nombre.
//----------------------------------------------------------------------------


#ifndef _NAMED_LIST_H_
#define _NAMED_LIST_H_

#include "StrUtil.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
template<class T> struct TNamedListNode
{
  enum { MAX_NAME = 32 };
  TNamedListNode  *pNext, *pPrev;
  T                 t;
  char              szName[MAX_NAME];

  TNamedListNode(const T &_t, const char *pszName): t(_t) { StrUtil::SafeStrncpy(szName, pszName, sizeof(szName)); }

  const T &GetData() const  { return t; }
        T &GetData()        { return t; }
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
template<class T> class CNamedList
{
  public:

    CNamedList   (): m_pHead(NULL), m_pTail(NULL), m_nNodes(0) { }
    ~CNamedList  ()                                            { End(); }

    void    Init  ()                                  { End(); }
    void    End   ();

    unsigned  GetCount  () const                      { return m_nNodes; }
    bool      IsEmpty   () const                      { return m_nNodes == 0; }

    TNamedListNode<T>  *GetHead () const             { return m_pHead; }
    TNamedListNode<T>  *GetTail () const             { return m_pTail; }

    void    AddFirst  (const T& t, const char *pszName);
    void    AddEnd    (const T& t, const char *pszName);
    void    Add       (const T& t, const char *pszName)    { AddFirst(t, pszName); }
    bool    Remove    (const T& t);
    bool    Remove    (const char *pszName);
    void    Remove    (TNamedListNode<T> *pNode);
    void    RemoveAll ()                              { End(); }
    void    DeleteAll ();                                               // T must be a dynamically allocated pointer.

    bool                IsIn      (const T& t) const  { return FindNode(t) != NULL; }
    TNamedListNode<T>  *FindNode  (const T& t) const;
    TNamedListNode<T>  *FindNode  (const char *pszName) const;

  private:

    // Prohibo usarlo.
    CNamedList<T> &operator = (const CNamedList<T> &) { }

    TNamedListNode<T>  *m_pHead;
    TNamedListNode<T>  *m_pTail;
    unsigned            m_nNodes;
};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> void CNamedList<T>::End()
{
  TNamedListNode<T> *pNode = m_pHead;
  while (pNode != NULL)
  {
    TNamedListNode<T> *pNext = pNode->pNext;
    delete pNode;
    pNode = pNext;
  }
  m_pHead = NULL;
  m_pTail = NULL;
  m_nNodes = 0;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> void CNamedList<T>::AddFirst(const T& t, const char *pszName)
{
  TNamedListNode<T> *pNode = NEW(TNamedListNode<T>(t, pszName));

  if (m_pHead == NULL)
  {
    m_pHead = m_pTail = pNode;
    pNode->pNext = NULL;
  }
  else
  {
    pNode->pNext = m_pHead;
    m_pHead->pPrev = pNode;
    m_pHead = pNode;
  }
  pNode->pPrev = NULL;
  m_nNodes++;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> void CNamedList<T>::AddEnd(const T& t, const char *pszName)
{
  TNamedListNode<T> *pNode = NEW(TNamedListNode<T>(t, pszName));

  if (m_pTail == NULL)
  {
    m_pHead = m_pTail = pNode;
    pNode->pPrev = NULL;
  }
  else
  {
    pNode->pPrev = m_pTail;
    m_pTail->pNext = pNode;
    m_pTail = pNode;
  }
  pNode->pNext = NULL;
  m_nNodes++;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> bool CNamedList<T>::Remove(const T& t)
{
  TNamedListNode<T> *pNode = FindNode(t);
  if (pNode != NULL)
    Remove(pNode);
  return pNode != NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> bool CNamedList<T>::Remove(const char *pszName)
{
  TNamedListNode<T> *pNode = FindNode(pszName);
  if (pNode != NULL)
    Remove(pNode);
  return pNode != NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> void CNamedList<T>::Remove(TNamedListNode<T> *pNode)
{
  if (pNode->pPrev == NULL)     m_pHead = pNode->pNext;
  else                          pNode->pPrev->pNext = pNode->pNext;
  if (pNode->pNext == NULL)     m_pTail = pNode->pPrev;
  else                          pNode->pNext->pPrev = pNode->pPrev;
  delete pNode;
  m_nNodes--;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> void CNamedList<T>::DeleteAll()
{
  TNamedListNode<T> *pNode = m_pHead;
  while (pNode != NULL)
  {
    TNamedListNode<T> *pNext = pNode->pNext;
    delete pNode->t;
    delete pNode;
    pNode = pNext;
  }
  m_pHead = NULL;
  m_pTail = NULL;
  m_nNodes = 0;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> TNamedListNode<T> *CNamedList<T>::FindNode(const T& t) const
{
  TNamedListNode<T> *pNode = m_pHead;
  while (pNode != NULL && pNode->t != t)
  {
    pNode = pNode->pNext;
  }
  return pNode;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T> TNamedListNode<T> *CNamedList<T>::FindNode(const char *pszName) const
{
  TNamedListNode<T> *pNode = m_pHead;
  while (pNode != NULL && strnicmp(pNode->szName, pszName, sizeof(pNode->szName)) != 0)
    pNode = pNode->pNext;
  return pNode;
}




#endif // _NAMED_LIST_H_
