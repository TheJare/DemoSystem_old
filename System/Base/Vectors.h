// --------------------------------------------------------------------------------------
// Vectors.h
// Algebra classes
// --------------------------------------------------------------------------------------

#ifndef _VECTORS_H_
#define _VECTORS_H_

#include <math.h>

// Notes on handedness and stuff
// Left-handed like D3D, Z is positive into the scene.
// Row Vector, the way to transform is: v' = v*M
// 
// X,Y,Z are the axes transformed into world coords
//
//                  | X.x Y.x Z.x |
// v' = [ x y z ] * | X.y Y.y Z.y | + [ T.x T.y T.z ]
//                  | X.z Y.z Z.z |
// 
// 
// 

struct TQuaternion;

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
struct TVector2
{
  float x, y;

  inline TVector2()                    { }
  inline TVector2(float _x, float _y)  { x = _x; y = _y; }

  inline const TVector2& Zero      ()                    { x = 0.f; y = 0.f; return *this; }
  inline const TVector2& Set       (float _x, float _y)  { x = _x; y = _y; return *this; }
  inline const TVector2& SetAngDist(float a, float d)    { x = d*cosf(a); y = d*sinf(a); return *this; }
  inline const TVector2& Normalize ()                    { float f = InvMod(); x *= f; y *= f; return *this; }
  inline const TVector2& Rotate    (float a)             { float c = cosf(a), s = sinf(a); float tx = x*c - y*s; y = x*s + y*c; x = tx; return *this; }
  inline const TVector2& Rotate    (float c, float s)    { float tx = x*c - y*s; y = x*s + y*c; x = tx; return *this; }

  inline operator const float *()  const { return (float*)this; }
  inline operator       float *()        { return (float*)this; }
  inline const float &operator[](unsigned i)  const { return ((float*)this)[i]; }
  inline       float &operator[](unsigned i)        { return ((float*)this)[i]; }
                                                          
  inline       TVector2 GetNormalized  () const          { float f = InvMod(); return TVector2(x * f, y * f); }

  inline       TVector2 operator + (const TVector2 &v) const   { return TVector2(x + v.x, y + v.y); }
  inline       TVector2 operator - (const TVector2 &v) const   { return TVector2(x - v.x, y - v.y); }
  inline       TVector2 operator * (float f) const             { return TVector2(x * f, y * f); }
  inline       TVector2 operator / (float f) const             { float f1 = 1.f/f; return TVector2(x * f1, y * f1); }

  friend TVector2 operator * (float f, const TVector2 &v);
  friend TVector2 operator / (float f, const TVector2 &v);

  inline const TVector2 &operator += (const TVector2 &v)   { x += v.x; y += v.y; return *this; }
  inline const TVector2 &operator -= (const TVector2 &v)   { x -= v.x; y -= v.y; return *this; }
  inline const TVector2 &operator *= (float f)             { x *= f; y *= f; return *this; }
  inline const TVector2 &operator /= (float f)             { float f1 = 1.f/f; x *= f1; y *= f1; return *this; }

  inline       bool      operator == (const TVector2 &v) const   { return x == v.x && y == v.y; }


  inline float       Angle       () const                  { return atan2f(y, x); }
  inline float       Mod2        () const                  { return x*x + y*y; }
  inline float       Mod         () const                  { return sqrtf(x*x + y*y); }
  inline float       InvMod2     () const                  { return 1.f/(x*x + y*y); }
  inline float       InvMod      () const                  { return 1.f/sqrtf(x*x + y*y); }

  inline float       DotProd     (const TVector2 &v) const { return x*v.x + y*v.y; }
  inline float       CrossProd   (const TVector2 &v) const { return x*v.y - y*v.x; }

};

// --------------------------------------------------------------------------------------

inline TVector2 operator * (float f, const TVector2 &v)  { return TVector2(v.x * f, v.y * f); }
inline TVector2 operator / (float f, const TVector2 &v)  { float f1 = 1.f/f; return TVector2(v.x * f1, v.y * f1); }

// --------------------------------------------------------------------------------------

