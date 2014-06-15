// -------------------------------------------------------------------------------------
// File:        ReadChunker.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "Base.h"
#include "ReadChunker.h"
#include "StrUtil.h"
#include "vectors.h"

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CReadChunker::CReadChunker        (const CReadChunker &ch, bool bAlreadyIn)
{
  ASSERT(ch.IsOk());
  m_pStr = NULL;
  Init(ch, bAlreadyIn);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CReadChunker::Init        (const char *pszName)
{
  End();

  FILE *f = fopen(pszName, "rt");
  if (!f)
    return RET_FAIL;
  fseek(f, 0, SEEK_END);
  int len = (int)ftell(f);
  fseek(f, 0, SEEK_SET);
  char *pData = 0;
  TError ret = RET_OK;
  if (len > 0)
  {
    pData = new char[len];

    if (!pData)
      ret = RET_FAIL;
    else
      len = (int)fread(pData, 1, len, f);
  }
  if (len <= 0)
    ret = RET_FAIL;
  fclose(f);

  if (ret == RET_OK)
    m_pStr = (TStrInfo*)NEW_ARRAY(char, sizeof(*m_pStr)-sizeof(m_pStr->szStr) + len+1);

  if (!m_pStr)
    ret = RET_FAIL;
  else
  {
    m_pStr->pszCursor = m_pStr->szStr;
    m_pStr->nRef = 1;
    m_level = 1;
    m_szToken[0] = 0;
    m_bRead = false;

    StrUtil::SafeStrncpy(m_pStr->szStr, (const char *)pData, len+1);
    StrUtil::CleanLine(m_pStr->szStr, m_pStr->szStr);
  }
  delete[] pData;
  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CReadChunker::Init        (const CReadChunker &ch, bool bAlreadyIn)
{
  End();

  m_pStr = ch.m_pStr;
  m_pStr->nRef++;
  m_level = bAlreadyIn? 1 : 0;
  m_szToken[0] = 0;
  m_bRead = false;

  while ( m_level == 0 && ReadToken())
    ;
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CReadChunker::End         ()
{
  if (IsOk())
  {
    while (m_level > 0 && ReadToken())
      m_bRead = false;
    if (--m_pStr->nRef == 0)
      DISPOSE_ARRAY(m_pStr);
    m_pStr = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
const char *CReadChunker::ReadToken   ()
{
  if (!IsOk())
    return NULL;

  bool bFound = m_bRead;
  while (!bFound && m_level >= 0)
  {
    if (!StrUtil::GetToken(m_szToken, sizeof(m_szToken), m_pStr->pszCursor))
      m_level = -1;     // EOF
    if (strcmp(m_szToken, "{") == 0)
    {
      m_level++;
      if (m_level < 2)
        bFound = true;
      m_bRead = false;
    }
    else if (strcmp(m_szToken, "}") == 0)
    {
      m_level--;
      if (m_level == 0)
        m_level = -1;     // EOF
    }
    else if (m_level < 2)
    {
      m_bRead = true;
      bFound = true;
    }
  }

  return (m_level < 0)? NULL : m_szToken;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CReadChunker::SkipValue   ()
{
  ReadTokenDiscard();      // Skip keyword
  ReadToken();      // Get next token
  if (IsToken("="))
  {
      // "keyword = value" mode
    m_bRead = false;
    ReadTokenDiscard();    // Skip rhs value
  }
  else if (IsToken("{"))
  {
      // "keyword { chunk }" mode
    m_bRead = false;
    CReadChunker c(*this, true);      // Skip entire chunk
    c.End();
//    ReadTokenDiscard();
  }
  // else, plain "keyword" mode
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, bool *pBool)
{
  if (IsToken(pszTok) && ReadTokenDiscard() && IsToken("="))
  {
    m_bRead = false;
    ReadTokenDiscard();
    if (pBool)
    {
      int i;
      sscanf(m_szToken, "%i", &i);
      *pBool = (i != 0);
    }
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, int *pInt)
{
  if (IsToken(pszTok) && ReadTokenDiscard() && IsToken("="))
  {
    m_bRead = false;
    ReadTokenDiscard();
    if (pInt)
      sscanf(m_szToken, "%i", pInt);
    return true;
  }
  return false;
}
// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, float *pFloat)
{
  if (IsToken(pszTok) && ReadTokenDiscard() && IsToken("="))
  {
    m_bRead = false;
    ReadTokenDiscard();
    if (pFloat)
      *pFloat = (float)atof(m_szToken);
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, char *pszString, int len)
{
  if (IsToken(pszTok) && ReadTokenDiscard() && IsToken("="))
  {
    m_bRead = false;
    ReadTokenDiscard();
    StrUtil::SafeStrncpy(pszString, m_szToken, len);
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, std::string *s)
{
  if (IsToken(pszTok) && ReadTokenDiscard() && IsToken("="))
  {
    m_bRead = false;
    ReadTokenDiscard();
    *s = m_szToken;
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, TVector3 *pVec)
{
  if (IsToken(pszTok))
  {
    CReadChunker c(*this);
    pVec->x = c.ReadFloat();
    pVec->y = c.ReadFloat();
    pVec->z = c.ReadFloat();
    c.End();
    ReadTokenDiscard();
    return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CReadChunker::ReadValue   (const char *pszTok, TQuaternion *pQuat)
{
  if (IsToken(pszTok))
  {
    CReadChunker c(*this);
    pQuat->x = c.ReadFloat(1);
    pQuat->y = c.ReadFloat();
    pQuat->z = c.ReadFloat();
    pQuat->w = c.ReadFloat();
    c.End();
    ReadTokenDiscard();
    return true;
  }
  return false;
}
