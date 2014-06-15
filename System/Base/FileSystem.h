// -------------------------------------------------------------------------------------
// File:        FileSystem.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "FSFile.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
class FileSystem
{
    static void      Close       (CFile *pFile, bool bFreeMem);

  public:

    static TError    Init    (const char *pszBaseDir);
    static void      End     ();
    static bool      IsOk    ();

    static void      GetBaseDir  (char *pDest, int len);
    static void      SetBaseDir  (const char *pszBaseDir);

    static void      AddZipFiles (const char *pszDir, const char *pszExt = "ZIP", const char *pszPassword = NULL);

    static CFile    *Open        (const char *pszName, const char *pszMode = "rb");
    static void      Close       (CFile *pFile)                                     { Close(pFile, true); }

    static const void *ReadFile    (int *pSize, const char *pszName, const char *pszMode = "rb");
    static void        FreeFile    (const void *&pData);

};

#endif //_FILESYSTEM_H_
