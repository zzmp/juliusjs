/**
 * @file   m_adin.c
 * 
 * <JA>
 * @brief  音声入力デバイスの初期化
 * </JA>
 * 
 * <EN>
 * @brief  Initialize audio input device
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Fri Mar 18 16:17:23 2005
 *
 * $Revision: 1.17 $
 * 
 */
/*
 * Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/julius.h>


/** 
 * Set up device-specific parameters and functions to AD-in work area.
 *
 * @param a [i/o] AD-in work area
 * @param source [in] input source ID @sa adin.h
 * 
 * @return TRUE on success, FALSE if @a source is not available.
 */
static boolean
adin_select(ADIn *a, int source, int dev)
{
  switch(source) {
  case SP_RAWFILE:
#ifdef HAVE_LIBSNDFILE
    /* libsndfile interface */
    a->ad_standby 	   = adin_sndfile_standby;
    a->ad_begin 	   = adin_sndfile_begin;
    a->ad_end 		   = adin_sndfile_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_terminate 	   = NULL;
    a->ad_read 		   = adin_sndfile_read;
    a->ad_input_name	   = adin_sndfile_get_current_filename;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
#else  /* ~HAVE_LIBSNDFILE */
    /* built-in RAW/WAV reader */
    a->ad_standby 	   = adin_file_standby;
    a->ad_begin 	   = adin_file_begin;
    a->ad_end 		   = adin_file_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_terminate 	   = NULL;
    a->ad_read 		   = adin_file_read;
    a->ad_input_name	   = adin_file_get_current_filename;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
#endif
    break;
#ifdef USE_MIC
  case SP_MIC:
    /* microphone input */
    a->silence_cut_default = TRUE;
    a->enable_thread 	   = TRUE;
    switch(dev) {
    case SP_INPUT_DEFAULT:
      a->ad_standby 	     = adin_mic_standby;
      a->ad_begin 	     = adin_mic_begin;
      a->ad_end 	     = adin_mic_end;
      a->ad_read 	     = adin_mic_read;
      a->ad_input_name	     = adin_mic_input_name;
      a->ad_pause	     = adin_mic_pause;
      a->ad_terminate	     = adin_mic_terminate;
      a->ad_resume	     = adin_mic_resume;
    /* web audio integration */
#ifdef USE_WEBAUDIO
      a->enable_thread = FALSE;
#endif
      break;
#ifdef HAS_ALSA
    case SP_INPUT_ALSA:
      a->ad_standby 	     = adin_alsa_standby;
      a->ad_begin 	     = adin_alsa_begin;
      a->ad_end 	     = adin_alsa_end;
      a->ad_read 	     = adin_alsa_read;
      a->ad_input_name	     = adin_alsa_input_name;
      a->ad_pause	     = NULL;
      a->ad_terminate	     = NULL;
      a->ad_resume	     = NULL;
      break;
#endif
#ifdef HAS_OSS
    case SP_INPUT_OSS:
      a->ad_standby 	     = adin_oss_standby;
      a->ad_begin 	     = adin_oss_begin;
      a->ad_end 	     = adin_oss_end;
      a->ad_read 	     = adin_oss_read;
      a->ad_input_name	     = adin_oss_input_name;
      a->ad_pause	     = NULL;
      a->ad_terminate	     = NULL;
      a->ad_resume	     = NULL;
      break;
#endif
#ifdef HAS_ESD
    case SP_INPUT_ESD:
      a->ad_standby 	     = adin_esd_standby;
      a->ad_begin 	     = adin_esd_begin;
      a->ad_end 	     = adin_esd_end;
      a->ad_read 	     = adin_esd_read;
      a->ad_input_name	     = adin_esd_input_name;
      a->ad_pause	     = NULL;
      a->ad_terminate	     = NULL;
      a->ad_resume	     = NULL;
      break;
#endif
#ifdef HAS_PULSEAUDIO
    case SP_INPUT_PULSEAUDIO:
      a->ad_standby 	     = adin_pulseaudio_standby;
      a->ad_begin 	     = adin_pulseaudio_begin;
      a->ad_end 	     = adin_pulseaudio_end;
      a->ad_read 	     = adin_pulseaudio_read;
      a->ad_input_name	     = adin_pulseaudio_input_name;
      a->ad_pause	     = NULL;
      a->ad_terminate	     = NULL;
      a->ad_resume	     = NULL;
      break;
#endif
    default:
      jlog("ERROR: m_adin: invalid input device specified\n");
    }
    break;
#endif
#ifdef USE_NETAUDIO
  case SP_NETAUDIO:
    /* DatLink/NetAudio input */
    a->ad_standby 	   = adin_netaudio_standby;
    a->ad_begin 	   = adin_netaudio_begin;
    a->ad_end 		   = adin_netaudio_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_terminate 	   = NULL;
    a->ad_read 		   = adin_netaudio_read;
    a->ad_input_name	   = adin_netaudio_input_name;
    a->silence_cut_default = TRUE;
    a->enable_thread 	   = TRUE;
    break;
#endif
  case SP_ADINNET:
    /* adinnet network input */
    a->ad_standby 	   = adin_tcpip_standby;
    a->ad_begin 	   = adin_tcpip_begin;
    a->ad_end 		   = adin_tcpip_end;
    a->ad_resume	   = adin_tcpip_send_resume;
    a->ad_pause		   = adin_tcpip_send_pause;
    a->ad_terminate	   = adin_tcpip_send_terminate;
    a->ad_read 		   = adin_tcpip_read;
    a->ad_input_name	   = adin_tcpip_input_name;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
    break;
  case SP_STDIN:
    /* standard input */
    a->ad_standby 	   = adin_stdin_standby;
    a->ad_begin 	   = adin_stdin_begin;
    a->ad_end 		   = NULL;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_terminate 	   = NULL;
    a->ad_read 		   = adin_stdin_read;
    a->ad_input_name	   = adin_stdin_input_name;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
    break;
  case SP_MFCFILE:
  case SP_OUTPROBFILE:
    /* MFC_FILE is not waveform, so special handling on main routine should be done */
    break;
  default:
    jlog("Error: m_adin: unknown input ID\n");
    return FALSE;
  }

  return TRUE;
}


