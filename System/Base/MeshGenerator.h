// ----------------------------------------------------------------------------------------
// File:      MeshGenerator.h
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#ifndef _MESHGENERATOR_H_
#define _MESHGENERATOR_H_
#pragma once

#include "Vectors.h"

// --------------------

class CMeshGenerator
{
//    typedef __ Inherited;
  public:
    enum
    {
      F_NORMALS = 0x0001,
      F_MAPPING = 0x0002,
    };

    struct TVertex
    {
      TVector3 pos;
      TVector3 normal;
      TVector2 tex;
    };

    CMeshGenerator (): m_bOk(false)  { }
    ~CMeshGenerator()                { End(); }

    TError      Init      (int nVertices, int nIndices);
    void        End       ();
    bool        IsOk      ()                  const { return (m_bOk != 0); }

    void        Cube      (float size, unsigned flags = F_NORMALS);

// ------------
  private:
    bool           m_bOk;

    TVertex       *m_paVerts;
    int           *m_paIndices;
    int           m_nVertices;
    int           m_nIndices;

};

#endif //_MESHGENERATOR_H_
