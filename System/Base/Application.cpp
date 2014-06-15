// ----------------------------------------------------------------------------------------
// File:      Application.cpp
//
// Purpose:   
// ----------------------------------------------------------------------------------------

#include "Base.h"
#include "Application.h"
//#include "DisplayDevice.h"

// --------------------

// ------------------------------------------------------------------------
// Name:       Init
// Purpose:    Initialize the object
// Parameters: 
// Returns:    RET_OK if successful, RET_FAIL otherwise
// ------------------------------------------------------------------------
TError CApplication::Init (CDisplayDevice *pDev)
{
  End();
  m_pDev = pDev;
  return RET_OK;
}
