// -------------------------------------------------------------------------------------
// File:        FSFile.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "Base.h"
#include "FSFile.h"
#include "StrUtil.h"
#include <stdarg.h>

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CFile::InitFile  (FILE *f, dword flags)
{
  End();
  if (!f)
    return RET_FAIL;
  m_pData = NULL;
  long pos = ftell(f);
  fseek(f, 0, SEEK_END);
  m_size = ftell(f);
  fseek(f, pos, SEEK_SET);
  m_f = f;
  m_flags = flags;
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CFile::InitMem (void *pMem, unsigned size, dword flags)
{
  End();
  if (!pMem)
    return RET_FAIL;
  m_pData = (const byte*)pMem;
  m_size = size;
  m_offset = 0;
  m_f = NULL;
  m_flags = F_READ | (flags & (F_ZIP | F_TEXT));
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CFile::End     ()
{
  m_flags = 0;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CFile::Seek  (int pos, TSeekMode mode)
{
  if (!IsOk())
    return;
  if (m_f)
    fseek(m_f, pos, (int)mode);
  else
  {
    int newpos;
    if (mode == F_SEEK_SET)
      newpos = pos;
    else if (mode == F_SEEK_CUR)
      newpos = pos + m_offset;
    else // F_SEEK_END
      newpos = -pos + m_size;
    m_offset = (unsigned)Clamp(newpos, 0, (int)m_size);
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
char   *CFile::Gets  (char *pDest, int n, bool bClean)
{
  *pDest = '\0';
  do
  {
    if (Eof())
      return NULL;

    if (m_f)
      fgets(pDest, n, m_f);
    else
    {
      char *pOrg = pDest;
      if (IsText())
        while (n > 1 && m_offset < m_size)
        {
          int c = ReadTextByte();
          *pOrg++ = c;
          if (c == '\n')
            break;
          n--;
        }
      else
        while (n > 1 && m_offset < m_size)
        {
          int c = m_pData[m_offset++];
          *pOrg++ = c;
          if (c == '\n')
            break;
          n--;
        }
      *pOrg = '\0';
    }
    if (bClean)
      StrUtil::CleanLine(pDest, pDest);
  }
  while (bClean && pDest[0] == '\0');

  return pDest;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int CFile::Read  (void *pDest, int len)
{
  if (Eof())
    return 0;
  if (m_f)
    return fread(pDest, 1, len, m_f);

  int toRead;
  if (IsText())
  {
    char *pBuf = (char*)pDest;
    toRead = 0;
    while (toRead < len && m_offset < m_size)
    {
      *pBuf++ = (char)ReadTextByte();
      toRead++;
    }
  }
  else
  {
    toRead = Min(len, int(m_size - m_offset));
    memcpy(pDest, m_pData + m_offset, toRead);
    m_offset += toRead;
  }
  return toRead;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int CFile::Printf(const char *pszFmt, ...)
{
  if (!IsOk() || !m_f)
    return 0;

  va_list vl;

  va_start(vl, pszFmt);
  int r = vfprintf(m_f, pszFmt, vl);
  va_end(vl);

  return r;
}

