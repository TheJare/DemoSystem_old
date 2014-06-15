// --------------------------------------------------------------------------------------
// Vectors.cpp
// Algebra classes
// --------------------------------------------------------------------------------------

// Notes on handedness and stuff
// Left-handed like D3D, Z is positive into the scene.
// Row Vector, the way to transform is: v' = v*M
// 
// X,Y,Z are the axes transformed into world coords
//
//                  | X,x Y.x Z.x |
// v' = [ x y z ] * | X.y Y.y Z.y | + [ T.x T.y T.z ]
//                  | X.z Y.z Z.z |
// 

#include "Vectors.h"

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TMatrix4 operator * (const TMatrix4 &_l, const TMatrix4 &_m)
{
  TMatrix4 r;
  for (int i = 0; i < 4; ++i)
  {
    r.m[i][0] =  _l.m[i][0]*_m.m[0][0] + _l.m[i][1]*_m.m[1][0] + _l.m[i][2]*_m.m[2][0] + _l.m[i][3]*_m.m[3][0];
    r.m[i][1] =  _l.m[i][0]*_m.m[0][1] + _l.m[i][1]*_m.m[1][1] + _l.m[i][2]*_m.m[2][1] + _l.m[i][3]*_m.m[3][1];
    r.m[i][2] =  _l.m[i][0]*_m.m[0][2] + _l.m[i][1]*_m.m[1][2] + _l.m[i][2]*_m.m[2][2] + _l.m[i][3]*_m.m[3][2];
    r.m[i][3] =  _l.m[i][0]*_m.m[0][3] + _l.m[i][1]*_m.m[1][3] + _l.m[i][2]*_m.m[2][3] + _l.m[i][3]*_m.m[3][3];
  }
  return r;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TMatrix4 Transpose (const TMatrix4 &_l)
{
  TMatrix4 r;
  for (int i = 0; i < 4; ++i)
  {
    r.m[i][0] =  _l.m[0][i];
    r.m[i][1] =  _l.m[1][i];
    r.m[i][2] =  _l.m[2][i];
    r.m[i][3] =  _l.m[3][i];
  }
  return r;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
const TMatrix3 &TMatrix3::SetRotationAxisAngle (TVector3 v, float a)
{
  float c = cosf(a);
  float s = sinf(a);
  float ic = 1.f - c;
  float x2 = v.x*v.x*ic;
  float y2 = v.y*v.y*ic;
  float z2 = v.z*v.z*ic;
  float xy = v.x*v.y*ic;
  float xz = v.x*v.z*ic;
  float yz = v.x*v.x*ic;
  float xs = v.x*s;
  float ys = v.y*s;
  float zs = v.z*s;

  m[0][0] = x2 + c;
  m[0][1] = xy - zs;
  m[0][2] = xz + ys;

  m[1][0] = xy + zs;
  m[1][1] = y2 + c;
  m[1][2] = yz - xs;

  m[2][0] = xz - ys;
  m[2][1] = yz + xs;
  m[2][2] = z2 + c;
  return *this;
} 

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
const TMatrix3 &TMatrix3::SetLookAt (const TVector3 &vFrom, const TVector3 &vAt, TVector3 vUp)
{
  TVector3 vView = vAt - vFrom;

  float len = vView.Mod();
  if (len < 1e-6f)
    return *this;

  vView /= len;

  float dot = vUp.DotProd(vView);
  TVector3 vLup = vUp - vView*dot;

  // If this vector has near-zero length because the input specified a bogus up vector,
  // let's try a default up vector. If still have near-zero length, resort to a different axis.
  len = vLup.Mod();
  if (len < 1e-6f)
  {
    vLup.Set(0, 1, 0) - vView*vView.y;
    len = vLup.Mod();
    if (len < 1e-6f)
    {
      vUp.Set(0, 0, 1) - vView*vView.z;
      len = vLup.Mod();
      if (len < 1e-6f)
        return *this;
    }
  }
  vLup /= len;

  TVector3 vRight = vLup.CrossProd(vView);
  m[0][0] = vRight.x; m[0][1] = vLup.x; m[0][2] = vView.x;
  m[1][0] = vRight.y; m[1][1] = vLup.y; m[1][2] = vView.y;
  m[2][0] = vRight.z; m[2][1] = vLup.z; m[2][2] = vView.z;
  m[3][0] = -vRight.DotProd(vFrom);
  m[3][1] = -vLup.DotProd(vFrom);
  m[3][2] = -vView.DotProd(vFrom);
  return *this;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TMatrix3 TMatrix3::Inverse() const
{
  TMatrix3 r;
  // Invert a 3x3 using cofactors.  This is about 8 times faster than
  // the Numerical Recipes code which uses Gaussian elimination.

  r.m[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
  r.m[1][0] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
  r.m[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
  float fDet =
      m[0][0]*r.m[0][0] +
      m[0][1]*r.m[1][0] +
      m[0][2]*r.m[2][0];
  float fInvDet = 1.0f/fDet;
  r.m[0][0] *= fInvDet;
  r.m[1][0] *= fInvDet;
  r.m[2][0] *= fInvDet;

  r.m[0][1] = (m[0][2]*m[2][1] - m[0][1]*m[2][2])*fInvDet;
  r.m[0][2] = (m[0][1]*m[1][2] - m[0][2]*m[1][1])*fInvDet;
  r.m[1][1] = (m[0][0]*m[2][2] - m[0][2]*m[2][0])*fInvDet;
  r.m[1][2] = (m[0][2]*m[1][0] - m[0][0]*m[1][2])*fInvDet;
  r.m[2][1] = (m[0][1]*m[2][0] - m[0][0]*m[2][1])*fInvDet;
  r.m[2][2] = (m[0][0]*m[1][1] - m[0][1]*m[1][0])*fInvDet;
  r.m[3][0] = -GetRow(3).DotProd(GetRow(0));
  r.m[3][1] = -GetRow(3).DotProd(GetRow(1));
  r.m[3][2] = -GetRow(3).DotProd(GetRow(2));

  return r;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TMatrix3 TMatrix3::InverseTranspose() const
{
  TMatrix3 r;
  // Invert a 3x3 using cofactors and return the transposed result.

  r.m[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
  r.m[0][1] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
  r.m[0][2] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
  float fDet =
      m[0][0]*r.m[0][0] +
      m[0][1]*r.m[0][1] +
      m[0][2]*r.m[0][2];
  float fInvDet = 1.0f/fDet;
  r.m[0][0] *= fInvDet;
  r.m[0][1] *= fInvDet;
  r.m[0][2] *= fInvDet;

  r.m[1][0] = (m[0][2]*m[2][1] - m[0][1]*m[2][2])*fInvDet;
  r.m[2][0] = (m[0][1]*m[1][2] - m[0][2]*m[1][1])*fInvDet;
  r.m[1][1] = (m[0][0]*m[2][2] - m[0][2]*m[2][0])*fInvDet;
  r.m[2][1] = (m[0][2]*m[1][0] - m[0][0]*m[1][2])*fInvDet;
  r.m[1][2] = (m[0][1]*m[2][0] - m[0][0]*m[2][1])*fInvDet;
  r.m[2][2] = (m[0][0]*m[1][1] - m[0][1]*m[1][0])*fInvDet;
  r.m[3][0] = -GetRow(3).DotProd(GetRow(0));
  r.m[3][1] = -GetRow(3).DotProd(GetRow(1));
  r.m[3][2] = -GetRow(3).DotProd(GetRow(2));

  return r;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TVector3 TMatrix3::operator * (const TVector3 &v) const
{
  return TVector3(v.DotProd(GetRow(0)) + m[3][0],
                  v.DotProd(GetRow(1)) + m[3][1],
                  v.DotProd(GetRow(2)) + m[3][2]);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
// La multiplicacion de matrices asume que los vectores se multiplican por la izquierdaa: v' = v*M
// De esta forma, se rota nuestra traslacion por la rotacion de la matriz parametro.
TMatrix3 TMatrix3::operator * (const TMatrix3 &_m) const
{
  TMatrix3 r(m[0][0]*_m.m[0][0] + m[0][1]*_m.m[1][0] + m[0][2]*_m.m[2][0],
             m[0][0]*_m.m[0][1] + m[0][1]*_m.m[1][1] + m[0][2]*_m.m[2][1],
             m[0][0]*_m.m[0][2] + m[0][1]*_m.m[1][2] + m[0][2]*_m.m[2][2],
             
             m[1][0]*_m.m[0][0] + m[1][1]*_m.m[1][0] + m[1][2]*_m.m[2][0],
             m[1][0]*_m.m[0][1] + m[1][1]*_m.m[1][1] + m[1][2]*_m.m[2][1],
             m[1][0]*_m.m[0][2] + m[1][1]*_m.m[1][2] + m[1][2]*_m.m[2][2],
             
             m[2][0]*_m.m[0][0] + m[2][1]*_m.m[1][0] + m[2][2]*_m.m[2][0],
             m[2][0]*_m.m[0][1] + m[2][1]*_m.m[1][1] + m[2][2]*_m.m[2][1],
             m[2][0]*_m.m[0][2] + m[2][1]*_m.m[1][2] + m[2][2]*_m.m[2][2],
             
             _m.m[0][0]*m[3][0] + _m.m[1][0]*m[3][1] + _m.m[2][0]*m[3][2] + _m.m[3][0],
             _m.m[0][1]*m[3][0] + _m.m[1][1]*m[3][1] + _m.m[2][1]*m[3][2] + _m.m[3][1],
             _m.m[0][2]*m[3][0] + _m.m[1][2]*m[3][1] + _m.m[2][2]*m[3][2] + _m.m[3][2]);
  return r;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TQuaternion   TMatrix3::ToQuaternion() const
{
  // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
  // article "Quaternion Calculus and Fast Animation".

  TQuaternion q;
  float t = m[0][0] + m[1][1] + m[2][2];
  if (t > 0)
  {
    // |w| > 1/2, may as well choose w > 1/2
    float r = sqrtf(1.f + t);  // 2w
    q.w = .5f*r;
    r = .5f / r;  // 1/(4w)
    q.x = (m[1][2] - m[2][1])*r;
    q.y = (m[2][0] - m[0][2])*r;
    q.z = (m[0][1] - m[1][0])*r;
  }
  else
  {
    // |w| <= 1/2
    // Select an appropriate axis. Find the largest trace
    unsigned i = 0;
    if (m[1][1] > m[i][i])
      i = 1;
    if (m[2][2] > m[i][i] )
      i = 2;
    unsigned j = (i == 2)? 0 : i+1;
    unsigned k = (j == 2)? 0 : j+1;

    float r = sqrtf(1.f + m[i][i] - m[j][j] - m[k][k]);
    q[i] = .5f*r;
    r = .5f / r;
    q.w  = (m[j][k] - m[k][j])*r;
    q[j] = (m[i][j] + m[j][i])*r;
    q[k] = (m[i][k] + m[k][i])*r;
  }
  return q;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TMatrix3   TQuaternion::ToMatrix() const
{
  TMatrix3 m;

  float xs = x+x;
  float ys = y+y ;
  float zs = z+z;
  float wx = w * xs;
  float wy = w * ys;
  float wz = w * zs;
  float xx = x * xs;
  float xy = x * ys;
  float xz = x * zs;
  float yy = y * ys;
  float yz = y * zs;
  float zz = z * zs;

  m.m[0][0] = 1.f - (yy + zz); 
  m.m[0][1] = xy + wz; 
  m.m[0][2] = xz - wy; 

  m.m[1][0] = xy - wz; 
  m.m[1][1] = 1.f - (xx + zz); 
  m.m[1][2] = yz + wx; 

  m.m[2][0] = xz + wy;
  m.m[2][1] = yz - wx;
  m.m[2][2] = 1.f - (xx + yy);
  return m;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TQuaternion QuatLERP     (const TQuaternion &q0, const TQuaternion &q1, float a)
{
  TQuaternion q;
  float c = q0.DotProd(q1);
  if (c > 0.f)
  {
    q.x = q0.x + a * (q1.x - q0.x);
    q.y = q0.y + a * (q1.y - q0.y);
    q.z = q0.z + a * (q1.z - q0.z);
    q.w = q0.w + a * (q1.w - q0.w);
  }
  else
  {
    q.x = q0.x - a * (q1.x + q0.x);
    q.y = q0.y - a * (q1.y + q0.y);
    q.z = q0.z - a * (q1.z + q0.z);
    q.w = q0.w - a * (q1.w + q0.w);
  }
  return q;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TQuaternion QuatSLERP    (TQuaternion q0, TQuaternion q1, float a)
{
  TQuaternion q;
  float w, c, s, sclp, sclq;
  
  c = q0.DotProd(q1);

  if (c < .0f)
  {
    // Flip start quaternion
    q0.x = -q0.x; q0.y = -q0.y; q0.z = -q0.z; q0.w = -q0.w;
    c = -c;
  }

  if ((1.f + c) > .01f)
  {
    // If the quaternions are close, use linear interploation
    if ((1.f - c) < .01f)
    {
      sclp = 1.f - a;
      sclq = a;
    }
    else
    {
      w = acosf(c);
      s = 1.f / sqrtf(1.f - c*c);
      sclp = s*sinf((1.f - a)*w);
      sclq = s*sinf(a*w);
    } 
  } 
  else
  {
    q1.x = -q0.y; 
    q1.y =  q0.x;
    q1.z = -q0.w; 
    q1.w =  q0.z;
    sclp = sinf((.5f - a)*3.1415926535897932384626433832795f); // This cpp is stand alone so no external constants.
    sclq = sinf(a*3.1415926535897932384626433832795f);
  }
  q.x = sclp * q0.x + sclq * q1.x;
  q.y = sclp * q0.y + sclq * q1.y;
  q.z = sclp * q0.z + sclq * q1.z;
  q.w = sclp * q0.w + sclq * q1.w;

  return q;
}
