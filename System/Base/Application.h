// ----------------------------------------------------------------------------------------
// File:      Application.h
//
// Purpose:   Generic application class
// ----------------------------------------------------------------------------------------

#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#pragma once

class CDisplayDevice;

// --------------------

class CApplication
{
//    typedef Inherited ;
  public:
    CApplication (): m_pDev(NULL)    { }
    ~CApplication()                  { End(); }

            TError      Init          (CDisplayDevice *pDev);
    virtual void        End           ()            { m_pDev = NULL; }
            bool        IsOk          ()      const { return (m_pDev != 0); }

    virtual unsigned    GetTickLength ()      const { return 0; }     // 0 means variable timstep mode, otherwise return desired fixed timestep length.
            CDisplayDevice *
                       GetDisplayDevice  ()   const { return m_pDev; }

    virtual void        Run           (unsigned ms) { }
    virtual void        Draw          ()            { }

// ------------
  private:
    CDisplayDevice         *m_pDev;
};

#endif //_APPLICATION_H_