static const TVector2 V2_Zero (0.f, 0.f);
static const TVector2 V2_XAxis(1.f, 0.f);
static const TVector2 V2_YAxis(0.f, 1.f);


// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
struct TVector3: public TVector2
{
  float z;

  inline TVector3()                             { }
  inline TVector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

  inline const TVector3& Zero      ()                             { x = 0.f; y = 0.f; z = 0.f; return *this; }
  inline const TVector3& Set       (float _x, float _y, float _z) { x = _x; y = _y; z = _z; return *this; }
  inline const TVector3& SetAngDist(float ax, float az, float d)  { float r = d*cosf(az); x = r*cosf(ax); y = r*sinf(ax); z = d*sinf(az); return *this; }
  inline const TVector3& Normalize ()                             { float f = InvMod(); x *= f; y *= f; z *= f; return *this; }
                                                          
  inline       TVector3 GetNormalized  () const          { float f = InvMod(); return TVector3(x * f, y * f, z * f); }

  inline       TVector3 operator + (const TVector3 &v) const   { return TVector3(x + v.x, y + v.y, z + v.z); }
  inline       TVector3 operator - (const TVector3 &v) const   { return TVector3(x - v.x, y - v.y, z - v.z); }
  inline       TVector3 operator * (float f) const             { return TVector3(x * f, y * f, z * f); }
  inline       TVector3 operator / (float f) const             { float f1 = 1.f/f; return TVector3(x * f1, y * f1, z * f1); }

  friend TVector3 operator * (float f, const TVector3 &v);
  friend TVector3 operator / (float f, const TVector3 &v);

//  const TVector3 &operator =(const TVector3 &v) const   { x = v.x; y = v.y; return *this; }

  inline const TVector3 &operator += (const TVector3 &v)   { x += v.x; y += v.y; z += v.z; return *this; }
  inline const TVector3 &operator -= (const TVector3 &v)   { x -= v.x; y -= v.y; z -= v.z; return *this; }
  inline const TVector3 &operator *= (float f)             { x *= f; y *= f; z *= f; return *this; }
  inline const TVector3 &operator /= (float f)             { float f1 = 1.f/f; x *= f1; y *= f1; z *= f1; return *this; }

  inline       bool      operator == (const TVector3 &v) const   { return x == v.x && y == v.y && z == v.z; }


  inline float       Mod2        () const                  { return x*x + y*y + z*z; }
  inline float       Mod         () const                  { return sqrtf(x*x + y*y + z*z); }
  inline float       InvMod2     () const                  { return 1.f/(x*x + y*y + z*z); }
  inline float       InvMod      () const                  { return 1.f/sqrtf(x*x + y*y + z*z); }

  inline float       DotProd     (const TVector3 &v) const { return x*v.x + y*v.y + z*v.z; }
  inline TVector3    CrossProd   (const TVector3 &v) const { return TVector3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }

};


inline TVector3 operator * (float f, const TVector3 &v)  { return TVector3(v.x * f, v.y * f, v.z * f); }
inline TVector3 operator / (float f, const TVector3 &v)  { float f1 = 1.f/f; return TVector3(v.x * f1, v.y * f1, v.z * f1); }

static const TVector3 V3_Zero (0.f, 0.f, 0.f);
static const TVector3 V3_XAxis(1.f, 0.f, 0.f);
static const TVector3 V3_YAxis(0.f, 1.f, 0.f);
static const TVector3 V3_ZAxis(0.f, 0.f, 1.f);

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
struct TVector4: public TVector3
{
  float w;

  inline TVector4()                             { }
  inline TVector4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

  inline const TVector4& Zero      ()                                       { x = 0.f; y = 0.f; z = 0.f; w = 0.f; return *this; }
  inline const TVector4& Set       (float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; return *this; }
  inline const TVector4& Normalize ()                                       { float f = InvMod(); x *= f; y *= f; z *= f; w *= f; return *this; }

  inline       TVector4 GetNormalized  () const          { float f = InvMod(); return TVector4(x * f, y * f, z * f, w * f); }

