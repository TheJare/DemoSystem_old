// -------------------------------------------------------------------------------------
// File:        FSFile.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _FSFILE_H_
#define _FSFILE_H_

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
class CFile
{
  friend class FileSystem;

    CFile(): m_flags(0)         { }
    ~CFile()                    { End(); }
    TError    InitFile  (FILE *f, dword flags);
    TError    InitMem   (void *pMem, unsigned size, dword flags);
    void      End       ();

    // -----------------------
    enum
    {
      F_READ    = 0x0001,
      F_WRITE   = 0x0002,
      F_TEXT    = 0x0004,

      F_ZIP     = 0x0100,   // From a ZIP file.
    };

    int       ReadTextByte  ()  { if (m_pData[m_offset] == '\r' && m_offset+1 < m_size && m_pData[m_offset+1] == '\n') { m_offset += 2; return '\n'; } return m_pData[m_offset++]; }
    int       ReadByte      ()  { if (m_f) return fgetc(m_f); if (m_offset >= m_size) return -1; if (IsText()) return ReadTextByte(); return m_pData[m_offset++]; }

  public:

    // --------------------
    // --------------------

    enum TSeekMode    // Compatible with stdio.
    {
      F_SEEK_SET,
      F_SEEK_CUR,
      F_SEEK_END,
    };

    bool    IsOk  () const        { return (this != NULL) && (m_flags != 0); }
    bool    Eof   () const        { return !IsOk() || (m_f && feof(m_f)) || (m_pData && m_offset >= m_size); }
    bool    IsMem () const        { return IsOk() && m_pData; }
    bool    IsText() const        { return (m_flags & F_TEXT) != 0; }
    bool    IsZip () const        { return (m_flags & F_ZIP) != 0; }

    int     GetLen() const        { return m_size; }

    char   *Gets  (char *pDest, int n, bool bClean = false);
    int     Read  (void *pDest, int len);
    int     Write (const void *pSrc, int len)   { return (IsOk() && m_f)? fwrite(pSrc, 1, len, m_f) : 0; }
    int     Printf(const char *pszFmt, ...);

    int     Tell  ()                                      { return IsOk()? (m_f? (int)ftell(m_f) : m_offset) : 0; }
    void    Seek  (int pos, TSeekMode mode = F_SEEK_SET);

    int     GetChar ()                  { return IsOk()? ReadByte() : -1; }
    int     GetWord ()                  { if (IsOk()) { int k = ReadByte(); k += ReadByte() << 8; return k; } return -1; }
    int     GetLong ()                  { if (IsOk()) { int k = ReadByte(); k += ReadByte() << 8; k += ReadByte() << 16; k += ReadByte() << 24; return k; } return -1; }
    float   GetFloat()                  { if (IsOk()) { int k = ReadByte(); k += ReadByte() << 8; k += ReadByte() << 16; k += ReadByte() << 24; return *(float*)&k; } return -1; }

    void    PutChar (int c)             { if (IsOk() && m_f) fputc(c, m_f); }
    void    PutWord (int c)             { if (IsOk() && m_f) { fputc(c, m_f); fputc(c >> 8, m_f); } }
    void    PutLong (int c)             { if (IsOk() && m_f) { fputc(c, m_f); fputc(c >> 8, m_f); fputc(c >> 16, m_f); fputc(c >> 24, m_f); } }

    const byte *GetMemoryBuffer (int *pLen = NULL) { if (!IsMem()) return NULL; if (pLen) *pLen = (int)m_size; return m_pData; }

  private:

    dword       m_flags;
    FILE       *m_f;
    const byte *m_pData;
    unsigned    m_offset;
    unsigned    m_size;
};

#endif // _FSFILE_H_
