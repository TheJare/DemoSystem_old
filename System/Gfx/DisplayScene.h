// -------------------------------------------------------------------------------------
// File:        DisplayScene.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _DISPLAYSCENE_H_
#define _DISPLAYSCENE_H_

#include "vectors.h"
#include "NamedList.h"
#include "DynArray.h"

class CDisplayDevice;
class CTextureManager;
class CDisplayTexture;
class CBasicMesh;
class CReadChunker;
class CRenderContext;
class CVisualWorld;

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
class CDisplayScene
{
  public:
    enum
    {
      F_LIGHT   = 0x00001,
      F_CAMERA  = 0x00002,

      F_MESH    = 0x00004,
      F_NODE    = 0x00008,
    };

    class CRenderableInstance;

    struct TNode
    {
      char        szName[32];
      const TNode *pParent;
      TVector3    initPos;
      TQuaternion initRot;
      TVector3    initScale;
      TVector3    pos;
      TQuaternion rot;
      TVector3    scale;
      int         numPosKeys;
      int         numRotKeys;
      int         numScaleKeys;
      int         keysPerFrame;
      TVector3    *paPosKeys;
      TQuaternion *paRotKeys;
      TVector3    *paScaleKeys;
    };

    struct TMesh
    {
      char        szName[32];
      CBasicMesh  *pMesh;
    };
    struct TInstance
    {
      const TNode         *pNode;
      dword               color;
      const TMesh         *pMesh;
      CRenderableInstance *pRenderable;
    };

    struct TTexture
    {
      CDisplayTexture *pTexture;
    };
    struct TMaterial
    {
      char            szName[32];
      dword           ambient;
      dword           diffuse;
      dword           specular;
      const TTexture *pDiffuseTex;
      const TTexture *pEnvMap;
      float           fShininess;
      float           fShnStrength;
      bool            bUsesAlpha;
    };

    struct TCamera
    {
      const TNode *pNode;
      float        fov;
    };

    struct TLight
    {
      const TNode *pNode;
      dword        color;
      float        fPower;
      bool         bIsOn;
      bool         bDoDiffuse;
      bool         bDoSpecular;
    };

    // -----------------------------------------------------------

    CDisplayScene(): m_pDevice(NULL)      { }
    ~CDisplayScene()                      { End(); }

    TError      Init      (CDisplayDevice *pDevice, const char *pszSceneFile, CTextureManager *pMgr = NULL);
    void        End       ();
    bool        IsOk      ()                      const { return (m_pDevice != NULL); }

    int         GetNumNodes       ()              const { return m_aNodes      .GetCount(); }
    int         GetNumInstances   ()              const { return m_aInstances  .GetCount(); }
    int         GetNumMeshes      ()              const { return m_aMeshes     .GetCount(); }
    int         GetNumCameras     ()              const { return m_aCameras    .GetCount(); }
    int         GetNumTextures    ()              const { return m_aTextures   .GetCount(); }
    int         GetNumMaterials   ()              const { return m_aMaterials  .GetCount(); }
    int         GetNumLights      ()              const { return m_aLights     .GetCount(); }

    const TNode      &GetNode           (int i)         const { return m_aNodes      [i]; }
    const TInstance  &GetInstance       (int i)         const { return m_aInstances  [i]; }
    const TMesh      &GetMesh           (int i)         const { return m_aMeshes     [i]; }
    const TCamera    &GetCamera         (int i)         const { return m_aCameras    [i]; }
    const TTexture   &GetTexture        (int i)         const { return m_aTextures   [i]; }
    const TMaterial  &GetMaterial       (int i)         const { return m_aMaterials  [i]; }
    const TLight     &GetLight          (int i)         const { return m_aLights     [i]; }