  inline       TVector4 operator + (const TVector4 &v) const   { return TVector4(x + v.x, y + v.y, z + v.z, w + v.w); }
  inline       TVector4 operator - (const TVector4 &v) const   { return TVector4(x - v.x, y - v.y, z - v.z, w - v.w); }
  inline       TVector4 operator * (float f) const             { return TVector4(x * f, y * f, z * f, w * f); }
  inline       TVector4 operator / (float f) const             { float f1 = 1.f/f; return TVector4(x * f1, y * f1, z * f1, w * f1); }

  friend TVector4 operator * (float f, const TVector4 &v);
  friend TVector4 operator / (float f, const TVector4 &v);

  inline const TVector4 &operator += (const TVector4 &v)   { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
  inline const TVector4 &operator -= (const TVector4 &v)   { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
  inline const TVector4 &operator *= (float f)             { x *= f; y *= f; z *= f; w *= f; return *this; }
  inline const TVector4 &operator /= (float f)             { float f1 = 1.f/f; x *= f1; y *= f1; z *= f1; w *= f1; return *this; }

  inline       bool      operator == (const TVector4 &v) const   { return x == v.x && y == v.y && z == v.z && w == v.w; }


  inline float       Mod2        () const                  { return x*x + y*y + z*z + w*w; }
  inline float       Mod         () const                  { return sqrtf(x*x + y*y + z*z + w*w); }
  inline float       InvMod2     () const                  { return 1.f/(x*x + y*y + z*z + w*w); }
  inline float       InvMod      () const                  { return 1.f/sqrtf(x*x + y*y + z*z + w*w); }

  inline float       DotProd     (const TVector4 &v) const { return x*v.x + y*v.y + z*v.z + w*v.w; }

};


inline TVector4 operator * (float f, const TVector4 &v)  { return TVector4(v.x * f, v.y * f, v.z * f, v.w * f); }
inline TVector4 operator / (float f, const TVector4 &v)  { float f1 = 1.f/f; return TVector4(v.x * f1, v.y * f1, v.z * f1, v.w * f1); }

static const TVector4 V4_Zero (0.f, 0.f, 0.f, 0.f);
static const TVector4 V4_XAxis(1.f, 0.f, 0.f, 0.f);
static const TVector4 V4_YAxis(0.f, 1.f, 0.f, 0.f);
static const TVector4 V4_ZAxis(0.f, 0.f, 1.f, 0.f);
static const TVector4 V4_WAxis(0.f, 0.f, 0.f, 1.f);

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
union TMatrix4
{
  float m[4][4];
  struct
  {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
  };
};

TMatrix4 operator * (const TMatrix4 &_l, const TMatrix4 &_m);
TMatrix4 Transpose(const TMatrix4 &_l);

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
struct TMatrix3
{
  float m[4][3];

  inline TMatrix3()                                   { }
  inline TMatrix3(float m00, float m01, float m02,
                  float m10, float m11, float m12,
                  float m20, float m21, float m22,
                  float m30, float m31, float m32)
  {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32;
  }
  inline TMatrix3(float m00, float m01, float m02,
                  float m10, float m11, float m12,
                  float m20, float m21, float m22)
  {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
  }

  inline const TMatrix3 &SetTranslation   (const TVector3 &v)         { m[3][0] = v.x; m[3][1] = v.y; m[3][2] = v.z; return *this; }
  inline const TMatrix3 &SetTranslation   (float x, float y, float z) { m[3][0] = x; m[3][1] = y; m[3][2] = z; return *this; }
  inline const TMatrix3 &ZeroTranslation  ()                          { m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; return *this; }

  inline const TMatrix3 &SetRotationX     (float a)
  {
    float c = float(cos(a));
    float s = float(sin(a)); 
    m[0][0] =  1; m[0][1] =  0; m[0][2] =  0;
    m[1][0] =  0; m[1][1] =  c; m[1][2] =  s;
    m[2][0] =  0; m[2][1] = -s; m[2][2] =  c;
    return *this;
  }
  inline const TMatrix3 &SetRotationY     (float a)
  {
    float c = cosf(a);
    float s = sinf(a); 
    m[0][0] =  c; m[0][1] =  0; m[0][2] = -s;
    m[1][0] =  0; m[1][1] =  1; m[1][2] =  0;
    m[2][0] =  s; m[2][1] =  0; m[2][2] =  c;
    return *this;
  }
  inline const TMatrix3 &SetRotationZ     (float a)
  {
    float c = cosf(a);
    float s = sinf(a); 
    m[0][0] =  c; m[0][1] =  s; m[0][2] =  0;
    m[1][0] = -s; m[1][1] =  c; m[1][2] =  0;
    m[2][0] =  0; m[2][1] =  0; m[2][2] =  1;
    return *this;
  }

  inline const TMatrix3 &SetRotationXY    (float x, float y)
  {
    float cx = cosf(x);
    float sx = sinf(x); 
    float cy = cosf(y);
    float sy = sinf(y); 

    m[0][0] =     cy; m[0][1] =      0; m[0][2] = -sy;
    m[1][0] =  sx*sy; m[1][1] =     cx; m[1][2] =  sx*cy;
    m[2][0] =  cx*sy; m[2][1] =    -sx; m[2][2] =  cx*cy;

    return *this;
  }

  inline const TMatrix3 &SetRotationYX    (float x, float y)
  {
    float cx = cosf(x);
    float sx = sinf(x); 
    float cy = cosf(y);
    float sy = sinf(y); 

    m[0][0] =     cy; m[0][1] =  sy*sx; m[0][2] = -sy*cx;
    m[1][0] =      0; m[1][1] =     cx; m[1][2] =     sx;
    m[2][0] =     sy; m[2][1] = -cy*sx; m[2][2] =  cy*cx;

    return *this;
  }

  inline const TMatrix3 &SetScale         (float x, float y, float z)
  {
    m[0][0] =  x; m[0][1] =  0; m[0][2] =  0;
    m[1][0] =  0; m[1][1] =  y; m[1][2] =  0;
    m[2][0] =  0; m[2][1] =  0; m[2][2] =  z;
    return *this;
  }

  inline float Determinant() const
  {
    float c0 = m[1][1]*m[2][2] - m[1][2]*m[2][1];
    float c1 = m[1][2]*m[2][0] - m[1][0]*m[2][2];
    float c2 = m[1][0]*m[2][1] - m[1][1]*m[2][0];
    
    return m[0][0]*c0 + m[0][1]*c1 + m[0][2]*c2;
  }

  const TMatrix3 &SetRotationAxisAngle (TVector3 v, float a);
  const TMatrix3 &SetLookAt (const TVector3 &vFrom, const TVector3 &vAt, TVector3 vUp);

  // Invert a 3x3 using cofactors.  This is about 8 times faster than
  // the Numerical Recipes code which uses Gaussian elimination.
  TMatrix3 Inverse() const;
  TMatrix3 InverseTranspose() const;

  friend       TVector3 operator * (const TVector3 &v, const TMatrix3 &m);
               TVector3 operator * (const TVector3 &v) const;

  // La multiplicacion de matrices asume que los vectores se multiplican por la izquierdaa: v' = v*M
  // De esta forma, se rota nuestra traslacion por la rotacion de la matriz parametro.
               TMatrix3 operator * (const TMatrix3 &_m) const;

  inline       TVector3  &GetRow(unsigned i)           { return *(TVector3*)m[i]; }
  inline const TVector3  &GetRow(unsigned i)     const { return *(TVector3*)m[i]; }
  inline       TVector3  &operator[](unsigned i)       { return *(TVector3*)m[i]; }
  inline const TVector3  &operator[](unsigned i) const { return *(TVector3*)m[i]; }

  // Convert to 4x4 Direct3D-style matrix.
  inline void            ToMatrix4 (TMatrix4 &om) const
  {
    om._11 = m[0][0]; om._12 = m[0][1]; om._13 = m[0][2]; om._14 = 0;
    om._21 = m[1][0]; om._22 = m[1][1]; om._23 = m[1][2]; om._24 = 0;
    om._31 = m[2][0]; om._32 = m[2][1]; om._33 = m[2][2]; om._34 = 0;
    om._41 = m[3][0]; om._42 = m[3][1]; om._43 = m[3][2]; om._44 = 1;
  }

  // Convert to Transposed 4x4 Direct3D-style matrix.
  inline void            ToMatrix4Transpose (TMatrix4 &om) const
  {
    om._11 = m[0][0]; om._12 = m[1][0]; om._13 = m[2][0]; om._14 = m[3][0];
    om._21 = m[0][1]; om._22 = m[1][1]; om._23 = m[2][1]; om._24 = m[3][1];
    om._31 = m[0][2]; om._32 = m[1][2]; om._33 = m[2][2]; om._34 = m[3][2];
    om._41 = 0;       om._42 = 0;       om._43 = 0;       om._44 = 1;
  }

  TQuaternion   ToQuaternion() const;

};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
inline TVector3 operator * (const TVector3 &v, const TMatrix3 &m)
{
  return TVector3(v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + m.m[3][0],
                  v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + m.m[3][1],
                  v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + m.m[3][2]);
}

static const TMatrix3 M3_Identity(1.f, 0.f, 0.f,   0.f, 1.f, 0.f,   0.f, 0.f, 1.f,   0, 0, 0);


// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
struct TQuaternion: public TVector3
{
  float w;

  inline TQuaternion()                                       { }
  inline TQuaternion(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
  inline TQuaternion(const TVector3 &v, float a)             { Set(v, a); }

  inline const TQuaternion& Zero      ()                                        { x = 1.f; y = 0.f; z = 0.f; w = 0.f; return *this; }
  inline const TQuaternion& Normalize ()                                        { float f = InvMod(); x *= f; y *= f; z *= f; w *= f; return *this; }
  inline const TQuaternion& Negate    ()                                        { x = -x; y = -y; z = -z; w = -w; return *this; }
  inline const TQuaternion& Conjugate ()                                        { x = -x; y = -y; z = -z;         return *this; }
  inline const TQuaternion& Set       (float _x, float _y, float _z, float _w)  { x = _x; y = _y; z = _z; w = _w; return *this; }
  inline const TQuaternion& Set       (const TVector3 &v, float a)
  {
    float a2 = .5f*a;
    float s = sinf(a2);
    w       = cosf(a2);
    x = v.x*s;
    y = v.y*s;
    z = v.z*s;
    return *this;
  }
                                                          
  inline       TQuaternion GetNormalized  () const                  { float f = InvMod(); return TQuaternion(x * f, y * f, z * f, w * f); }
  inline       TQuaternion GetNegated     () const                  { return TQuaternion(x, y, z, -w); }

  inline       float       DotProd    (const TQuaternion &o) const  { return x*o.x + y*o.y + z*o.z + w*o.w; }

  inline       TQuaternion operator * (const TQuaternion &o) const
  {
    return TQuaternion( o.x*w + o.y*z - o.z*y + o.w*x,
                       -o.x*z + o.y*w + o.z*x + o.w*y,
                        o.x*y - o.y*x + o.z*w + o.w*z,
                       -o.x*x - o.y*y - o.z*z + o.w*w);
  }

  inline const TQuaternion& operator *= (const TQuaternion &o)
  {
    TQuaternion k = *this;
    *this = k*o;
    return *this;
  }

  TMatrix3   ToMatrix() const;

  inline float       Mod2        () const                  { return x*x + y*y + z*z + w*w; }
  inline float       Mod         () const                  { return sqrtf(x*x + y*y + z*z + w*w); }
  inline float       InvMod2     () const                  { return 1.f/(x*x + y*y + z*z + w*w); }
  inline float       InvMod      () const                  { return 1.f/sqrtf(x*x + y*y + z*z + w*w); }

  friend TQuaternion QuatLERP     (const TQuaternion &q0, const TQuaternion &q1, float a);
  friend TQuaternion QuatSLERP    (TQuaternion q0, TQuaternion q1, float a);

};

static const TQuaternion Q_Identity(0.f, 0.f, 0.f, 1.f);

#endif //_VECTORS_H_
