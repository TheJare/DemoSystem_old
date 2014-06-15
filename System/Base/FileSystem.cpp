// -------------------------------------------------------------------------------------
// File:        FileSystem.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "Base.h"
#include "FileSystem.h"
#include "LinkedList.h"
#include "ZipFile.h"
#include "StrUtil.h"
#include <stdarg.h>
#include <io.h>

// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------

static bool m_bOk = true;
static char m_szBaseDir[300] = "";
static CLinkedList<CZipFile *> m_Zips;
static CLinkedList<CFile *>   m_Files;

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError FileSystem::Init(const char *pszBaseDir)
{
  End();

  m_Zips.Init();
  m_Files.Init();
  m_bOk = true;
  SetBaseDir(pszBaseDir);
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::End()
{
  if (IsOk())
  {
    while (m_Files.GetHead())
      Close(m_Files.GetHead()->t);

    while (m_Zips.GetHead())
    {
      fclose(m_Zips.GetHead()->t->GetFile());
      m_Zips.Remove(m_Zips.GetHead());
    }
    m_bOk = false;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool FileSystem::IsOk  ()
{
  return m_bOk;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::GetBaseDir  (char *pDest, int len)
{
  StrUtil::SafeStrncpy(pDest, m_szBaseDir, len);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::SetBaseDir  (const char *pszBaseDir)
{
  StrUtil::SafeStrncpy(m_szBaseDir, pszBaseDir, sizeof(m_szBaseDir));
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CFile *FileSystem::Open (const char *pszName, const char *pszMode)
{
  if (!IsOk() || pszName == NULL || *pszName == '\0')
    return NULL;

  char buf[1000];
  if (!m_szBaseDir[0] || pszName[0] == '\\' || strchr(pszName, ':') != NULL)
    strcpy(buf, pszName);
  else
    sprintf(buf, "%s\\%s", m_szBaseDir, pszName);

  dword flags = (strchr(pszMode, 'r')? CFile::F_READ : 0)
              | (strchr(pszMode, 'w')? CFile::F_WRITE : 0)
              | (strchr(pszMode, 'a')? CFile::F_WRITE : 0)
              | (strchr(pszMode, '+')? (CFile::F_WRITE | CFile::F_WRITE) : 0)
              | (strchr(pszMode, 't')? CFile::F_TEXT : 0);

  CFile *pFile = NULL;
  FILE *f = fopen(buf, pszMode);
  if (f)
  {
    GLOG(("Opening HD file %s\n", buf));
    pFile = NEW(CFile);
    pFile->InitFile(f, flags);
  }
  else if ((flags & (CFile::F_READ | CFile::F_WRITE)) == CFile::F_READ)
  {
    // buscar en los ZIPs
    int len = strlen(buf);
    bool bFound = false;
    TLinkedListNode<CZipFile*> *pNode = m_Zips.GetHead();
    while (!bFound && pNode)
    {
      CZipFile *pZip = pNode->t;
      for (int i = 0; !bFound && i < pZip->GetNumFiles(); i++)
        if (pZip->IsFilename(i, buf, len))
        {
          int size = pZip->GetFileLen(i);
          char *pData = NEW_ARRAY(char, size);
          if (pData && RET_OK == pZip->ReadFile(i, pData))
          {
            GLOG(("Opening packed file %s\n", buf));
            pFile = NEW(CFile);
            pFile->InitMem(pData, size, (flags & CFile::F_TEXT) | CFile::F_ZIP | CFile::F_READ);
            bFound = true;
          }
          else
            DISPOSE_ARRAY(pData);
        }
      pNode = pNode->pNext;
    }
  }

  if (pFile)
    m_Files.Add(pFile);
  else
    GLOG(("ERROR opening File %s\n", buf));

  return pFile;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::Close (CFile *pFile, bool bFreeMem)
{
  if (IsOk() && pFile->IsOk())
  {
    TLinkedListNode<CFile*> *pNode = m_Files.FindNode(pFile);
    ASSERT(pNode);
    if (pNode)
    {
      m_Files.Remove(pNode);
      if (pFile->m_f)
        fclose(pFile->m_f);
      if (bFreeMem)
      {
        void *pData = (void*)pFile->m_pData; 
        DISPOSE_ARRAY(pData);
      }
      DISPOSE(pFile);
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
const void *FileSystem::ReadFile    (int *pSize, const char *pszName, const char *pszMode)
{
  CFile *pFile = Open(pszName, pszMode);
  if (!pFile)
    return NULL;

  // Try to reuse the already-expanded file
  const byte *pData = pFile->GetMemoryBuffer(pSize);
  if (pData && !pFile->IsText())
    Close(pFile, false);
  else
  {
    // Otherwise we have to manually decode the file
    int len = pFile->GetLen();
    byte *pDest = NEW_ARRAY(byte, len);
    if (pDest)
    {
      pData = pDest;
      len = pFile->Read(pDest, len);
      if (pSize)
        *pSize = len;
    }
    Close(pFile);
  }

  return pData;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::FreeFile    (const void *&pData)
{
  void *p = (void*)pData;
  DISPOSE_ARRAY(p);
  pData = NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void FileSystem::AddZipFiles (const char *pszDir, const char *pszExt, const char *pszPassword)
{
  char buf[1000];
  char dir[1000];
  if (!m_szBaseDir[0] || pszDir[0] == '\\' || strchr(pszDir, ':') != NULL)
    strcpy(dir, pszDir);
  else
    sprintf(buf, "%s\\%s", m_szBaseDir, pszDir);

  if (dir[0])
    sprintf(buf, "%s\\*.%s", dir, pszExt);
  else
    sprintf(buf, "*.%s", pszExt);
  struct _finddata_t fd;
  long hFind;
  hFind = _findfirst(buf, &fd);
  if (hFind != -1)
  {
    do
    {
      if (dir[0])
        sprintf(buf, "%s\\%s", dir, fd.name);
      else
        sprintf(buf, "%s", fd.name);

      GLOG(("Found Pakfile %s, opening...\n", buf));
      FILE *f = fopen(buf, "rb");
      if (f)
      {
        CZipFile *pZip = NEW(CZipFile);

        if (pZip && RET_OK == pZip->Init(f, pszPassword))
        {
          GLOG(("   Pakfile added\n", buf));
          m_Zips.Add(pZip);
        }
        else
        {
          GLOG(("   Not a real pakfile!\n", buf));
          fclose(f);
          DISPOSE(pZip);
        }
      }
    }
    while (_findnext(hFind, &fd) == 0);
    _findclose(hFind);
  }

  return;
}
