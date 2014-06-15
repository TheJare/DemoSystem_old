//----------------------------------------------------------------------------
//  Nombre:    MusicPlayer.h
//
//  Contenido: Tocador de modulos
//----------------------------------------------------------------------------

#ifndef _MUSIC_PLAYER_H_
#define _MUSIC_PLAYER_H_

//#include "MidasDLL.h"

class CMusicPlayer
{
  public:
    CMusicPlayer            (): m_flags(0)    { }
    ~CMusicPlayer           ()                { End(); }

    TError      Init        (unsigned hwnd);
    void        End         ();
    bool        IsOk        () const      { return (m_flags & FLAG_OK) != 0; }

    bool        IsLoaded    () const      { return (m_module || m_stream); }
    bool        IsPlaying   () const      { return (m_flags & FLAG_PLAYING) != 0; }

    TError      LoadModule  (const char *pszName);
    void        FreeSong    ();

    TError      Play        (bool bLoop = true);
    void        Stop        ();
    void        Pause       ();
    void        Resume      ();

    void        Run         (uint timediff);

    void        GetPosition (int *pPos, int *pRow) const;

    void        SetPosition (int pos);

    void        SetMasterVolume (float volume);
    void        SetVolume       (float volume);
    void        SetFade         (uint time, float destVol);

    float       GetVolume       () const        { return m_fVolume; }
    float       GetMasterVolume () const        { return m_fMasterVolume; }
    bool        IsFading        () const        { return m_fFadeSpeed != 0.f; }


  private:
    enum
    {
      FLAG_OK             = 0x0001,
      FLAG_PLAYING        = 0x0002,
    };

    uint                  m_flags;

//    struct TInternalData;
//    static struct TInternalData s_Data;

//    //typedef void *THandle;
    typedef dword THandle;

    THandle               m_module;
    THandle               m_stream;
    THandle               m_playHandle;

    const void           *m_pSongData;

    float                 m_fMasterVolume;
    float                 m_fVolume;
    float                 m_fFadeSpeed;
    float                 m_fDestVolume;
};


#endif // _MUSIC_PLAYER_H_
