//----------------------------------------------------------------------------
//  Nombre:    MusicPlayer.cpp
// 
//  Contenido: Tocador de modulos
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "MusicPlayer.h"
#include "FileSystem.h"

#define BASSDEF(f) (WINAPI *f) // define the functions as pointers
#include "bass.h"
//#pragma comment(lib, "bass.lib")
 
/*
struct CMusicPlayer::TInternalData
{
  HMUSIC hmusic;
};
*/

//----------------------------------------------------------------------
//----------------------------------------------------------------------

//CMusicPlayer::TInternalData CMusicPlayer::s_Data;


//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
TError      CMusicPlayer::Init        (unsigned hwnd)
{
  End();

  HINSTANCE bass=LoadLibrary("BASS.DLL"); // load BASS
  if (!bass)
    return RET_FAIL;

  // Each function pointer var has its own type, but GetProcAddress returns
  // a generic funcPtr type, so we use a nonportable lvalue cast trick.
#define INITBASSF(f) *(void**)&f=(void*)GetProcAddress(bass, #f)
//#define INITBASSF(f) f=GetProcAddress(bass, #f)

  INITBASSF(BASS_Init);
  INITBASSF(BASS_Start);
  INITBASSF(BASS_Stop);
  INITBASSF(BASS_Free);
  INITBASSF(BASS_GetInfo);
  INITBASSF(BASS_Pause);
  INITBASSF(BASS_SetGlobalVolumes);

  INITBASSF(BASS_StreamCreateFile);
  INITBASSF(BASS_StreamPlay);
  INITBASSF(BASS_StreamFree);

  INITBASSF(BASS_MusicLoad);
  INITBASSF(BASS_MusicFree);
  INITBASSF(BASS_MusicPlay);
  INITBASSF(BASS_MusicSetPositionScaler);

  INITBASSF(BASS_ChannelStop);
  INITBASSF(BASS_ChannelGetFlags);
  INITBASSF(BASS_ChannelGetPosition);
  INITBASSF(BASS_ChannelSetPosition);
  INITBASSF(BASS_ChannelGetAttributes);

  BASS_Init(-1, 44100, 0, (HWND) hwnd);

  BASS_INFO info;
  info.size=sizeof(BASS_INFO);
  BASS_GetInfo(&info);
  if (info.flags&DSCAPS_EMULDRIVER)
    // device does NOT have hardware support 
    GLOG(("device does NOT have hardware support\n"));
  else
    GLOG(("device DOES have hardware support\n"));

  BASS_Start(); // Start digital output

  m_flags = FLAG_OK;
  m_module = 0;
  m_stream = 0;
  m_fMasterVolume = 1.f;

//  BASS_SetGlobalVolumes(0, -1, -1);

  return RET_OK;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::End         ()
{
  if (!IsOk())
    return;

  FreeSong();

	BASS_Stop();	// Stop digital output

  // It's not actually necessary to free the musics and samples
  // because they are automatically freed by BASS_Free()
	BASS_Free();	/* Close digital sound system */

}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
TError      CMusicPlayer::LoadModule  (const char *pszName)
{
  if (!IsOk())
    return RET_FAIL;

  FreeSong();
  m_fVolume = 1.f;
  m_fFadeSpeed = 0.f;

  int musicLen = 0;

  m_pSongData = FileSystem::ReadFile(&musicLen, pszName);
  if (m_pSongData)
  {
    m_module = BASS_MusicLoad(TRUE, (void*)m_pSongData, 0, musicLen, BASS_MUSIC_RAMP);
    if (m_module)
      BASS_MusicSetPositionScaler(m_module, 1);
    else
      m_stream = BASS_StreamCreateFile(TRUE, (void*)m_pSongData, 0, musicLen, BASS_MP3_SETPOS);

    // Since MOD playback makes its own copy of the music, we only keep the music
    // file when we have an MP3.
    if (!m_stream)
      FileSystem::FreeFile(m_pSongData);
  }

  return IsLoaded()? RET_OK : RET_FAIL;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::FreeSong ()
{
  if (!IsOk() || !IsLoaded())
    return;

  Stop();
  if (m_module)
    BASS_MusicFree(m_module);
  else
    BASS_StreamFree(m_stream);
  FileSystem::FreeFile(m_pSongData);
  m_module = 0;
  m_stream = 0;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
TError      CMusicPlayer::Play        (bool bLoop)
{
  if (!IsOk() || !IsLoaded())
    return RET_FAIL;

  Stop();
  if (m_module)
    BASS_MusicPlay(m_module);
  else
    BASS_StreamPlay(m_stream, FALSE, 0);

  m_flags |= FLAG_PLAYING;
  return RET_OK;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::Stop        ()
{
  if (!IsOk())
    return;

  if (m_module)
    BASS_ChannelStop(m_module);
  if (m_stream)
    BASS_ChannelStop(m_stream);
  m_flags &= ~FLAG_PLAYING;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::Pause       ()
{
  if (!IsOk() || !IsPlaying())
    return;

  if (BASS_Pause())
    m_flags &= ~FLAG_PLAYING;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::Resume      ()
{
  if (!IsOk() || !IsLoaded() || IsPlaying())
    return;

  if (BASS_Start())
    m_flags |= FLAG_PLAYING;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::Run         (uint timediff)
{
//  float msThisFrame = 1000.f/(float)timediff;
//  if (!IsOk() || !IsLoaded() || !IsPlaying())
//    return;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::GetPosition (int *pPos, int *pRow) const 
{
  if (pPos != NULL)
    *pPos = 0;
  if (pRow != NULL)
    *pRow = 0;

  if (!IsOk())
    return;

  DWORD pos = BASS_ChannelGetPosition(m_module? m_module : m_stream);
  if (m_module)
  {
    if (pPos != NULL)
      *pPos = pos & 0xFFFF;
    if (pRow != NULL)
      *pRow = (pos >> 16) & 0xFFFF;
  }
  else
  {
    DWORD freq;
    BASS_ChannelGetAttributes(m_stream, &freq, 0, 0); // sample rate
    DWORD flags = BASS_ChannelGetFlags(m_stream); // stereo/mono, 8/16 bit flags

    GLOG(("RAW POSITION: %d  ", pos));
    pos = pos*10 / (freq*(flags&BASS_SAMPLE_MONO?1:2)*(flags&BASS_SAMPLE_8BITS?1:2)); // the time length

    if (pPos != NULL)
      *pPos = pos / 10;
    if (pRow != NULL)
      *pRow = pos % 10;
  }
  GLOG(("POSITION: %d:%d\n", pPos? *pPos : 0, pRow? *pRow : 0));
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::SetPosition (int pos)
{
  if (!IsOk() || m_playHandle == 0)
    return;

  if (m_module)
    BASS_ChannelSetPosition(m_module, pos);
  else
  {
    DWORD freq;
    BASS_ChannelGetAttributes(m_stream, &freq, 0, 0); // sample rate
    DWORD flags = BASS_ChannelGetFlags(m_stream); // stereo/mono, 8/16 bit flags

    pos = pos * ( freq * (flags&BASS_SAMPLE_MONO?1:2)*(flags&BASS_SAMPLE_8BITS?1:2)); // the time length

    GLOG(("SETTING RAW POSITION: %d  ", pos));
    BASS_ChannelSetPosition(m_stream, pos);
  }
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void        CMusicPlayer::SetVolume   (float volume)
{
  BASS_SetGlobalVolumes(int(volume*100), -1, -1);
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:
//----------------------------------------------------------------------
void        CMusicPlayer::SetFade     (uint time, float destVol)
{
}