    TNode      &GetNode           (int i)         { return m_aNodes      [i]; }
    TInstance  &GetInstance       (int i)         { return m_aInstances  [i]; }
    TMesh      &GetMesh           (int i)         { return m_aMeshes     [i]; }
    TCamera    &GetCamera         (int i)         { return m_aCameras    [i]; }
    TTexture   &GetTexture        (int i)         { return m_aTextures   [i]; }
    TMaterial  &GetMaterial       (int i)         { return m_aMaterials  [i]; }
    TLight     &GetLight          (int i)         { return m_aLights     [i]; }

    TMesh      *FindMesh          (const char *pszName);
    TNode      *FindNode          (const char *pszName);
    TMaterial  *FindMaterial      (const char *pszName);

    const TMesh      *FindMesh          (const char *pszName) const   { return ((CDisplayScene*)this)->FindMesh     (pszName); }
    const TNode      *FindNode          (const char *pszName) const   { return ((CDisplayScene*)this)->FindNode     (pszName); }
    const TMaterial  *FindMaterial      (const char *pszName) const   { return ((CDisplayScene*)this)->FindMaterial (pszName); }

    int         GetLen            ()              const { return m_nFrames; }
    dword       GetBackground     ()              const { return m_clrBackground; }
    dword       GetAmbient        ()              const { return m_clrAmbient;    }
    void        SetBackground     (dword clr)           { m_clrBackground = clr; }
    void        SetAmbient        (dword clr)           { m_clrAmbient = clr;    }

    void        SetCurTime        (float fTime);
    void        Draw              (CRenderContext &rc) const;

    // --------------------------------
    // Other
    void            GetNodeAtTime   (TVector3 *pPos, TQuaternion *pRot, TVector3 *pScale, const TNode *pNode, float fTime) const;

    // -----------------------------------------------------------
    enum
    {
      DRAW_USING_LIGHTS = 0x0001,
      DRAW_SCALING      = 0x0002,
    };

  private:
    CDisplayDevice    *m_pDevice;
    CVisualWorld      *m_pWorld;

    int               m_nFrames;
    int               m_framesPerSec;
    dword             m_clrBackground;
    dword             m_clrAmbient;

    CDynArray<TNode     >  m_aNodes;
    CDynArray<TInstance >  m_aInstances;
    CDynArray<TMaterial >  m_aMaterials;
    CDynArray<TTexture  >  m_aTextures;
    CDynArray<TMesh     >  m_aMeshes;
    CDynArray<TCamera   >  m_aCameras;
    CDynArray<TLight    >  m_aLights;

    CTextureManager  *m_pTextureManager;
    bool              m_bOwnTextureManager;

    // --------------------------------
    // Internal loading methods.
    TError          LoadScene           (const char *pszName);
    TError          LoadChunkGeneral    (CReadChunker &ch);

    template<class T>
    TError          LoadArray         (CDynArray<T> &a, CReadChunker &ch, const char *pszChunk);
    TError          LoadTextures      (CReadChunker &ch);
    TError          LoadControllers   (CReadChunker &ch);

    TError          Load   (TTexture *pTexture, CReadChunker &ch);
    TError          Load   (TMaterial *pMaterial, CReadChunker &ch);
    TError          Load   (TNode *pNode, CReadChunker &ch);
    TError          Load   (TCamera *pCamera, CReadChunker &ch);
    TError          Load   (TLight *pLight, CReadChunker &ch);
    TError          Load   (TMesh *pMesh, CReadChunker &ch);
    TError          Load   (TInstance *pInstance, CReadChunker &ch);

    // --------------------------------
    // Internal drawing methods
    struct TTransparentSection
    {
      const TInstance *pInstance;
      int              numSection;
    };
    static bool     ApplyInstanceTransform  (CRenderContext &rc, const TInstance *pInstance);
    static void     DrawInstanceSection     (CRenderContext &rc, const TInstance *pInstance, int section, dword drawFlags);
    static void     DrawInstance            (CRenderContext &rc, const TInstance *pInstance, CDynArray <TTransparentSection> &TransparentList, dword drawFlags);
};

#endif _DISPLAYSCENE_H_
