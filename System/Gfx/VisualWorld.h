// -------------------------------------------------------------------------------------
// File:        VisualWorld.h
//
// Purpose:     General-purpose Scenegraph
// -------------------------------------------------------------------------------------

#ifndef _VISUAL_WORLD_H_
#define _VISUAL_WORLD_H_

#include "Vectors.h"
#include "DynArray.h"

class CRenderContext;
class CDisplayDevice;

// -------------------------------

class CVisualWorld
{
  public:
    class IRenderable
    {
      public:
        enum
        {
          RF_HAS_TRANSPARENCY     = 0x0001,
          RF_HAS_SOLID            = 0x0002,
          RF_IS_HIDDEN            = 0x0004,

          DRAW_SOLID              = -1,
          DRAW_TRANSPARENT        = -2,
          DRAW_ALL                = -3,
        };

        // Must be implemented
        virtual void  Draw            (CRenderContext &rc, int nSection = DRAW_ALL)   const = 0;       // nSection may be DRAW_xxxx

        // Optional
        virtual bool  IsVisible       (CRenderContext &rc)                      const { return true; }
        virtual int   GetNumSections  ()                                        const { return 1; }
        virtual bool  IsTransparent   (int nSection)                            const { return false; }
        virtual dword GetMaterialRef  (int nSection)                            const { return 0; }
        virtual float GetRenderDepth  (const CRenderContext &rc)                const { return 0.f; }
        virtual dword GetRenderFlags  ()                                        const { return 0; }
        virtual void  PreDraw         (CRenderContext &rc)                            { }

        // Utility
                bool  HasTransparency ()                                        const { return (GetRenderFlags() & RF_HAS_TRANSPARENCY) != 0; }
                bool  HasSolid        ()                                        const { return (GetRenderFlags() & RF_HAS_SOLID) != 0; }
                bool  IsHidden        ()                                        const { return (GetRenderFlags() & RF_IS_HIDDEN) != 0; }
    };

    // ---------------------------

    struct TBucket
    {
      enum
      {
        F_PRESORTED       = 0x0001,       // No sorting to be done while drawing transparent sections
        F_SORTBYMATERIAL  = 0x0002,       // Solid nodes should be sorted by MaterialRef
        F_TRANSPARENT     = 0x0004,       // All nodes are transparent.
      };
      dword                   flags;
      CDynArray<IRenderable*> aNodes;

      TBucket(): flags(0) { }
    };

    // ---------------------------

    struct TSolidSection
    {
      IRenderable *pRenderable;
      int          nSection;
    };

    // --------------------------------------------------------------------

    CVisualWorld  (): m_pDevice(NULL)       { }
    ~CVisualWorld ()                        { End(); }

    TError      Init      (CDisplayDevice *pDevice, int nBuckets = 1);
    void        End       ();
    bool        IsOk      ()                                    const { return (m_pDevice != NULL); }

    void        AddNode         (IRenderable *pRenderable, int iBucket = 0);
    void        RemoveNode      (IRenderable *pRenderable);
    int         FindNode        (IRenderable *pRenderable)      const;     // Return bucket #, -1 if not found

    void        SetBucketFlags  (int iBucket, dword flags);

    void        PreDraw         (CRenderContext &rc);
    void        DrawBucket      (CRenderContext &rc, unsigned nBucket)  const;
    void        Draw            (CRenderContext &rc)                    const;

    // -----------------------------------------------------------
  private:
    CDisplayDevice                    *m_pDevice;
    CDynArray<TBucket>                m_aBuckets;

    mutable CDynArray<TSolidSection>  m_aSolidNodes;        // Reuse the dynamic array memory from last frame.
    mutable CDynArray<IRenderable*>   m_aTransparentNodes;  // Reuse the dynamic array memory from last frame.
};

#endif //_VISUAL_WORLD_H_
