// --------------------------------------------------------------------------
// File:        ZipFile.cpp
//
// Purpose:     The implementation of a quick'n dirty ZIP file reader class.
//              (C) Copyright 2000 Javier Arevalo. Use and modify as you like
//              Get zlib from http://www.cdrom.com/pub/infozip/zlib/
// --------------------------------------------------------------------------

#include "Base.h"
#include "ZipFile.h"
#include "zlib.h"
#include <string.h>


// --------------------------------------------------------------------------
// ZIP file structures. Note these have to be packed.
// --------------------------------------------------------------------------

#pragma pack(2)
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct CZipFile::TZipLocalHeader
{
  enum
  {
    SIGNATURE = 0x04034b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,

    FLAG_ENCRYPTED = 0x0001,
  };
  dword   sig;
  word    version;
  word    flag;
  word    compression;      // COMP_xxxx
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;
  dword   ucSize;
  word    fnameLen;         // Filename string follows header.
  word    xtraLen;          // Extra field follows filename.
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct CZipFile::TZipDirHeader
{
  enum
  {
    SIGNATURE = 0x06054b50,
  };
  dword   sig;
  word    nDisk;
  word    nStartDisk;
  word    nDirEntries;
  word    totalDirEntries;
  dword   dirSize;
  dword   dirOffset;
  word    cmntLen;
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
struct CZipFile::TZipDirFileHeader
{
  enum
  {
    SIGNATURE   = 0x02014b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,

    FLAG_ENCRYPTED = 0x0001,
  };
  dword   sig;
  word    verMade;
  word    verNeeded;
  word    flag;
  word    compression;      // COMP_xxxx
  word    modTime;
  word    modDate;
  dword   crc32;
  dword   cSize;            // Compressed size
  dword   ucSize;           // Uncompressed size
  word    fnameLen;         // Filename string follows header.
  word    xtraLen;          // Extra field follows filename.
  word    cmntLen;          // Comment field follows extra field.
  word    diskStart;
  word    intAttr;
  dword   extAttr;
  dword   hdrOffset;

  char *GetName   () const { return (char *)(this + 1);   }
  char *GetExtra  () const { return GetName() + fnameLen; }
  char *GetComment() const { return GetExtra() + xtraLen; }
};

#pragma pack()

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------


namespace
{

  const dword crc_table[256] =
  {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
  };

  __forceinline dword CRC32(dword c, byte b)
  {
    return crc_table[byte(c^b)] ^ (c >> 8);
  }

  void update_keys(dword Keys[3], byte c)
  {
    Keys[0] = CRC32(Keys[0], c);
    Keys[1] = (Keys[1] + byte(Keys[0]))*134775813 + 1;
    Keys[2] = CRC32(Keys[2], Keys[1] >> 24);
  }

  byte __forceinline decrypt_byte(dword Keys[3], byte c)
  {
    dword temp = word(Keys[2]) | 2;
    byte res = byte((temp * (temp ^ 1)) >> 8) ^ c;
    update_keys(Keys, res);
    return res;
  }



};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object and read the zip file directory.
// Parameters:    A stdio FILE* used for reading.
// --------------------------------------------------------------------------
TError CZipFile::Init(FILE *f, const char *pszPassword)
{
  End();
  if (f == NULL)
    return RET_FAIL;

  // Assuming no extra comment at the end, read the whole end record.
  TZipDirHeader dh;

  fseek(f, -(int)sizeof(dh), SEEK_END);
  long dhOffset = ftell(f);
  memset(&dh, 0, sizeof(dh));
  fread(&dh, sizeof(dh), 1, f);

  // Check
  if (dh.sig != TZipDirHeader::SIGNATURE)
    return RET_FAIL;

  // Go to the beginning of the directory.
  fseek(f, dhOffset - dh.dirSize, SEEK_SET);

  // Allocate the data buffer, and read the whole thing.
  m_pDirData = new char[dh.dirSize + dh.nDirEntries*sizeof(*m_papDir)];
  if (!m_pDirData)
    return RET_FAIL;
  memset(m_pDirData, 0, dh.dirSize + dh.nDirEntries*sizeof(*m_papDir));
  fread(m_pDirData, dh.dirSize, 1, f);

  // Now process each entry.
  char *pfh = m_pDirData;
  m_papDir = (const TZipDirFileHeader **)(m_pDirData + dh.dirSize);

  TError ret = RET_OK;

  for (int i = 0; i < dh.nDirEntries && ret == RET_OK; i++)
  {
    TZipDirFileHeader &fh = *(TZipDirFileHeader*)pfh;

    // Store the address of nth file for quicker access.
    m_papDir[i] = &fh;

    // Check the directory entry integrity.
    if (fh.sig != TZipDirFileHeader::SIGNATURE)
      ret = RET_FAIL;
    else
    {
      pfh += sizeof(fh);

      // Convert UNIX slashes to DOS backlashes.
      for (int j = 0; j < fh.fnameLen; j++)
        if (pfh[j] == '/')
          pfh[j] = '\\';

      // Skip name, extra and comment fields.
      pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
    }
  }
  if (ret != RET_OK)
    delete[] m_pDirData;
  else
  {
    m_nEntries = dh.nDirEntries;
    m_f = f;

    m_aEncryptionKeys[0] = 305419896L;
    m_aEncryptionKeys[1] = 591751049L;
    m_aEncryptionKeys[2] = 878082192L;
    if (pszPassword && *pszPassword)
    {
      while (*pszPassword)
      {
        update_keys(m_aEncryptionKeys, byte(*pszPassword));
        pszPassword++;
      }
    }
  }

  return ret;
}

// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Finish the object
// Parameters:    
// --------------------------------------------------------------------------
void CZipFile::End()
{
  if (IsOk())
  {
    delete[] m_pDirData;
    m_nEntries = 0;
  }
}

// --------------------------------------------------------------------------
// Function:      GetFilename
// Purpose:       Return the name of a file
// Parameters:    The file index and the buffer where to store the filename
// --------------------------------------------------------------------------
void CZipFile::GetFilename(int i, char *pszDest)  const
{
  if (pszDest != NULL)
  {
    if (i < 0 || i >= m_nEntries)
      *pszDest = '\0';
    else
    {
      memcpy(pszDest, m_papDir[i]->GetName(), m_papDir[i]->fnameLen);
      pszDest[m_papDir[i]->fnameLen] = '\0';
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CZipFile::IsFilename    (int i, const char *pszName, int len) const
{
  if (pszName == NULL || i < 0 || i >= m_nEntries)
    return false;
  if (len < 0)
    len = strlen(pszName);
  return (len == m_papDir[i]->fnameLen && strnicmp(pszName, m_papDir[i]->GetName(), m_papDir[i]->fnameLen) == 0 && strlen(pszName) == m_papDir[i]->fnameLen);
}
    
// --------------------------------------------------------------------------
// Function:      GetFileLen
// Purpose:       Return the length of a file so a buffer can be allocated
// Parameters:    The file index.
// --------------------------------------------------------------------------
int CZipFile::GetFileLen(int i) const
{
  if (i < 0 || i >= m_nEntries)
    return -1;
  else
    return m_papDir[i]->ucSize;
}

// --------------------------------------------------------------------------
// Function:      ReadFile
// Purpose:       Uncompress a complete file
// Parameters:    The file index and the pre-allocated buffer
// --------------------------------------------------------------------------
TError CZipFile::ReadFile(int i, void *pBuf)
{
  if (pBuf == NULL || i < 0 || i >= m_nEntries)
    return RET_FAIL;

  // Quick'n dirty read, the whole file at once.
  // Ungood if the ZIP has huge files inside

  // Go to the actual file and read the local header.
  fseek(m_f, m_papDir[i]->hdrOffset, SEEK_SET);
  TZipLocalHeader h;

  memset(&h, 0, sizeof(h));
  fread(&h, sizeof(h), 1, m_f);
  if (h.sig != TZipLocalHeader::SIGNATURE)
    return RET_FAIL;

  // Skip extra fields
  fseek(m_f, h.fnameLen + h.xtraLen, SEEK_CUR);

  TError ret = RET_OK;

  // Init encryption stuff
  dword aKeys[3] = { m_aEncryptionKeys[0], m_aEncryptionKeys[1], m_aEncryptionKeys[2] };

  // Read encryption header.
  if (h.flag & TZipLocalHeader::FLAG_ENCRYPTED)
  {
    byte c;

    for (int i = 0; i < 12; i++)
      c = decrypt_byte(aKeys, (byte)fgetc(m_f));

    if (c != byte(h.crc32 >> 24))
      return RET_FAIL;
  }

  // Check compression method
  if (h.compression == TZipLocalHeader::COMP_STORE)
  {
    // Simply read in raw stored data.
    if (h.flag & TZipLocalHeader::FLAG_ENCRYPTED)
    {
      byte *pData = (byte*)pBuf;
      for (unsigned i = 0; i < h.cSize; i++)
        pData[i] = decrypt_byte(aKeys, (byte)fgetc(m_f));
    }
    else
      fread(pBuf, h.cSize, 1, m_f);

    return RET_OK;
  }
  else if (h.compression != TZipLocalHeader::COMP_DEFLAT)
    return RET_FAIL;

  // Alloc compressed data buffer and read the whole stream
  char *pcData = new char[h.cSize];
  if (!pcData)
    return RET_FAIL;

  memset(pcData, 0, h.cSize);

  // Read in raw data.
  if (h.flag & TZipLocalHeader::FLAG_ENCRYPTED)
  {
    byte *pData = (byte*)pcData;
    for (unsigned i = 0; i < h.cSize; i++)
      pData[i] = decrypt_byte(aKeys, (byte)fgetc(m_f));
  }
  else
    fread(pcData, h.cSize, 1, m_f);

  // Setup the inflate stream.
  z_stream stream;
  int err;

  stream.next_in = (Bytef*)pcData;
  stream.avail_in = (uInt)h.cSize;
  stream.next_out = (Bytef*)pBuf;
  stream.avail_out = h.ucSize;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  // Perform inflation. wbits < 0 indicates no zlib header inside the data.
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err == Z_OK)
  {
    err = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (err == Z_STREAM_END)
      err = Z_OK;
    inflateEnd(&stream);
  }
  if (err != Z_OK)
    ret = RET_FAIL;

  delete[] pcData;
  return ret;
}
