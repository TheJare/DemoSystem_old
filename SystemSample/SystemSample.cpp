#include "Base.h"
#include <windows.h>
#include "WinWindow.h"
#include "FileSystem.h"
#include "DisplayDevice.h"
#include "RenderContext.h"
#include "vectors.h"
#include "InputManager.h"

#include "Simple1_vs.h"
#include "Simple1_ps.h"
#include <d3d9.h>
#include "DisplayVertex.h"
#include <d3dx9.h>

  // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif

#define MOTIONBLUR
#ifdef MOTIONBLUR
#define DM_FLAGS (CDisplayDevice::F_WANT_ZBUFFER /*| CDisplayDevice::F_WANT_WBUFFER*/ | CDisplayDevice::F_WANT_NODISCARD)
#else
//#define DM_FLAGS (CDisplayDevice::F_WANT_ZBUFFER | CDisplayDevice::F_WANT_WBUFFER | CDisplayDevice::F_WANT_FSAA)
#define DM_FLAGS (CDisplayDevice::F_WANT_ZBUFFER /*| CDisplayDevice::F_WANT_WBUFFER*/)
#endif

#define DISPLAYRESX 1440
#define DISPLAYRESY  900

// --------------------------------------------------------------------------
// Simplistic window message handler.
// --------------------------------------------------------------------------
class CMyHandler : public CWinWindow::IMsgHandler
{
    bool m_bPaused;
    bool m_bWantSwitch;
  public:
                  CMyHandler  (): m_bPaused(false), m_bWantSwitch(false)          { }
            bool  IsPaused    ()                      const { return m_bPaused; }
            bool  WantSwitch  ()                            { bool bWant = m_bWantSwitch; m_bWantSwitch = false; return bWant; }
            void  Pause       (bool bPaused)                { m_bPaused = bPaused; }

