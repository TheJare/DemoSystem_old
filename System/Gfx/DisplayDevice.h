// -------------------------------------------------------------------------------------
// File:        DisplayDevice.h 
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _DISPLAYDEVICE_H_
#define _DISPLAYDEVICE_H_

#include "DXNames.h"
#include "LinkedList.h"
#include "Vectors.h"

class CWinWindow;

struct TMatrix3;
struct TProjectionViewport;

class CDisplayDevice;

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class IDisplayMaterial
{
  public:
    virtual void Set        (CDisplayDevice *, int nPass = 0) const = 0;
    virtual int  GetNPasses ()                                const { return 1; }
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CDisplayDevice
{
  public:
    enum
    {
      F_WANT_FULLSCREEN   = 0x0001,
      F_WANT_ZBUFFER      = 0x0002,
      F_WANT_STENCIL      = 0x0010,
      F_WANT_WBUFFER      = 0x1000,
      F_WANT_FSAA         = 0x2000,
      F_WANT_NODISCARD    = 0x4000,       // Incompatible with F_WANT_FSAA -> out of memory?
    };

    class IManager
    {
      public:
        // Return false if want to be removed from the list due to some kind of error.
        virtual bool Shutdown(CDisplayDevice *pDevice) = 0;
        virtual bool Restore (CDisplayDevice *pDevice) = 0;
    };

    CDisplayDevice    (): m_pWindow(NULL)     { }
    ~CDisplayDevice   ()                      { End(); }

    TError        Init      (CWinWindow *pWindow);
    void          End       ();
    bool          IsOk      ()                      const { return (m_pWindow != NULL); }

    DXN_Direct3DDevice
                  *GetDirect3DDevice()              const { return m_pDevice; }
    DXN_Direct3D  *GetDirect3D      ()              const { return m_pD3D; }
    CWinWindow    *GetWindow        ()              const { return m_pWindow; }


    TError        SetDisplayMode    (int w, int h, int bpp, bool bFullscreen = true, unsigned wantFlags = F_WANT_ZBUFFER | F_WANT_WBUFFER | F_WANT_FSAA);
    void          Shutdown          ();
    TError        Restore           ();
    bool          UsingZBuffer      ()              const { return (m_mode.flags & F_WANT_ZBUFFER) != 0; }
    bool          UsingStencil      ()              const { return (m_mode.flags & F_WANT_STENCIL) != 0; }
    bool          UsingFSAA         ()              const { return (m_mode.flags & F_WANT_FSAA) != 0; }

    unsigned      GetMaxTextureW    ()              const { return m_maxTextureW; }
    unsigned      GetMaxTextureH    ()              const { return m_maxTextureH; }

    bool          IsAvailable       ()              const { return IsOk() && !(m_flags & F_INRESET); }
    bool          IsDeviceReady     () const;

    bool          HasDynamicTextures()              const { return (m_flags & FS_DYNAMIC_TEXTURES) != 0; }

    TError        BeginFrame        ();
    void          EndFrame          ();
    void          Update            ();
    void          Clear             (TColor color = 0xFF000000U, float depth = 1.f, int stValue = -1);

    int           GetWidth          ()              const { return m_mode.w; }
    int           GetHeight         ()              const { return m_mode.h; }
    int           GetBpp            ()              const { return m_mode.bpp; }
    bool          IsFullScreen      ()              const { return m_mode.IsFS(); }
    int           GetFlags          ()              const { return m_mode.flags; }

    void          Flush             ();

    void          SetViewport       (int x, int y, int w, int h);
    void          SetProjection     (float ivpw, float ivph, float Q, float zn);
    void          SetProjectionViewport
                                    (const TProjectionViewport *vp);

    void          SetWorldTransform (const TMatrix3 &m);
    void          SetViewTransform  (const TMatrix3 &m);
    void          SetCameraTransform(const TMatrix3 &m);  // Como la anterior pero invirtiendo automaticamente la matriz

    const TMatrix3&     GetWorldTransform ()              const { return m_WorldTransform; }
    const TMatrix3&     GetViewTransform  ()              const { return m_ViewTransform; }
    const TMatrix4&     GetProjTransform  ()              const { return m_ProjTransform; }
    const TMatrix4&     GetTransWorldViewProj()           const { if (m_bWorldViewProjDirty) RecomputeWorldViewProj(); return m_TransWorldViewProj; }
    const TMatrix4&     GetTransScreenTransform  ()       const { if (m_bScreenTransformDirty) RecomputeScreenTransform(); return m_TransScreenTransform; }

    void          RegisterManager   (IManager *pMgr);
    void          UnregisterManager (IManager *pMgr);

    void          SetTexture        (int stage, DXN_Direct3DTexture *pTex);
    void          SetRS             (unsigned rs, unsigned val)               { ASSERT(rs < MAXRENDERSTATE); if (m_aRenderState[rs] != val) InternalSetRS(rs, val); }
    void          SetTSS            (int stage, unsigned tss, unsigned val)   { ASSERT(stage < MAXTEXTURESTAGES); ASSERT(tss < MAXTEXTURESTAGESTATES); if (m_aTextureStageState[stage][tss] != val) InternalSetTSS(stage, tss, val); }
    void          SetSS             (int stage, unsigned ss, unsigned val)    { ASSERT(stage < MAXTEXTURESTAGES); ASSERT(ss < MAXSAMPLERSTATES); if (m_aSamplerState[stage][ss] != val) InternalSetSS(stage, ss, val); }
    void          ResetMaterial     ();
    void          EnableHWTnL       (bool bEnable);
    bool          UsingHWTnL        () const;

    void          SetFVFShader      (unsigned fvf);

    unsigned      GetCachedRS       (unsigned rs)                       const { ASSERT(rs < MAXRENDERSTATE); return m_aRenderState[rs]; }
    unsigned      GetCachedTSS      (int stage, unsigned tss)           const { ASSERT(stage < MAXTEXTURESTAGES); ASSERT(tss < MAXTEXTURESTAGESTATES); return m_aTextureStageState[stage][tss]; }
    unsigned      GetCachedSS       (int stage, unsigned ss)            const { ASSERT(stage < MAXTEXTURESTAGES); ASSERT(ss < MAXSAMPLERSTATES); return m_aSamplerState[stage][ss]; }

  private:
    enum
    {
      F_INRESET       = 0x80000000,

      FS_MASK         = 0x0FFFFFFF,
      FS_DYNAMIC_TEXTURES = 0x00000001,

      MAXTEXTURESTAGES      =   4,
      MAXTEXTURESTAGESTATES =  33,
      MAXSAMPLERSTATES      =  14,
      MAXRENDERSTATE        = 210,
    };

    struct TDisplayMode
    {
      int      w, h, bpp;
      unsigned flags;

      bool IsOk()   const { return bpp != 0; }
      bool IsFS()   const { return (flags & F_WANT_FULLSCREEN) != 0; }
      bool IsDB()   const { return (flags & (F_WANT_ZBUFFER | F_WANT_STENCIL | F_WANT_WBUFFER)) != 0; }
      bool IsFSAA() const { return (flags & F_WANT_FSAA) != 0; }
      bool IsNoDiscard() const { return (flags & F_WANT_NODISCARD) != 0; }
    };

    // ------------------------------------------------------

    unsigned  FindDesktopFormat       () const;
    unsigned  FindBackbufferFormat    (int bpp) const;
    unsigned  FindDepthbufferFormat   (unsigned format, unsigned flags) const;
    unsigned  FindFormatBpp           (unsigned fmt) const;

    void      ShutdownManagers        ();
    void      RestoreManagers         ();

    void      InternalSetRS           (unsigned rs, unsigned val);
    void      InternalSetTSS          (int stage, unsigned tss, unsigned val);
    void      InternalSetSS           (int stage, unsigned ss, unsigned val);

    void      RecomputeWorldViewProj() const;
    void      RecomputeScreenTransform() const;

    // ------------------------------------------------------

    CWinWindow          *m_pWindow;
    DXN_Direct3D        *m_pD3D;
    DXN_Direct3DDevice  *m_pDevice;
    unsigned            m_flags;
    unsigned            m_desktopFormat;

    TDisplayMode        m_mode;
    TDisplayMode        m_newMode;

    TMatrix3            m_WorldTransform;
    TMatrix3            m_ViewTransform;
    TMatrix4            m_ProjTransform;
    TVector2            m_ViewportPos;
    TVector2            m_ViewportSize;
    mutable TMatrix4    m_TransWorldViewProj;   // Cache the transpose of the xform matrices
    mutable TMatrix4    m_TransScreenTransform; //
    mutable bool        m_bWorldViewProjDirty;
    mutable bool        m_bScreenTransformDirty;

    CLinkedList<IManager*> m_ManagerList;

    // State cache.
    unsigned            m_aRenderState[MAXRENDERSTATE];
    unsigned            m_aTextureStageState[MAXTEXTURESTAGES][MAXTEXTURESTAGESTATES];
    unsigned            m_aSamplerState[MAXTEXTURESTAGES][MAXSAMPLERSTATES];
    unsigned            m_FVFShader;
    bool                m_bUsingSoftwareVP;

    // Caps
    unsigned            m_maxTextureW;
    unsigned            m_maxTextureH;
};

#endif // _DISPLAYDEVICE_H_
