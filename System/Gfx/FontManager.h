//----------------------------------------------------------------------------
//  Nombre:    FontManager.h
//
//  Contenido: Agrupacion de fonts
//----------------------------------------------------------------------------

#ifndef _FONT_MANAGER_H_
#define _FONT_MANAGER_H_

#include "LinkedList.h"
#include "DisplayDevice.h"
#include "DynamicVB.h"

//----------------------------------------------------------------------------

class CDisplayDevice;
class CDisplayFont;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CFontManager: public CDisplayDevice::IManager
{
  public:
    CFontManager      (): m_pDev(NULL)           { }
    ~CFontManager     ()                         { End(); }

    TError    Init        (CDisplayDevice *pDisplay);
    void      End         ();
    bool      IsOk        () const                  { return m_pDev != NULL; }

    CDisplayFont *
              InitFont    (const char *pszName, uint wantMatFlags);
    bool      ReleaseFont (const char *pszName);
    bool      ReleaseFont (CDisplayFont *pTex);

    CDisplayFont *
              GetFont     (const char *pszName) const;

        // Return false if want to be removed from the list due to some kind of error.
    virtual bool Shutdown(CDisplayDevice *pDevice);
    virtual bool Restore (CDisplayDevice *pDevice);

  private:
    struct TFont;
    TLinkedListNode<TFont*> *
              FindFont (const char *pszName) const;

    TLinkedListNode<TFont*> *
              FindFont (CDisplayFont *) const;

    CDynamicVB            m_VB;

    CDisplayDevice       *m_pDev;
    CLinkedList<TFont*>   m_Fonts;
};

#endif // _FONT_MANAGER_H_
