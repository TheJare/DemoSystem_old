//----------------------------------------------------------------------------
//  Nombre:    DirectInputManager.cpp
//
//  Contenido: Handle DirectInput entry
//----------------------------------------------------------------------------

#include "Base.h"
#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#include "InputManager.h"
#include <math.h>

#include "WinWindow.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError CInputManager::CDevice::Init(CInputManager *pMgr, EType type)
{
  End();
  m_pMgr = pMgr;
  m_type = type;
  m_bCaptured = false;
  memset(m_aAxis, 0, sizeof(m_aAxis));
  m_buttons = 0;
  m_prevButtons = 0;
  return RET_OK;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
void CInputManager::CDevice::End()
{
  m_pMgr = NULL;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError CInputManager::CMouse::Init(CInputManager *pMgr)
{
  CDevice::Init(pMgr, DIMD_MOUSE);
  m_pDev = NULL;

  HRESULT hr = m_pMgr->GetDInput()->CreateDevice(GUID_SysMouse, &m_pDev, NULL);
  if (!FAILED(hr))
  {
    m_pDev->SetDataFormat(&c_dfDIMouse2);
    hr = m_pDev->SetCooperativeLevel(reinterpret_cast<HWND>(m_pMgr->GetWindow()->GetHwnd()), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    if (FAILED(hr))
      SAFE_RELEASE(m_pDev);
    else
    {
      DIPROPDWORD dipdw;
      dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj        = 0;
      dipdw.diph.dwHow        = DIPH_DEVICE;
      dipdw.dwData            = 16;
      m_pDev->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
    }
  }
  return IsOk()? RET_OK : RET_FAIL;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
void CInputManager::CMouse::End()
{
  if (IsOk())
    SAFE_RELEASE(m_pDev);
  CDevice::End();
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError CInputManager::CJoystick::Init(CInputManager *pMgr, unsigned ord)
{
  if (RET_OK != CDevice::Init(pMgr, DIMD_JOYSTICK))
    return RET_FAIL;

  UINT njoys = ::joyGetNumDevs();
  if (ord >= njoys)
    End();
  else
  {
    JOYCAPS jc;
    MMRESULT r = ::joyGetDevCaps(ord, &jc, sizeof(jc));
    if (r != JOYERR_NOERROR)
      End();
    else
    {
      m_ord = ord;
      m_minAxis = jc.wXmin; // Let's assume all axes have the same range... for now at least.
      m_maxAxis = jc.wXmax;
      Poll();
    }
  }
  return IsOk()? RET_OK : RET_FAIL;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError    CInputManager::CDevice::Capture(bool bCapture)
{
  m_bCaptured = bCapture;
  return RET_OK;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError    CInputManager::CMouse::Capture(bool bCapture)
{
  if (!IsOk() || !m_pDev)
    return RET_FAIL;

  HRESULT hr = bCapture? m_pDev->Acquire() : m_pDev->Unacquire();
  if (hr == DI_OK || hr == DI_NOEFFECT || hr == S_FALSE)
  {
    m_bCaptured = bCapture;
    if (m_bCaptured)
    {
      DIMOUSESTATE2 st;
      HRESULT hr = m_pDev->GetDeviceState(sizeof(st), &st);
      if (FAILED(hr) && hr != E_PENDING)
        m_bCaptured = false;
      else
      {
        m_aAxis[0] = st.lX;
        m_aAxis[1] = st.lY;
        m_aAxis[2] = st.lZ;
        m_prevButtons = 0;
        m_buttons = 0;
        for (int i = 0; i < 8; ++i)
          if (st.rgbButtons[i] & 0x80)
            m_buttons |= (1 << i);
      }
    }
  }

  return (m_bCaptured == bCapture)? RET_OK : RET_FAIL;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError CInputManager::CMouse::Poll()
{
  if (!IsOk() || !m_pDev || !m_bCaptured)
    return RET_FAIL;

  CDevice::Poll();
  DWORD dwElements;   // number of items to be retrieved
  do
  {
    dwElements = 1;   // number of items to be retrieved
    DIDEVICEOBJECTDATA od;
    HRESULT hr = m_pDev->GetDeviceData(sizeof(od), &od, &dwElements, 0);
    if (FAILED(hr))
    {
      m_bCaptured = false; // Error happened, we lost the mouse
      return RET_FAIL;
    }

    if (dwElements > 0)
    {
      switch (od.dwOfs)
      {
        case DIMOFS_X: m_aAxis[0] += static_cast<int>(od.dwData); break;
        case DIMOFS_Y: m_aAxis[1] += static_cast<int>(od.dwData); break;
        case DIMOFS_Z: m_aAxis[2] += static_cast<int>(od.dwData); break;

#define MACRO_CHECKBUTTON(n) case DIMOFS_BUTTON##n: if (od.dwData & 0x80) m_buttons |= 1 << n; else m_buttons &= ~(1 << n); break
        MACRO_CHECKBUTTON(0);
        MACRO_CHECKBUTTON(1);
        MACRO_CHECKBUTTON(2);
        MACRO_CHECKBUTTON(3);
        MACRO_CHECKBUTTON(4);
        MACRO_CHECKBUTTON(5);
        MACRO_CHECKBUTTON(6);
        MACRO_CHECKBUTTON(7);
#undef MACRO_CHECKBUTTON
      }
    }
  }
  while (dwElements > 0);

  return RET_OK;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError    CInputManager::CJoystick::Poll ()
{
  if (!IsOk() || !m_bCaptured)
    return RET_FAIL;

  CDevice::Poll();

  JOYINFOEX ji;
  ji.dwSize = sizeof(ji);
  ji.dwFlags = JOY_RETURNALL;
  if (JOYERR_NOERROR != joyGetPosEx(m_ord, &ji))
  {
    m_bCaptured = false;
    return RET_FAIL;
  }

  // Leer los valores
  m_buttons = ji.dwButtons;
  m_aAxis[0] = (ji.dwXpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;
  m_aAxis[1] = (ji.dwYpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;
  m_aAxis[2] = (ji.dwZpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;
  m_aAxis[3] = (ji.dwRpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;
  m_aAxis[4] = (ji.dwUpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;
  m_aAxis[5] = (ji.dwVpos-m_minAxis)*1000/(m_maxAxis-m_minAxis) - 500;

  return RET_OK;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
TError    CInputManager::Init              (const CWinWindow &Win)
{
  End();

  HRESULT hr = DirectInput8Create((HINSTANCE)::GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL);
  if (FAILED(hr))
    return RET_FAIL;

  m_pWin = &Win;

  m_Mouse.Init(this);
  for (int i = 0; i < MAX_JOYSTICKS; i++)
    m_aJoysticks[i].Init(this, i);
  return RET_OK;
}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
void      CInputManager::End               ()
{
  if (IsOk())
  {
    m_Mouse.End();
    for (int i = 0; i < MAX_JOYSTICKS; i++)
      m_aJoysticks[i].End();
    SAFE_RELEASE(m_pDI);
  }
}

