//----------------------------------------------------------------------------
//  Nombre:    Base.h
//
//  Contenido: Definición de los tipos de datos basicos
//----------------------------------------------------------------------------

#ifndef _BASE_H_
#define _BASE_H_

#define _HAS_EXCEPTIONS 0 // Disable exceptions in std::

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------
// Tipos destinados a trabajo lógico
//----------------------------------------------------------------------------
typedef unsigned char  uchar;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned long  ulong;

//----------------------------------------------------------------------------
// Tipos destinados a uso como almacenamiento de memoria
//----------------------------------------------------------------------------
typedef unsigned char  byte;
typedef unsigned int   dword;
typedef unsigned short word;

//----------------------------------------------------------------------------
// PI & other constants, plus Radians / Degrees conversion
//----------------------------------------------------------------------------
#define PI 3.1415926535897932384626433832795f

#define HALF_PI (PI/2.f)

#define RAD_90  HALF_PI
#define RAD_180 PI
#define RAD_270 (PI*3.f/2.f)
#define RAD_30  (RAD_90/3.f)
#define RAD_45  (RAD_90/2.f)
#define RAD_60  (RAD_90*2.f/3.f)
#define RAD_120 (RAD_90*4.f/3.f)
#define RAD_135 (RAD_180*3.f/4.f)

#define SQR_2 1.4142135623730950488016887242097f
#define SQR_3 1.7320508075688772935274463415059f

#define SIN_30 0.5f
#define COS_30 0.86602540378443864676372317075294f
#define TAN_30 (SIN_30/COS_30)

#define SIN_45 SQR_2
#define COS_45 SQR_2
#define TAN_45 1.f

#define SIN_60 0.86602540378443864676372317075294f
#define COS_60 0.5f
#define TAN_60 (SIN_60/COS_60)

#define DEGTORAD(a) (float(a)/180.f*PI)
#define RADTODEG(a) (float(a)/PI*180.f)

//----------------------------------------------------------------------------
// NULL
//----------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

//----------------------------------------------------------------------------
// Macro magic
//----------------------------------------------------------------------------
#define MACRO_PASTE(a,b) a##b

//----------------------------------------------------------------------------
// Constantes para la gestión de errores
//----------------------------------------------------------------------------
enum
{
  RET_OK = 0,
  RET_FAIL,
  ERR_START
};

typedef int TError;

#define RET_SIMPLIFY(errcode) ((errcode) == RET_OK ? RET_OK : RET_FAIL)

//----------------------------------------------------------------------------
// LOG
//----------------------------------------------------------------------------
#ifdef VERSIONFINAL
  #define GLOG(a)     ((void)0)
#else
  void BaseLog(const char *fmt, ...);
  #define GLOG(a)     (BaseLog a)
#endif // VERSIONFINAL

// Messagebox simplote
void  BaseAlert (const char *pszText);
void  BaseAlertf(const char *fmt, ...);

//----------------------------------------------------------------------------
// Assert
//----------------------------------------------------------------------------
#ifdef VERSIONFINAL
  #define ASSERT(exp)       ((void)0)
  #define ASSERTM(exp,msg)  ((void)0)
#else
  void  NotifyAssertFailure(const char *sText, const char *sFile, unsigned Line);
  void  NotifyAssertFailure(const char *sText,const char *sMensaje,const char *sFile, unsigned Line);
  #define ASSERT(exp)       (void)( (exp) || (NotifyAssertFailure(#exp, __FILE__, __LINE__), 0) )
  #define ASSERTM(exp,msg)  (void)( (exp) || (NotifyAssertFailure(#exp,msg, __FILE__, __LINE__), 0) )
#endif  // VERSIONFINAL

//----------------------------------------------------------------------------
// Macros para manejo de punteros
//----------------------------------------------------------------------------

  // Memoria dinamica:
#define NEW(a)          (new a)
#define NEW_ARRAY(a,b)  (new a[b])
#define NUKE_PTR(p)             (delete (p), (p) = NULL)
#define NUKE_ARRAY_PTR(p)       (delete[] (p), (p) = NULL)
#define DISPOSE(p)           (delete (p), (p) = NULL)
#define DISPOSE_ARRAY(p)     (delete[] (p), (p) = NULL)

  // Incrementar un puntero (al tipo que sea) en un numero de bytes dado.
#define POINTER_ADD_T(t,p,a)    ( (t*) (((unsigned)(p)) + (a)) )
#define POINTER_ADD(p,a)        POINTER_ADD_T(void, p, a)

// Utiles para DX
#define SAFE_RELEASE(p)         ((p)? ((p)->Release(),(p) = NULL) : NULL)

// Placement new.
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
  inline void *operator new(size_t, void *p)  { return p; }
  inline void operator delete(void *, void *) { }
#endif

//----------------------------------------------------------------------------
// Macros para manejo de arrays
//----------------------------------------------------------------------------

// Averiguar el numero de elementos en un array (OJO: tiene que ser un array, no puede ser un puntero).
#define SIZE_ARRAY(a)           (sizeof(a)/sizeof(*(a)))
#define ARRAY_LEN(a)            (sizeof(a)/sizeof(*(a)))

//----------------------------------------------------------------------------
// Inlines para gestion numerica
//----------------------------------------------------------------------------

  // ---------
template<class T>
inline void Swap  (T &a, T &b)
//          ----
{
  T t = a; a = b; b = t;
}

  // ---------
template<class T>
inline T Max  (T a, T b)
//       ---
{
  if (a >= b)
    return a;
  return b;
}

  // ---------
template<class T>
inline T Min  (T a, T b)
//       ---
{
  if (a >= b)
    return b;
  return a;
}

  // ---------
template<class T>
inline T Clamp  (T v, T a, T b)
//       -----
{
  if (v < a)
    return a;
  if (v > b)
    return b;
  return v;
}

  // ---------
template<class T>
inline T Pow2 (T c)
//       ----
{
  return c*c;
}

  // ---------
template<class T>
inline T Lerp (T a, T b, float t)
//       ----
{
  return a + t*(b-a);
}

//----------------------------------------------------------------------------
// Vectores, ángulos, color
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Memoria
//----------------------------------------------------------------------------

inline void CopyMem( void* _destino, void const* _fuente, unsigned _bytes )
  { memcpy(_destino,_fuente,_bytes); }
  // falla si destino y fuente solapados

inline void MoveMem( void* _destino, void const* _fuente, unsigned _bytes )
  { memmove(_destino,_fuente,_bytes); }
  // funciona si destino y fuente solapados

inline void ZeroMem( void* _destino, unsigned _bytes )
  { memset(_destino,0,_bytes); }

// --------------------------------------------------------------------------
// Color
// --------------------------------------------------------------------------

typedef unsigned TColor;

__forceinline TColor MakeColor(int r, int g, int b, int a = 255) { return (a << 24) | (r << 16) | (g << 8) | b; }
__forceinline int    Color_A(TColor c) { return (c >> 24); }
__forceinline int    Color_R(TColor c) { return (c >> 16) & 0xFF; }
__forceinline int    Color_G(TColor c) { return (c >> 8)  & 0xFF; }
__forceinline int    Color_B(TColor c) { return (c     )  & 0xFF; }


#endif  //_BASE_H_