/** 
 * <JA>
 * 音声入力デバイスを初期化し，音入力切出用パラメータをセットアップする. 
 *
 * @param adin [in] AD-in ワークエリア
 * @param jconf [in] 全体設定パラメータ
 * @param arg [in] デバイス依存引数
 * </JA>
 * <EN>
 * Initialize audio device and set up parameters for sound detection.
 * 
 * @param adin [in] AD-in work area
 * @param jconf [in] global configuration parameters
 * @param arg [in] device-specific argument
 * </EN>
 */
static boolean
adin_setup_all(ADIn *adin, Jconf *jconf, void *arg)
{

  if (jconf->input.use_ds48to16) {
    if (jconf->input.use_ds48to16 && jconf->input.sfreq != 16000) {
      jlog("ERROR: m_adin: in 48kHz input mode, target sampling rate should be 16k!\n");
      return FALSE;
    }
    /* setup for 1/3 down sampling */
    adin->ds = ds48to16_new();
    adin->down_sample = TRUE;
    /* set device sampling rate to 48kHz */
    if (adin_standby(adin, 48000, arg) == FALSE) { /* fail */
      jlog("ERROR: m_adin: failed to ready input device\n");
      return FALSE;
    }
  } else {
    adin->ds = NULL;
    adin->down_sample = FALSE;
    if (adin_standby(adin, jconf->input.sfreq, arg) == FALSE) { /* fail */
      jlog("ERROR: m_adin: failed to ready input device\n");
      return FALSE;
    }
  }

  /* set parameter for recording/silence detection */
  if (adin_setup_param(adin, jconf) == FALSE) {
    jlog("ERROR: m_adin: failed to set parameter for input device\n");
    return FALSE;
  }

  adin->input_side_segment = FALSE;

  return TRUE;
}

