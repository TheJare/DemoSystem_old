// ----------------------------------------------------------------------------------------
// File:      MeshGenerator.cpp
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#include "Base.h"
#include "MeshGenerator.h"
#include "vectors.h"

struct TVector3;

// --------------------
namespace
{
  char s_aCubeVtx[][3] =
  {
/* ?
    { -1, -1, -1 }, { -1, -1,  1 }, { -1,  1, -1 }, { -1,  1, -1 }, { -1, -1,  1 }, { -1,  1,  1 },
    {  1, -1, -1 }, {  1,  1, -1 }, {  1, -1,  1 }, {  1, -1,  1 }, {  1,  1, -1 }, {  1,  1,  1 },

    { -1, -1, -1 }, {  1, -1, -1 }, { -1, -1,  1 }, { -1, -1,  1 }, {  1, -1, -1 }, {  1, -1,  1 },
    { -1,  1, -1 }, { -1,  1,  1 }, {  1,  1, -1 }, {  1,  1, -1 }, { -1,  1,  1 }, {  1,  1,  1 },
  
    { -1, -1, -1 }, { -1,  1, -1 }, {  1, -1, -1 }, {  1, -1, -1 }, { -1,  1, -1 }, {  1,  1, -1 },
    { -1, -1,  1 }, {  1, -1,  1 }, { -1,  1,  1 }, { -1,  1,  1 }, {  1, -1,  1 }, {  1,  1,  1 },
*/

    { -1, -1, -1 }, { -1, -1,  1 }, { -1,  1, -1 }, { -1,  1,  1 },
    {  1, -1, -1 }, {  1,  1, -1 }, {  1, -1,  1 }, {  1,  1,  1 },

  };
  char s_aCubeIdx[][3] =
  {
    { 2, 0, 1 },  { 2, 1, 3 }, 
    { 6, 4, 5 },  { 6, 5, 7 },

    { 1, 0, 4 },  { 1, 4, 6 },
    { 5, 2, 3 },  { 5, 3, 7 },

    { 4, 0, 2 },  { 4, 2, 5 },
    { 3, 1, 6 },  { 3, 6, 7 },
  };
}

// ------------------------------------------------------------------------
// Name:       Init
// Purpose:    Initialize the object
// Parameters: 
// Returns:    RET_OK if successful, RET_FAIL otherwise
// ------------------------------------------------------------------------
TError CMeshGenerator::Init (int nVertices, int nIndices)
{
  End();
  m_paVerts   = NEW_ARRAY(TVertex, nVertices);
  m_paIndices = NEW_ARRAY(int, nIndices*3);
  m_nVertices = nVertices;
  m_nIndices  = nIndices;
  m_bOk = true;
  return RET_OK;
}

// ------------------------------------------------------------------------
// Name:       End
// Purpose:    Deinitialize the object
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void CMeshGenerator::End ()
{
  if (IsOk())
  {
    DISPOSE_ARRAY(m_paVerts);
    DISPOSE_ARRAY(m_paIndices);
    m_bOk = false;
  }
}

// ------------------------------------------------------------------------
// Name:       Cube
// Purpose:    Generate a cube
// Parameters: 
// Returns:    
// ------------------------------------------------------------------------
void CMeshGenerator::Cube      (float size, unsigned flags)
{
  Init(ARRAY_LEN(s_aCubeVtx), ARRAY_LEN(s_aCubeIdx));

/*
  int i;
  float *p = pPos;
  for (i = 0; i < ARRAY_LEN(s_aCubeVtx); i++)
  {
    p[0] = float(s_aCubeVtx[i][0]);
    p[1] = float(s_aCubeVtx[i][1]);
    p[2] = float(s_aCubeVtx[i][2]);
    if (flags & F_NORMALS)
      ((TVector3*)p)[1].Set(1, 0, 0);         // Default normal
    p = POINTER_ADD_T(float, p, stride);
  }
  int *pi = pIdx;
  const char *pOrgIdx = s_aCubeIdx[0];
  for (i = 0; i < ARRAY_LEN(s_aCubeIdx); i++)
  {
    if (pi)
    {
      pi[0] = pOrgIdx[0];
      pi[1] = pOrgIdx[1];
      pi[2] = pOrgIdx[2];
      pi += 3;
    }
    // Calc normal of each odd face and store it in the first vertex of said face
    if ((flags & F_NORMALS) && (i&1))
    {
      const TVector3 v0 = *POINTER_ADD_T(TVector3, pPos, stride*pOrgIdx[0]);
      const TVector3 v1 = *POINTER_ADD_T(TVector3, pPos, stride*pOrgIdx[1]);
      TVector3 v = v0.CrossProd(v1);
      v.Normalize();
      POINTER_ADD_T(TVector3, pPos, stride*pOrgIdx[0])[1] = v;
    }
    pOrgIdx += 3;
  }
*/
}
