//----------------------------------------------------------------------------
//  Nombre:    DirectInputManager.h
//
//  Contenido: Handle DirectInput entry
//----------------------------------------------------------------------------


#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

class CWinWindow;

typedef struct IDirectInput8        IDInput;
typedef struct IDirectInputDevice8  IDInputDevice;

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
class CInputManager
{

public:

  class CDevice
  {
    public:
      enum EType
      {
        DIMD_KEYBOARD,
        DIMD_MOUSE,
        DIMD_JOYSTICK
      };

      CDevice (): m_pMgr(NULL)  { }
      ~CDevice()                { End(); }

              TError    Init        (CInputManager *pMgr, EType type);
      virtual void      End         ();
              bool      IsOk        ()                  const { return (m_pMgr != NULL); }

      virtual TError    Poll        ()                        { m_prevButtons = m_buttons; return RET_OK; }

      virtual TError    Capture     (bool bCapture);
              bool      IsCaptured  ()                  const { return m_bCaptured; }

              int       GetAxis     (unsigned n)        const { return (n < MAX_AXIS)? m_aAxis[n] : 0; }
              void      SetAxis     (unsigned n, int v)       { if (n < MAX_AXIS) m_aAxis[n] = v; }
              unsigned  GetButtons  ()                  const { return m_buttons; }
              unsigned  GetPushedButtons  ()            const { return m_buttons & ~m_prevButtons; }
              unsigned  GetReleasedButtons()            const { return ~m_buttons & m_prevButtons; }

    protected:
      enum
      {
        MAX_AXIS = 6,
      };
      CInputManager *m_pMgr;
      EType               m_type;
      bool                m_bCaptured;
      int                 m_aAxis[MAX_AXIS];
      unsigned            m_buttons;
      unsigned            m_prevButtons;
  };

  // ------------------------------------
  class CMouse: public CDevice
  { 
    public:
      CMouse(): m_pDev(0) { }

              TError    Init        (CInputManager *pMgr);
      virtual void      End         ();
      virtual TError    Capture     (bool bCapture);
      virtual TError    Poll        ();
    protected:
      IDInputDevice       *m_pDev;
  };

  // ------------------------------------
  class CJoystick: public CDevice
  { 
    public:
              TError    Init        (CInputManager *pMgr, unsigned ord);
      virtual TError    Poll        ();
  protected:
      unsigned m_ord;
      unsigned m_minAxis;
      unsigned m_maxAxis;

  };

  // ------------------------------------

  CInputManager (): m_pDI(NULL) { }
  ~CInputManager()              { End(); }

  TError    Init              (const CWinWindow &Win);
  void      End               ();
  bool      IsOk              ()                            const { return (m_pDI != NULL); }

  IDInput  *GetDInput         ()                            const { return m_pDI; }
  const CWinWindow *
            GetWindow         ()                            const { return m_pWin; }
  
  TError    PollMouse         ()                                  { return m_Mouse.Poll(); }
  TError    CaptureMouse      (bool bCapture)                     { return m_Mouse.Capture(bCapture); }
  bool      IsMouseCaptured   ()                            const { return m_Mouse.IsCaptured(); }
  int       GetMouseX         ()                            const { return m_Mouse.GetAxis(0); }
  int       GetMouseY         ()                            const { return m_Mouse.GetAxis(1); }
  int       GetMouseZ         ()                            const { return m_Mouse.GetAxis(2); }
  void      SetMouseX         (int v)                             { m_Mouse.SetAxis(0, v); }
  void      SetMouseY         (int v)                             { m_Mouse.SetAxis(1, v); }
  void      SetMouseZ         (int v)                             { m_Mouse.SetAxis(2, v); }
  unsigned  GetMouseButtons   ()                            const { return m_Mouse.GetButtons(); }

  const CMouse   &
            GetMouse          ()                            const { return m_Mouse; }
  CMouse   &GetMouse          ()                                  { return m_Mouse; }

  unsigned  GetNumJoysticks   ()                            const { return m_nJoysticks; }
  const CJoystick &
             GetJoystick      (unsigned n = 0)              const { ASSERT(n < MAX_JOYSTICKS); return m_aJoysticks[n]; }
  CJoystick &GetJoystick      (unsigned n = 0)                    { ASSERT(n < MAX_JOYSTICKS); return m_aJoysticks[n]; }

private:
  enum
  {
    MAX_JOYSTICKS = 8,
  };
  IDInput        *m_pDI;
  CMouse         m_Mouse;
  CJoystick      m_aJoysticks[MAX_JOYSTICKS];
  unsigned       m_nJoysticks;

  const CWinWindow *m_pWin;
};

#endif // _INPUTMANAGER_H_
