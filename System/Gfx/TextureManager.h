//----------------------------------------------------------------------------
//  Nombre:    TextureManager.h
//
//  Contenido: Agrupacion de texturas.
//----------------------------------------------------------------------------

#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "LinkedList.h"
#include "DisplayDevice.h"

//----------------------------------------------------------------------------

class CDisplayDevice;
class CDisplayTexture;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CTextureManager: public CDisplayDevice::IManager
{
  public:
    CTextureManager      (): m_pDev(NULL)           { }
    ~CTextureManager     ()                         { End(); }

    TError    Init        (CDisplayDevice *pDisplay);
    void      End         ();
    bool      IsOk        () const                  { return m_pDev != NULL; }

    CDisplayTexture *
              InitMaterial    (const char *pszName, uint wantMatFlags);
    bool      ReleaseMaterial (const char *pszName);
    bool      ReleaseMaterial (CDisplayTexture *pTex);

    CDisplayTexture *
              GetMaterial (const char *pszName) const;
    CDisplayDevice *
              GetDevice   ()                    const { return m_pDev; }

        // Return false if want to be removed from the list due to some kind of error.
    virtual bool Shutdown(CDisplayDevice *pDevice);
    virtual bool Restore (CDisplayDevice *pDevice);

  private:
    struct TTexture;
    TLinkedListNode<TTexture*> *
              FindMaterial (const char *pszName) const;

    TLinkedListNode<TTexture*> *
              FindMaterial (CDisplayTexture *) const;

    CDisplayDevice           *m_pDev;
    CLinkedList<TTexture*>   m_Textures;
};

#endif // _TEXTURE_MANAGER_H_
