// -------------------------------------------------------------------------------------
// File:        DisplayScene.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "DisplayScene.h"
#include "DisplayTexture.h"
#include "TextureManager.h"
#include "BasicMesh.h"
#include "DisplayVertex.h"
#include "RenderContext.h"
#include "ReadChunker.h"
#include "VisualWorld.h"

#define DEFAULT_SPECULAR_VALUE  .7f
#define SHININESS_SCALE          128
#define DEFAULT_SHININESS_VALUE .3f
#define SCALE_EPSILON           .01f
#define DEFAULT_FPS             30
#define DEFAULT_LIGHT_RANGE     5000

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CDisplayScene::CRenderableInstance: public CVisualWorld::IRenderable
{
  public:
    CRenderableInstance(CDisplayScene *pScene, CDisplayScene::TInstance *pInstance): m_pScene(pScene), m_pInstance(pInstance), m_pMesh(pInstance->pMesh->pMesh)
    {
      dword flags = 0;
      for (int i = 0; i < m_pMesh->GetNumSections(); ++i)
      {
        const TMaterial *pMat = (const TMaterial *)m_pMesh->GetSectionData(i);
        if (!pMat || !pMat->bUsesAlpha)
          flags |= RF_HAS_SOLID;
        else 
          flags |= RF_HAS_TRANSPARENCY;
      }
      m_flags = flags;
    }

    virtual void  Draw            (CRenderContext &rc, int nSection)   const;       // nSection may be DRAW_xxxx

    // Optional
//    virtual bool  IsVisible       (CRenderContext &rc)                      const { return true; }
    virtual int   GetNumSections  ()                                        const { return m_pMesh->GetNumSections(); }
    virtual bool  IsTransparent   (int nSection)                            const { const TMaterial *pMat = ((const TMaterial *)m_pMesh->GetSectionData(nSection)); return pMat && pMat->bUsesAlpha; }
    virtual dword GetMaterialRef  (int nSection)                            const { return m_pMesh->GetSectionData(nSection); }
    virtual float GetRenderDepth  (const CRenderContext &rc)                const;
    virtual dword GetRenderFlags  ()                                        const { return m_flags; }
//    virtual void  PreDraw         (CRenderContext &rc)                            { }

  private:
    CDisplayScene            *m_pScene;
    CDisplayScene::TInstance *m_pInstance;
    CBasicMesh               *m_pMesh;
    dword                     m_flags;

    void      DrawInstanceSection (CRenderContext &rc, int section, dword drawFlags) const;
};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::CRenderableInstance::Draw (CRenderContext &rc, int nSection)   const
{
  TMatrix3 m;
  const TNode *pNode = m_pInstance->pNode;

  TVector3 v = pNode->pos;
  TQuaternion r = pNode->rot;
  TVector3 s = pNode->scale;

  bool bScaled = (fabsf(s.x - 1.f) > SCALE_EPSILON)
              && (fabsf(s.y - 1.f) > SCALE_EPSILON)
              && (fabsf(s.z - 1.f) > SCALE_EPSILON);
  if (bScaled)
  {
    TMatrix3 sm;
    sm.SetScale(s.x, s.y, s.z);
    m = sm*r.ToMatrix();
  }
  else
    m = r.ToMatrix();

  m.SetTranslation(v.x, v.y, v.z);
  rc.GetDevice()->SetWorldTransform(m);

  dword drawFlags = m_pScene->GetNumLights()? DRAW_USING_LIGHTS : 0;
  if (bScaled)
    drawFlags |= DRAW_SCALING;

  if ((unsigned)nSection < (unsigned)m_pMesh->GetNumSections())
    DrawInstanceSection(rc, nSection, drawFlags);
  else
  {
    for (int i = 0; i < m_pMesh->GetNumSections(); ++i)
    {
      const TMaterial *pMat = ((const TMaterial *)m_pMesh->GetSectionData(i));
      bool bAlpha = pMat && pMat->bUsesAlpha;
      if (   (bAlpha && nSection != CDisplayScene::CRenderableInstance::DRAW_SOLID)
          || (!bAlpha && nSection != CDisplayScene::CRenderableInstance::DRAW_TRANSPARENT))
        DrawInstanceSection(rc, i, drawFlags);
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::CRenderableInstance::DrawInstanceSection (CRenderContext &rc, int section, dword drawFlags) const
{
  const TMaterial *pMat = (const TMaterial *)m_pMesh->GetSectionData(section);

  // Setup shader
  CDisplayShader sh;
  sh.Init();

  if (pMat && pMat->pDiffuseTex)
  {
    sh.GetStage(0, 0)->pTex = pMat->pDiffuseTex->pTexture;
  }
  else
  {
    sh.GetPass(0)->TFactor = pMat? ((drawFlags & DRAW_USING_LIGHTS)? 0xFFFFFFFF : pMat->diffuse) : m_pInstance->color;
    sh.GetStage(0, 0)->ColorArg2 = D3DTA_TFACTOR;
    sh.GetStage(0, 0)->AlphaArg2 = D3DTA_TFACTOR;
  }
  if (pMat && pMat->pEnvMap)
  {
    sh.GetStage(0, 1)->pTex = pMat->pEnvMap->pTexture;
    sh.GetStage(0, 1)->ColorOp = D3DTOP_ADD;
    sh.GetStage(0, 1)->AlphaOp = D3DTOP_SELECTARG1;
//    sh.GetStage(0, 1)->TexGenMode = 1 | D3DTSS_TCI_CAMERASPACENORMAL;//*/D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
    sh.GetStage(0, 1)->TexGenMode = 1 | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
  }

  if (pMat && pMat->bUsesAlpha)
  {
    sh.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
    sh.GetPass(0)->DstBlend = D3DBLEND_INVSRCALPHA;
  }
  else
  {
    sh.GetPass(0)->SrcBlend = D3DBLEND_ONE;
    sh.GetPass(0)->DstBlend = D3DBLEND_ZERO;
  }

  // Setup lighting material properties
  if (drawFlags & DRAW_USING_LIGHTS)
  {
    sh.GetPass(0)->Lighting = true;
    D3DMATERIAL9 m;

    if (pMat)
    {
      m.Diffuse.a = (1.f/255.f)*Color_A(pMat->diffuse);
      m.Diffuse.r = (1.f/255.f)*Color_R(pMat->diffuse);
      m.Diffuse.g = (1.f/255.f)*Color_G(pMat->diffuse);
      m.Diffuse.b = (1.f/255.f)*Color_B(pMat->diffuse);

      m.Ambient.a = (1.f/255.f)*Color_A(pMat->ambient);
      m.Ambient.r = (1.f/255.f)*Color_R(pMat->ambient);
      m.Ambient.g = (1.f/255.f)*Color_G(pMat->ambient);
      m.Ambient.b = (1.f/255.f)*Color_B(pMat->ambient);

      m.Specular.a = (pMat->fShnStrength/255.f)*Color_A(pMat->specular);
      m.Specular.r = (pMat->fShnStrength/255.f)*Color_R(pMat->specular);
      m.Specular.g = (pMat->fShnStrength/255.f)*Color_G(pMat->specular);
      m.Specular.b = (pMat->fShnStrength/255.f)*Color_B(pMat->specular);
      m.Power = pMat->fShininess*SHININESS_SCALE;
    }
    else
    {
      m.Diffuse.a = (1.f/255.f)*Color_A(m_pInstance->color);
      m.Diffuse.r = (1.f/255.f)*Color_R(m_pInstance->color);
      m.Diffuse.g = (1.f/255.f)*Color_G(m_pInstance->color);
      m.Diffuse.b = (1.f/255.f)*Color_B(m_pInstance->color);

      m.Ambient = m.Diffuse;

      m.Specular.a = 1.f;
      m.Specular.r = DEFAULT_SPECULAR_VALUE;
      m.Specular.g = DEFAULT_SPECULAR_VALUE;
      m.Specular.b = DEFAULT_SPECULAR_VALUE;
      m.Power = DEFAULT_SHININESS_VALUE*SHININESS_SCALE;
    }

    m.Emissive.a = 0;
    m.Emissive.r = 0;
    m.Emissive.g = 0;
    m.Emissive.b = 0;
    rc.GetDevice()->GetDirect3DDevice()->SetMaterial(&m);
  }
  rc.GetDevice()->SetRS(D3DRS_NORMALIZENORMALS, (drawFlags & (DRAW_USING_LIGHTS | DRAW_SCALING))? TRUE : FALSE);

  sh.Set(rc.GetDevice());

  m_pMesh->Draw(rc.GetDevice(), section);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
float CDisplayScene::CRenderableInstance::GetRenderDepth  (const CRenderContext &rc) const
{
  const TMatrix3 &m = rc.GetDevice()->GetViewTransform();

  TVector3 v = m_pInstance->pNode->pos*m;
  return -v.z;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Init      (CDisplayDevice *pDevice, const char *pszSceneFile, CTextureManager *pMgr)
{
  End();

  m_nFrames           = 0;
  m_framesPerSec      = DEFAULT_FPS;
  m_clrBackground     = 0;
  m_clrAmbient        = 0xFFFFFFFF;

  if (pMgr)
  {
    m_pTextureManager = pMgr;
    m_bOwnTextureManager = false;
    if (!m_pTextureManager->IsOk())
      m_pTextureManager->Init(pDevice);
  }
  else
  {
    m_pTextureManager = NEW(CTextureManager);
    m_bOwnTextureManager = true;
    m_pTextureManager->Init(pDevice);
  }
  m_pWorld = new CVisualWorld;
  m_pWorld->Init(pDevice, 1);
  m_pWorld->SetBucketFlags(0, CVisualWorld::TBucket::F_SORTBYMATERIAL);
//  m_pWorld->SetBucketFlags(0, CVisualWorld::TBucket::F_PRESORTED);

  m_pDevice = pDevice;

  if (RET_OK != LoadScene(pszSceneFile))
  {
    End();
    return RET_FAIL;
  }
  m_pTextureManager->Restore(m_pDevice);

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::End       ()
{
  if (IsOk())
  {
    struct T
    {
      static  void Delete(TNode&t)     { DISPOSE(t.paPosKeys); DISPOSE(t.paRotKeys); DISPOSE(t.paScaleKeys); }
      static  void Delete(TMesh&t)     { DISPOSE(t.pMesh); }
      static  void Delete(TInstance&t) { DISPOSE(t.pRenderable); }
    };

    m_aNodes           .Foreach(T::Delete);
    m_aNodes           .End();
    m_aInstances       .Foreach(T::Delete);
    m_aInstances       .End();
    for (unsigned i = 0; i < m_aTextures.GetCount(); i++)
      m_pTextureManager->ReleaseMaterial(m_aTextures[i].pTexture);
    m_aTextures        .End();
    m_aMaterials       .End();
    m_aCameras         .End();
    m_aLights          .End();
    m_aMeshes          .Foreach(T::Delete);
    m_aMeshes          .End();

    DISPOSE(m_pWorld);
    if (m_bOwnTextureManager)
      DISPOSE(m_pTextureManager);
    m_pDevice = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::LoadScene           (const char *pszFile)
{
  ASSERT(IsOk());

  CReadChunker ch;

  if (RET_OK != ch.Init(pszFile))
    return RET_FAIL;

  TError ret = RET_OK;
  while (ret == RET_OK && !ch.Eof() && strcmp(ch.ReadToken(), "General") != 0)
    ;
  ret = LoadChunkGeneral(CReadChunker(ch));

  while (ret == RET_OK && !ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.IsToken("Textures"))      ret = LoadTextures(CReadChunker(ch));
    else if (ch.IsToken("Materials"))     ret = LoadArray(m_aMaterials, CReadChunker(ch), "Material")   ;
    else if (ch.IsToken("Nodes"))         ret = LoadArray(m_aNodes, CReadChunker(ch), "Node")           ;
    else if (ch.IsToken("Instances"))     ret = LoadArray(m_aInstances, CReadChunker(ch), "Instance")   ;
    else if (ch.IsToken("Cameras"))       ret = LoadArray(m_aCameras, CReadChunker(ch), "Camera")       ;
    else if (ch.IsToken("Lights"))        ret = LoadArray(m_aLights, CReadChunker(ch), "Light")         ;
    else if (ch.IsToken("Meshes"))        ret = LoadArray(m_aMeshes, CReadChunker(ch), "Mesh")          ;
    else if (ch.IsToken("Controllers"))   ret = LoadControllers(CReadChunker(ch))                       ;

    ch.SkipValue();
  }

  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::LoadChunkGeneral    (CReadChunker &ch)
{
  int start = 0, end = 0;
  unsigned n = 0;
  int frameRate     = DEFAULT_FPS;

  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("Start",           &start))          ;
    else if (ch.ReadValue("AmbientColor",    &m_clrAmbient))   ;
    else if (ch.ReadValue("BackgroundColor", &m_clrBackground));
    else if (ch.ReadValue("FrameRate",       &frameRate))      ;
    else if (ch.ReadValue("End",             &end))                m_nFrames = end - start;
    else if (ch.ReadValue("Nodes",           &n))                  m_aNodes.Init(n);     
    else if (ch.ReadValue("Materials",       &n))                  m_aMaterials.Init(n); 
    else if (ch.ReadValue("Textures",        &n))                  m_aTextures.Init(n);  
    else if (ch.ReadValue("Instances",       &n))                  m_aInstances.Init(n); 
    else if (ch.ReadValue("Meshes",          &n))                  m_aMeshes.Init(n);    
    else if (ch.ReadValue("Cameras",         &n))                  m_aCameras.Init(n);   
    else if (ch.ReadValue("Lights",          &n))                  m_aLights.Init(n);    
    else
      ch.SkipValue();
  }
  m_framesPerSec = frameRate;
  m_nFrames = (end - start);
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
template<class T>
TError CDisplayScene::LoadArray (CDynArray<T> &a, CReadChunker &ch, const char *pszChunk)
{
  TError ret = RET_OK;
  while (ret == RET_OK && !ch.Eof())
  {
    // Read keywords of the style "keyword { chunk }"
    if (ch.IsToken(pszChunk))
    {
      T &t = a.Add();
      ret = Load(&t, CReadChunker(ch));
    }
    ch.SkipValue();
  }
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::LoadTextures  (CReadChunker &ch)
{
  char buf[500];

  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
    if (ch.ReadValue("Texture",  buf, sizeof(buf)))
      m_aTextures.Add().pTexture = m_pTextureManager->InitMaterial(buf, 0);
    else
      ch.SkipValue();
  }
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::LoadControllers (CReadChunker &ch)
{
  TError ret = RET_OK;

  while (ret == RET_OK && !ch.Eof())
  {
    // Read keywords of the style "keyword { chunk }"
    if (ch.IsToken("Controller"))
    {
      CReadChunker c(ch);
      int owner = -1, ns = 0, sr = 1;
      char buf[500] = "";
      while (!c.Eof())
      {
          // Read keywords of the style "keyword = value"
             if (c.ReadValue("OwnerID",       &owner))            ;
        else if (c.ReadValue("KeyType",       buf, sizeof(buf)))  ;
        else if (c.ReadValue("SampleRate",    &sr))               ;
        else if (c.ReadValue("NumSamples",    &ns))               ;
        else
        {
          // Read keywords of the style "keyword { chunk }"
          if (c.IsToken("Samples") && owner >= 0 && buf[0] && ns > 0)
          {
            CReadChunker sc(c);

            TNode *pNode = &m_aNodes[owner];
            if (stricmp(buf, "POS_ROT") == 0 || stricmp(buf, "POS_ROT_SCALE") == 0)
            {
              bool bScale = (stricmp(buf, "POS_ROT_SCALE") == 0);
              pNode->keysPerFrame = sr;
              pNode->numPosKeys = ns;
              pNode->numRotKeys = ns;
              pNode->numScaleKeys = bScale? ns : 0;
              ASSERT(pNode->paPosKeys == NULL);
              ASSERT(pNode->paRotKeys == NULL);
              ASSERT(pNode->paScaleKeys == NULL);
              pNode->paPosKeys = NEW_ARRAY(TVector3, ns);
              pNode->paRotKeys = NEW_ARRAY(TQuaternion, ns);
              pNode->paScaleKeys = bScale? NEW_ARRAY(TQuaternion, ns) : NULL;
              for (int i = 0; i < ns; i++)
              {
                pNode->paPosKeys[i].x = sc.ReadFloat();
                pNode->paPosKeys[i].y = sc.ReadFloat();
                pNode->paPosKeys[i].z = sc.ReadFloat();

                pNode->paRotKeys[i].x = sc.ReadFloat();
                pNode->paRotKeys[i].y = sc.ReadFloat();
                pNode->paRotKeys[i].z = sc.ReadFloat();
                pNode->paRotKeys[i].w = sc.ReadFloat();

                if (bScale)
                {
                  pNode->paScaleKeys[i].x = sc.ReadFloat();
                  pNode->paScaleKeys[i].y = sc.ReadFloat();
                  pNode->paScaleKeys[i].z = sc.ReadFloat();
                }
              }
            }
          }
          c.SkipValue();
        }
      }
    }
    ch.SkipValue();
  }
  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TMaterial *pMaterial, CReadChunker &ch)
{
  ZeroMem(pMaterial, sizeof(*pMaterial));

  int dt = -1, st = -1, ot = false;
  float sh = 0, shStr = 0, fOpacity = 1;

  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("AmbientClr",    &pMaterial->ambient))    ;
    else if (ch.ReadValue("Name",          pMaterial->szName, sizeof(pMaterial->szName)))   ;
    else if (ch.ReadValue("DiffuseClr",    &pMaterial->diffuse))    ;
    else if (ch.ReadValue("SpecularClr",   &pMaterial->specular))   ;
    else if (ch.ReadValue("DiffuseTex",    &dt))                    ;
    else if (ch.ReadValue("EnvMapTex",     &st))                    ;
    else if (ch.ReadValue("OpacityTex",    &ot))                    ;
    else if (ch.ReadValue("Shininess",     &pMaterial->fShininess)) ;
    else if (ch.ReadValue("ShnStrength",   &pMaterial->fShnStrength))  ;
    else if (ch.ReadValue("Opacity",       &fOpacity))  ;
    else
      ch.SkipValue();
  }

  pMaterial->diffuse = (pMaterial->diffuse & 0xFFFFFF) | (unsigned(fOpacity*(pMaterial->diffuse >> 24)) << 24);

  if (dt >= 0 && (unsigned)dt < m_aTextures.GetCount())
  {
    pMaterial->pDiffuseTex = &m_aTextures[dt];
    if (ot)
      pMaterial->bUsesAlpha = true;
  }
  if ((pMaterial->diffuse & 0xFF000000) != 0xFF000000)
    pMaterial->bUsesAlpha = true;
  if (st >= 0 && (unsigned)st < m_aTextures.GetCount())
    pMaterial->pEnvMap     = &m_aTextures[st];

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TNode *pNode, CReadChunker &ch)
{
  ZeroMem(pNode, sizeof(*pNode));
  pNode->initRot.x = 1;

  int parent = -1;
  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("ParentId",      &parent))            ;
    else if (ch.ReadValue("Name",          pNode->szName, sizeof(pNode->szName)))   ;
    else if (ch.ReadValue("Position",      &pNode->initPos))    ;
    else if (ch.ReadValue("Rotation",      &pNode->initRot))    ;
    else if (ch.ReadValue("Scale",         &pNode->initScale))  ;
    else
      ch.SkipValue();
  }
  pNode->pos = pNode->initPos;
  pNode->rot = pNode->initRot;
  pNode->scale = pNode->initScale;
  if (parent != -1 && (unsigned)parent < m_aNodes.GetCount())
    pNode->pParent = &m_aNodes[parent];
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TCamera *pCamera, CReadChunker &ch)
{
  ZeroMem(pCamera, sizeof(*pCamera));

  int node = -1;
  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("NodeID",        &node))          ;
    else if (ch.ReadValue("FOV",           &pCamera->fov))  ;
    else
      ch.SkipValue();
  }
  if (node != -1 && (unsigned)node < m_aNodes.GetCount())
    pCamera->pNode = &m_aNodes[node];

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TLight *pLight, CReadChunker &ch)
{
  ZeroMem(pLight, sizeof(*pLight));
  int node = -1;
  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("NodeID",        &node))                  ;
    else if (ch.ReadValue("IsOn",          &pLight->bIsOn))         ;
    else if (ch.ReadValue("Color",         &pLight->color))         ;
    else if (ch.ReadValue("Intensity",     &pLight->fPower))        ;
    else if (ch.ReadValue("DoDiffuse",     &pLight->bDoDiffuse))    ;
    else if (ch.ReadValue("DoSpecular",    &pLight->bDoSpecular))   ;
    else
      ch.SkipValue();
  }
  if (node != -1 && (unsigned)node < m_aNodes.GetCount())
    pLight->pNode = &m_aNodes[node];
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TMesh *pMesh, CReadChunker &ch)
{
  ZeroMem(pMesh, sizeof(*pMesh));
  int nv = 0, nf = 0, ns = 0;
  int is = 0;

  TError ret = RET_OK;
  while (ret == RET_OK && !ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("Name",          pMesh->szName, sizeof(pMesh->szName)))   ;
    else if (ch.ReadValue("NVerts",        &nv))                                    ;
    else if (ch.ReadValue("NFaces",        &nf))                                    ;
    else if (ch.ReadValue("NSubsets",      &ns))                                    ;
    else
    {
      // Read keywords of the style "keyword { chunk }"
      if (ch.IsToken("Subset"))
      {
        if (is == 0 && is < ns && nv > 0 && nf > 0)
        {
          ASSERT(!pMesh->pMesh);
          pMesh->pMesh = NEW(CBasicMesh);
          ret = pMesh->pMesh->Init(m_pDevice, DV_FVF_VERTEXCUV, nv, nf, sizeof(TVertexCUV), ns);
          if (RET_OK == ret)
          {
            CReadChunker c(ch);
            int mat = -1, sv = 0, nv = 0, sf = 0, nf = 0;
            while (c.ReadToken())
            {
              c.ReadValue("MatId",         &mat);
              c.ReadValue("StartV",        &sv);
              c.ReadValue("NVerts",        &nv);
              c.ReadValue("StartF",        &sf);
              c.ReadValue("NFaces",        &nf);
            }
            pMesh->pMesh->SetSection(is, sf, nf, sv, nv);
            if (mat != -1 && (unsigned)mat < m_aMaterials.GetCount())
              pMesh->pMesh->SetSectionData(is, (dword)&m_aMaterials[mat]);
          }
        }
        is++;
      }
      else if (ch.IsToken("Verts"))
      {
        CReadChunker c(ch);
        TVertexCUV *pv = (TVertexCUV*)pMesh->pMesh->LockVB();

        if (pv)
        {
          for (int i = 0; i < nv && !c.Eof(); i++)
          {
            pv[i].x  = c.ReadFloat();
            pv[i].y  = c.ReadFloat();
            pv[i].z  = c.ReadFloat();
            pv[i].nx = c.ReadFloat();
            pv[i].ny = c.ReadFloat();
            pv[i].nz = c.ReadFloat();
            pv[i].c  = c.ReadDword();
            pv[i].u  = c.ReadFloat();
            pv[i].v  = c.ReadFloat();
          }
          pMesh->pMesh->UnlockVB();
        }
      }
      else if (ch.IsToken("Faces"))
      {
        CReadChunker c(ch);
        word *pi = pMesh->pMesh->LockIB();

        if (pi)
        {
          for (int i = 0; i < nf*3 && !c.Eof(); i++)
            pi[i]  = (word)c.ReadInt();
          pMesh->pMesh->UnlockIB();
        }
      }
      ch.SkipValue();
    }
  }

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayScene::Load  (TInstance *pInstance, CReadChunker &ch)
{
  ZeroMem(pInstance, sizeof(*pInstance));
  int node = -1;
  char buf[500];
  while (!ch.Eof())
  {
      // Read keywords of the style "keyword = value"
         if (ch.ReadValue("Node",          &node))              ;
    else if (ch.ReadValue("MeshName",      buf, sizeof(buf)))   ;
    else if (ch.ReadValue("WireColor",     &pInstance->color))  ;
    else
      ch.SkipValue();
  }
  if (node != -1 && (unsigned)node < m_aNodes.GetCount())
  {
    pInstance->pNode = &m_aNodes[node];
    pInstance->pMesh = FindMesh(buf);
    pInstance->pRenderable = new CRenderableInstance(this, pInstance);
    m_pWorld->AddNode(pInstance->pRenderable, 0);
  }

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CDisplayScene::TMesh  *CDisplayScene::FindMesh(const char *pszName)
{
  for (unsigned i = 0; i < m_aMeshes.GetCount(); i++)
    if (stricmp(pszName, m_aMeshes[i].szName) == 0)
      return &m_aMeshes[i];
  return NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CDisplayScene::TNode  *CDisplayScene::FindNode(const char *pszName)
{
  for (unsigned i = 0; i < m_aNodes.GetCount(); i++)
    if (stricmp(pszName, m_aNodes[i].szName) == 0)
      return &m_aNodes[i];
  return NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CDisplayScene::TMaterial  *CDisplayScene::FindMaterial(const char *pszName)
{
  for (unsigned i = 0; i < m_aMaterials.GetCount(); i++)
    if (stricmp(pszName, m_aMaterials[i].szName) == 0)
      return &m_aMaterials[i];
  return NULL;
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::SetCurTime        (float fTime)
{
  for (unsigned i = 0; i < m_aNodes.GetCount(); i++)
  {
    TMatrix3 m;
    TNode *pNode = &m_aNodes[i];

    GetNodeAtTime(&pNode->pos, &pNode->rot, &pNode->scale, pNode, fTime);
  }

  // Other animation parameters -> fov, etc.
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::GetNodeAtTime   (TVector3 *pPos, TQuaternion *pRot, TVector3 *pScale, const TNode *pNode, float fTime) const
{
  float fFrame = fTime*m_framesPerSec;

  if (pPos)
  {
    if (!pNode->numPosKeys)
      *pPos = pNode->initPos;
    else
    {
      float kFrame = fFrame / pNode->keysPerFrame;
      int nKey = int(kFrame);
      float fPart = kFrame - float(nKey);
      int curf  = nKey % pNode->numPosKeys;
      int nextf = (nKey + 1) % pNode->numPosKeys;
      *pPos = Lerp(pNode->paPosKeys[curf], pNode->paPosKeys[nextf], fPart);
    }
  }
  if (pRot)
  {
    if (!pNode->numRotKeys)
      *pRot = pNode->initRot;
    else
    {
      float kFrame = fFrame / pNode->keysPerFrame;
      int nKey = int(kFrame);
      float fPart = kFrame - float(nKey);
      int curf  = nKey % pNode->numRotKeys;
      int nextf = (nKey + 1) % pNode->numRotKeys;
      *pRot = QuatSLERP(pNode->paRotKeys[curf], pNode->paRotKeys[nextf], fPart);
    }
  }
  if (pScale)
  {
    if (!pNode->numScaleKeys)
      *pScale = pNode->initScale;
    else
    {
      float kFrame = fFrame / pNode->keysPerFrame;
      int nKey = int(kFrame);
      float fPart = kFrame - float(nKey);
      int curf  = nKey % pNode->numScaleKeys;
      int nextf = (nKey + 1) % pNode->numScaleKeys;
      *pScale = Lerp(pNode->paScaleKeys[curf], pNode->paScaleKeys[nextf], fPart);
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayScene::Draw              (CRenderContext &rc) const
{
  if (!IsOk())
    return;

  unsigned i;

  TProjectionViewport vp = rc.GetViewport();

  // Set up thje camera to use
  if (m_aCameras.GetCount())
  {
    TMatrix3 m;
    const TNode *pNode = m_aCameras[0].pNode;

    TProjectionViewport newVp;
    newVp = vp;
    newVp.SetFOV(m_aCameras[0].fov);
    rc.SetViewport(newVp);

    TVector3 v = pNode->pos;
    TQuaternion r = pNode->rot;

    m = r.Conjugate().ToMatrix();
    m.ZeroTranslation();
      
    TVector3 iv(v*m);

    m.SetTranslation(-iv.x, -iv.y, -iv.z);
    rc.GetDevice()->SetViewTransform(m);
  }

  // Set up lighting
  unsigned totalLights = 0;
  for (i = 0; totalLights < 8u && i < m_aLights.GetCount(); i++)
  {
    const TLight &sl = m_aLights[i];
    if (sl.bIsOn)
    {
      D3DLIGHT9 l = { D3DLIGHT_POINT };
      if (sl.bDoDiffuse)
      {
        l.Diffuse.a = (sl.fPower/255.f)*Color_A(sl.color);
        l.Diffuse.r = (sl.fPower/255.f)*Color_R(sl.color);
        l.Diffuse.g = (sl.fPower/255.f)*Color_G(sl.color);
        l.Diffuse.b = (sl.fPower/255.f)*Color_B(sl.color);
      }

      if (sl.bDoSpecular)
        l.Specular = l.Diffuse;

      l.Position.x = sl.pNode->pos.x;
      l.Position.y = sl.pNode->pos.y;
      l.Position.z = sl.pNode->pos.z;
      l.Range = DEFAULT_LIGHT_RANGE;
      l.Falloff = 1;
      l.Attenuation0 = 1.f;

      rc.GetDevice()->GetDirect3DDevice()->SetLight(totalLights, &l);
      rc.GetDevice()->GetDirect3DDevice()->LightEnable(totalLights, TRUE);
      totalLights++;
    }
  }
  bool bUseLights = (totalLights != 0);
  if (totalLights)
    rc.GetDevice()->SetRS(D3DRS_AMBIENT, m_clrAmbient);

  m_pWorld->Draw(rc);
}
