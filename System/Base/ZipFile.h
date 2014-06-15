// --------------------------------------------------------------------------
// File:        ZipFile.h
//
// Purpose:     The declaration of a quick'n dirty ZIP file reader class.
//              (C) Copyright 2000 Javier Arevalo. Use and modify as you like
//              Get zlib from http://www.cdrom.com/pub/infozip/zlib/
// --------------------------------------------------------------------------

#ifndef _ZIPFILE_H_
#define _ZIPFILE_H_

#include <stdio.h>

class CZipFile
{
  public:

    CZipFile    (): m_nEntries(0) { }
    ~CZipFile   ()                { End(); }

    TError  Init          (FILE *f, const char *pszPassword = NULL);
    void    End           ();
    bool    IsOk          ()         const { return (m_nEntries != 0); }

    FILE   *GetFile       ()         const { return IsOk()? m_f : NULL; }

    int     GetNumFiles   ()         const { return m_nEntries; }

    bool    IsFilename    (int i, const char *pszName, int len = -1) const;
    void    GetFilename   (int i, char *pszDest) const;
    int     GetFileLen    (int i) const;

    TError  ReadFile      (int i, void *pBuf);

  private:

    struct TZipDirHeader;
    struct TZipDirFileHeader;
    struct TZipLocalHeader;

    FILE                     *m_f;
    char                     *m_pDirData; // Raw data buffer.
    int                       m_nEntries; // Number of entries.

    // Pointers to the dir entries in pDirData.
    const TZipDirFileHeader **m_papDir;   

    dword m_aEncryptionKeys[3];
};

#endif // _ZIPFILE_H_