    virtual bool  MsgProc(CWinWindow *pWnd, unsigned msg, unsigned wParam, unsigned lParam, unsigned &result);
};
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
bool CMyHandler::MsgProc(CWinWindow *pWnd, unsigned msg, unsigned wParam, unsigned lParam, unsigned &result)
{
  switch (msg)
  {
    case WM_CHAR:
      if ((char)wParam == '\033')
        ::PostQuitMessage(0);
      else if ((char)wParam == 'p' || (char)wParam == 'P')
        m_bPaused = !m_bPaused;
      break;
    case WM_SYSKEYDOWN:
      if (wParam == VK_RETURN && (lParam & (1 << 29)))  // Alt+Enter
      {
        m_bWantSwitch = true;
        result = 0;
        return true;
      }
      break;
    case WM_SETCURSOR:
//      ::SetCursor(NULL);
      result = true;
      return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void DoStuff(CWinWindow &Win, CMyHandler &Handler)
{
  TError ret = RET_OK;

  CInputManager Dim;
  Dim.Init(Win);

  // Inicializamos el device
  CDisplayDevice Dev;
  Dev.Init(&Win);

// --------------------------

  // Ponemos el modo de video y tal.
#ifdef _DEBUG
  ret = Dev.SetDisplayMode(DISPLAYRESX, DISPLAYRESY, 32, false, DM_FLAGS);
#else
  if (RET_OK != Dev.SetDisplayMode(DISPLAYRESX, DISPLAYRESY, 32, true, DM_FLAGS))
    ret = Dev.SetDisplayMode(DISPLAYRESX, DISPLAYRESY, 16, true, DM_FLAGS);
#endif

  if (RET_OK != ret)
  {
    BaseAlertf("Error, can't initialize the display!\n");
    return;
  }

//  ::ShowCursor(false);

// --------------------------
// --------------------------
  DWORD initTime = ::GetTickCount();
  DWORD prevTime = initTime;

  Dim.CaptureMouse(true);

  D3DVERTEXELEMENT9 declaration[] =
  {
      { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
      { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      D3DDECL_END()
  };
  LPDIRECT3DVERTEXDECLARATION9  pVertexDeclaration = NULL;
  LPDIRECT3DVERTEXSHADER9       pVertexShader      = NULL;
  LPDIRECT3DPIXELSHADER9        pPixelShader       = NULL;

  HRESULT hr = Dev.GetDirect3DDevice()->CreateVertexDeclaration( declaration, &pVertexDeclaration );
  hr = Dev.GetDirect3DDevice()->CreateVertexShader( g_vs20_Simple1, &pVertexShader );
  hr = Dev.GetDirect3DDevice()->CreatePixelShader( g_ps20_Simple1, &pPixelShader );

  while (Win.MessagePump(false)
         && Win.IsOk()
        )
  {

    // Movidas de temporizacion habituales.
    dword act;
    DWORD time;
    // Pa que no se chupe todo el tiempo lo limitamos a 1000fps :-P
    do
    {
      time = ::GetTickCount();
      act = time - prevTime;
      if (act < 1)
        Sleep(1);
    } while (act < 1);
    prevTime = time;

    // El run logico
    static float angle = 0;
    if (act > 0 && !Handler.IsPaused())
    {
//      Demo.Run(act);
      angle += .001f*act;
    }

    // Trasteamos un poco con el raton
    static float cxangle = 0;
    static float cyangle = 1.f;
    static float czangle = 10.f;
    dword buttons = 0;

    Dim.PollMouse();
    if (!Dim.IsMouseCaptured())
      Dim.CaptureMouse(true);
    else
    {
      cxangle += .005f*Dim.GetMouseX();
      cyangle += .005f*Dim.GetMouseY();
      czangle += .005f*Dim.GetMouseZ();
      Dim.SetMouseX(0);
      Dim.SetMouseY(0);
      Dim.SetMouseZ(0);
      buttons |= Dim.GetMouseButtons();
    }

    if (!Dim.GetJoystick(0).IsCaptured())
      Dim.GetJoystick(0).Capture(true);
    Dim.GetJoystick(0).Poll();
    cxangle += .00005f*Dim.GetJoystick(0).GetAxis(0);
    cyangle += .00005f*Dim.GetJoystick(0).GetAxis(1);
    czangle += .0005f*Dim.GetJoystick(0).GetAxis(3);
    buttons |= Dim.GetJoystick(0).GetButtons();

    // Guarreamos con las movidas de la ventana y tal.
    // Esto es unpoco cerdo porque tampoco tocamos el estilo de la ventana, pero bueno.
    bool bWantSwitch = Handler.WantSwitch();
    unsigned wx = Win.GetWidth();
    unsigned wy = Win.GetHeight();
    if (wx > 0 && wy > 0 && (wx != Dev.GetWidth() || wy != Dev.GetHeight() || bWantSwitch))
    {
      bool bFullScreen = bWantSwitch? !Dev.IsFullScreen() : Dev.IsFullScreen();
      if (RET_OK != Dev.SetDisplayMode(wx, wy, Dev.GetBpp(), bFullScreen, Dev.GetFlags()))
        if (RET_OK != Dev.SetDisplayMode(DISPLAYRESX, DISPLAYRESY, 32, false, Dev.GetFlags()))
          Dev.SetDisplayMode(DISPLAYRESX, DISPLAYRESY, 16, false, Dev.GetFlags());
    }

    // Si podemos dibujar, lo hacemos.
    if (RET_OK == Dev.BeginFrame())
    {
      // Limpiamos en device
//      Dev.Clear(0xFF000000 | (rand() << 16) | rand());  // fondos aleatorios rules!!
      Dev.Clear();

      // Preparamos la proyeccion
      TProjectionViewport vp(0, 0, Dev.GetWidth(), Dev.GetHeight());

      // Podriamos hacer esto directamente, pero si usamos un RC,
      // podemos gamberrear con alguna operacion 2D sencillita.
      //Dev.SetProjectionViewport(&vp);
      CRenderContext rc;
      rc.Init(&Dev, vp);

      // Aqui dibujamos lo que nos salga de los eggs.
//      Demo.Draw();

      // Movemos la camara.
      TMatrix3 mCam;
      float r = czangle*cosf(cyangle);
      mCam.SetLookAt(TVector3(r*cosf(cxangle), czangle*sinf(cyangle), r*sinf(cxangle)), TVector3(0, 2.5f, 0), TVector3(0, 1, 0));
      Dev.SetViewTransform(mCam);

        // Tambien podemos usar la SetCameraTransform para darle una transformacion en mundo y que la invierta el solito.
//      mCam.SetRotationZ(angle);
//      mCam.SetTranslation(5.f*sinf(angle), 0, -10);
//      Dev.SetCameraTransform(mCam);

      // Dibujamos una rejilla en el plano XZ
      // Un triangulo en el cuadrante X>0 Z>0
      // Y un eje vertical
      TMatrix3 mObj(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0);

#define LIMIT 30
      Dev.SetWorldTransform(mObj);
      rc.DrawTri(TVector3(0, -.01f, 0), TVector3( 0, -.01f, LIMIT), TVector3(LIMIT, -.01f, 0), 0xFF0000FF);
      for (float x = -LIMIT; x <= .1f+LIMIT; x++)
        rc.DrawLine3D(TVector3(x, 0, -LIMIT), TVector3(x, 0, LIMIT), 0xFFFF0000);
      for (float z = -LIMIT; z <= .1f+LIMIT; z++)
        rc.DrawLine3D(TVector3(-LIMIT, 0, z), TVector3(LIMIT, 0, z), 0xFF00FF00);
      rc.DrawLine3D(TVector3(0, 0, 0), TVector3(0, LIMIT, 0), 0xFF0000FF);

      // Y luego dibujamos un "objeto"
      mObj.SetRotationY(angle);
      mObj.SetTranslation(TVector3(0, 0, 0));
      Dev.SetWorldTransform(mObj);
      rc.DrawTri(TVector3(-5, 0, 0), TVector3( 5, 0, 0), TVector3( 0, 5, 0), 0x80FFFFFF);
      rc.DrawTri(TVector3( 5, 0, 0), TVector3(-5, 0, 0), TVector3( 0, 5, 0), 0xFF008080);

      // Alguna parida 2D
      rc.FillRect(2, 2, 400, 20, 0x80FFFF00U);
      for (int i = 0; i < 32; i++)
        if (buttons & (1 << i))
           rc.FillRect(2.f+30*i, 2, 25, 20, 0xFF0000FFU);

//      hr = Dev.GetDirect3DDevice()->SetVertexShaderConstantF( 0, &Dev.GetTransWorldViewProj().m[0][0], 4 );
      hr = Dev.GetDirect3DDevice()->SetVertexShaderConstantF( 0, &Dev.GetTransScreenTransform().m[0][0], 4 );
      hr = Dev.GetDirect3DDevice()->SetVertexDeclaration( pVertexDeclaration );
      hr = Dev.GetDirect3DDevice()->SetVertexShader( pVertexShader );
      hr = Dev.GetDirect3DDevice()->SetPixelShader( pPixelShader );

      {
        float x = 200, y = 200, w = 300, h = 300, z = .5f;
        TColor c = 0xFFFF00FF;

        TLVertex av[4];
        av[0].Set(x,   y,   z, c);
        av[1].Set(x+w, y,   z, c);
        av[2].Set(x,   y+h, z, c);
        av[3].Set(x+w, y+h, z, c);
        Dev.GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, av, sizeof(*av));
      }
      {
        float x = -50, y = -50, w = 300, h = 300, z = 1.f;
        TColor c = 0xFF00FF00;

        TLVertex av[4];
        av[0].Set(x,   y,   z, c);
        av[1].Set(x+w, y,   z, c);
        av[2].Set(x,   y+h, z, c);
        av[3].Set(x+w, y+h, z, c);
        Dev.GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, av, sizeof(*av));
      }
      {
        TLVertexUV av[2];
        av[0].Set(0, 0, 0, 0xFFFF00FF, 0, 0);
        av[1].Set(LIMIT, LIMIT, 0, 0xFFFF00FF, 0, 0);
        hr = Dev.GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, av, sizeof(*av));
        hr = Dev.GetDirect3DDevice()->SetVertexShader( NULL );
        hr = Dev.GetDirect3DDevice()->SetPixelShader( NULL );
      }

      // Hemos acabao.
      Dev.EndFrame();
      Dev.Update();
    }
  }
  SAFE_RELEASE(pVertexDeclaration);
  SAFE_RELEASE(pVertexShader);
  SAFE_RELEASE(pPixelShader);
}

#pragma comment(lib, "winmm.lib")

// --------------------------------------------------------------------------
// WinMain
// --------------------------------------------------------------------------
int WINAPI WinMain(
  HINSTANCE hInstance,     // handle to current instance
  HINSTANCE hPrevInstance, // handle to previous instance
  LPSTR     lpCmdLine,     // command line
  int       nCmdShow       // show state
)
{

  // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  CWinWindow Win;
  CMyHandler Handler;

  FileSystem::Init("");
  FileSystem::AddZipFiles("", "zip");
  FileSystem::AddZipFiles("", "dat");
  FileSystem::AddZipFiles("", "pak");
//  FileSystem::AddZipFiles("", "exe");
  FileSystem::SetBaseDir("data");

  // Debug mode is windowed, release/final mode is fullscreen.
#ifdef _DEBUG
  Win.Init("SystemSample", &Handler);
#else
// Amazing, this doesn't work - when the app is set to fullscreen, the window doesn't cover the screen WTF!?!?!?!
//  Win.Init("Durantro", WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, &Handler);
// Amazing, this doesn't work either - when the app is set to fullscreen, the window doesn't go WS_POPUP WTF!?!?!?!
//  Win.Init("Durantro", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE, &Handler);
//  Win.Init("Durantro", WS_OVERLAPPEDWINDOW | WS_VISIBLE, &Handler);

  Win.Init("SystemSample", WS_POPUP | WS_VISIBLE, &Handler);
#endif

  DoStuff(Win, Handler);

  Win.End();

  FileSystem::End();

  return 0;
}