/** 
 * <JA>
 * 設定パラメータに従い音声入力デバイスをセットアップする. 
 *
 * @param recog [i/o] エンジンインスタンス
 * 
 * </JA>
 * <EN>
 * Set up audio input device according to the jconf configurations.
 * 
 * @param recog [i/o] engine instance
 * </EN>
 *
 * @callgraph
 * @callergraph
 */
boolean
adin_initialize(Recog *recog)
{
  char *arg = NULL;
  ADIn *adin;
  Jconf *jconf;
#ifdef ENABLE_PLUGIN
  FUNC_INT func;
  int sid;
#endif

  adin = recog->adin;
  jconf = recog->jconf;

  jlog("STAT: ###### initialize input device\n");

  /* select input device: file, mic, netaudio, etc... */
#ifdef ENABLE_PLUGIN
  sid = jconf->input.plugin_source;
  if (sid >= 0) {
    /* set plugin properties and functions to adin */
    func = (FUNC_INT) plugin_get_func(sid, "adin_get_configuration");
    if (func == NULL) {
      jlog("ERROR: invalid plugin: adin_get_configuration() not exist\n");
      return FALSE;
    }
    adin->silence_cut_default = (*func)(1);
    adin->enable_thread = (*func)(2);

    adin->ad_standby 	   = (boolean (*)(int, void *)) plugin_get_func(sid, "adin_standby");
    adin->ad_begin 	   = (boolean (*)(char *)) plugin_get_func(sid, "adin_open");
    adin->ad_end 	   = (boolean (*)()) plugin_get_func(sid, "adin_close");
    adin->ad_resume 	   = (boolean (*)()) plugin_get_func(sid, "adin_resume");
    adin->ad_pause 	   = (boolean (*)()) plugin_get_func(sid, "adin_pause");
    adin->ad_terminate 	   = (boolean (*)()) plugin_get_func(sid, "adin_terminate");
    adin->ad_read 	   = (int (*)(SP16 *, int)) plugin_get_func(sid, "adin_read");
    adin->ad_input_name	   = (char * (*)()) plugin_get_func(sid, "adin_input_name");
    if (adin->ad_read == NULL) {
      jlog("ERROR: m_adin: selected plugin has no function adin_read()\n");
      return FALSE;
    }
  } else {
#endif
    /* built-in */
    if (adin_select(adin, jconf->input.speech_input, jconf->input.device) == FALSE) {
      jlog("ERROR: m_adin: failed to select input device\n");
      return FALSE;
    }

    /* set sampling frequency and device-dependent configuration
       (argument is device-dependent) */
    switch(jconf->input.speech_input) {
    case SP_ADINNET:		/* arg: port number */
      arg = mymalloc(100);
      sprintf(arg, "%d", jconf->input.adinnet_port);
      break;
    case SP_RAWFILE:		/* arg: filename of file list (if any) */
      if (jconf->input.inputlist_filename != NULL) {
	arg = mymalloc(strlen(jconf->input.inputlist_filename)+1);
	strcpy(arg, jconf->input.inputlist_filename);
      } else {
	arg = NULL;
      }
      break;
    case SP_STDIN:
      arg = NULL;
      break;
#ifdef USE_NETAUDIO
    case SP_NETAUDIO:		/* netaudio server/port name */
      arg = mymalloc(strlen(jconf->input.netaudio_devname)+1);
      strcpy(arg, jconf->input.netaudio_devname);
      break;
#endif
    }
#ifdef ENABLE_PLUGIN
  }
#endif

  if (adin_setup_all(adin, jconf, arg) == FALSE) {
    return FALSE;
  }

  if (arg != NULL) free(arg);

  return TRUE;
}

/* end of file */
