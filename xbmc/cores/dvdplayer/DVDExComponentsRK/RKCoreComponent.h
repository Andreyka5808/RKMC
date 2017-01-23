#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DynamicDll.h"
#include "cores/dvdplayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/dvdplayer/DVDStreamInfo.h"
#include "threads/Thread.h"
#include "threads/CriticalSection.h"
#include "guilib/Geometry.h"

#define RK_LIBRARY "librkffplayer.so"

typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned long long int  RK_U64;
typedef int                     RK_RET;
typedef int                     RK_S32;
typedef void*                   RK_PTR;
typedef const void*             RK_ENV;
typedef long long int           RK_PTS;
typedef long long int           RK_DTS;

typedef void (*RKCallBackFn)(RK_ENV env, RK_PTR data, RK_U32 size);

class DllLibRKCodecInterface
{
public:
  virtual ~DllLibRKCodecInterface(){}
  virtual RK_RET RK_CodecInit(RK_PTR info) = 0;
  virtual RK_RET RK_CodecOpen() = 0;
  virtual RK_RET RK_CodecWrite(RK_U32 type, RK_PTR data, RK_U32 isize, RK_PTS pts, RK_DTS dts) = 0;
  virtual RK_RET RK_CodecClose() = 0;
  virtual RK_RET RK_CodecFlush() = 0;
  virtual RK_RET RK_CodecReset() = 0;
  virtual RK_RET RK_CodecPause() = 0;
  virtual RK_RET RK_CodecResume() = 0;
  virtual RK_RET RK_CodecSendCommand(RK_U32 cmd, RK_PTR param) = 0;     
  virtual RK_RET RK_CodecRegisterListener(RK_ENV env, RK_U32 type, RKCallBackFn fn) = 0;
};

class DllLibRKCodec : public DllDynamic, DllLibRKCodecInterface
{
  DECLARE_DLL_WRAPPER(DllLibRKCodec, RK_LIBRARY);

  DEFINE_METHOD1(RK_RET, RK_CodecInit, (RK_PTR p1));
  DEFINE_METHOD0(RK_RET, RK_CodecOpen);
  DEFINE_METHOD5(RK_RET, RK_CodecWrite, (RK_U32 p1, RK_PTR p2, RK_U32 p3, RK_PTS p4, RK_DTS p5));
  DEFINE_METHOD0(RK_RET, RK_CodecClose);
  DEFINE_METHOD0(RK_RET, RK_CodecFlush);
  DEFINE_METHOD0(RK_RET, RK_CodecReset);
  DEFINE_METHOD0(RK_RET, RK_CodecPause);
  DEFINE_METHOD0(RK_RET, RK_CodecResume);
  DEFINE_METHOD2(RK_RET, RK_CodecSendCommand, (RK_U32 p1, RK_PTR p2));
  DEFINE_METHOD3(RK_RET, RK_CodecRegisterListener, (RK_ENV p1, RK_U32 p2, RKCallBackFn p3));

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(RK_CodecInit)
    RESOLVE_METHOD(RK_CodecOpen)
    RESOLVE_METHOD(RK_CodecWrite)
    RESOLVE_METHOD(RK_CodecReset)
    RESOLVE_METHOD(RK_CodecClose)
    RESOLVE_METHOD(RK_CodecFlush)
    RESOLVE_METHOD(RK_CodecPause)
    RESOLVE_METHOD(RK_CodecResume)
    RESOLVE_METHOD(RK_CodecSendCommand)
    RESOLVE_METHOD(RK_CodecRegisterListener)
  END_METHOD_RESOLVE()
};

#define RKMC_SETTING_RKCODEC       "videoplayer.userkcodec"
#define RKMC_SETTING_RKCODEC_VP9   "videoplayer.rkcodec.vp9"
#define RKMC_SETTING_RKCODEC_MPEG2 "videoplayer.rkcodec.mpeg2"
#define RKMC_SETTING_FP3D     "videoplayer.framepacking"
#define RKMC_SETTING_3D24HZ   "videoplayer.force24hzbluray3d"
#define RKMC_SETTING_3DSWITCH "videoplayer.forcehdmi3d"
#define RKMC_SETTING_FRACHDMI "videoplayer.rkmc.frachdmi"

enum RKCodecDecodeType
{
  RK_VIDEO               = 1,
  RK_AUDIO               = 0,
  RK_SUBTITLE            = 2,
  RK_STEREO_OFF          = 0x0000,
  RK_STEREO_LR           = 0x1000,
  RK_STEREO_BT           = 0x2000,
  RK_STEREO_MVC          = 0x4000,
  RK_STEREO_2D           = 0x8000,
  RK_STEREO_MASK         = 0xF000,
};

enum RKListenerType
{
  RK_DECODE              = 1,
  RK_RENDER              = 2	
};

enum RKCodecCommand
{
  RK_CMD_NONE            = 0,
  RK_CMD_SETSPEED        = 1,
  RK_CMD_SYNC            = 2,
  RK_CMD_EOS             = 3,
  RK_CMD_SETRES          = 4,
  RK_CMD_SETCROP         = 5,
  RK_CMD_MAX             = 1000
};

enum RKCodecConfig
{
  RK_CONF_NONE           = 1000,
  RK_CONF_FORCE24HZ_3D,
  RK_CONF_FORCESWITCH_3D,
  RK_CONF_MAX            = 2000
};

enum RKDecodeRetStatus
{
  RK_DECODE_STATE_ERROR          = -1,
  RK_DECODE_STATE_BUFFER         = 0,
  RK_DECODE_STATE_PICTURE        = 1,
  RK_DECODE_STATE_BUFFER_PICTURE = 2,
  RK_DECODE_STATE_BYPASS         = 3
};

enum RKCodecSyncMode
{
  RK_SERVER_SYNC   = 1,
  RK_CLIENT_NOTIFY = 2
};

struct RKCodecStreamInfo_t {
  RK_U32 codec;
  RK_U32 type;
  RK_U32 flags;
  RK_PTR filename;

  RK_U32 fpsscale;
  RK_U32 fpsrate;
  RK_U32 rfpsscale;
  RK_U32 rfpsrate;
  RK_U32 height; 
  RK_U32 width;
  RK_U32 profile; 
  RK_U32 ptsinvalid;
  RK_U32 bitsperpixel;
  RK_U32 stereo_mode;
	
  RK_U32 channels;
  RK_U32 samplerate;
  RK_U32 bitrate;
  RK_U32 blockalign;
  RK_U32 bitspersample;
	
  RK_PTR extradata; 
  RK_U32 extrasize;
  RK_U32 codec_tag;

  RK_S32 colortrc;

};

struct RKCodecDisplayInfo_t {
  RK_U32 type;
  RK_PTS raw;
  RK_PTS pts;
  RK_U32 eos;
  RK_PTS record;
};

typedef struct RKCodecStreamInfo_t RKCodecStreamInfo;
typedef struct RKCodecDisplayInfo_t RKCodecDisplayInfo;

class CRKCodec : public CThread
{
public:
  CRKCodec();
  ~CRKCodec();
  bool OpenDecoder(CDVDStreamInfo &hints);
  void CloseDecoder();
  int DecodeVideo(uint8_t *pData, size_t size, double dts, double pts);
  void Reset();
  void Flush();
  void SetSpeed(int speed);
  void SendCommand(RK_U32 p1, RK_PTR p2);
  static void OnDisplayEvent(RK_ENV env, RK_PTR data, RK_U32 size);
  void SetDisplayInfo(const RKCodecDisplayInfo* info);
  RKCodecDisplayInfo* GetDisplayInfo();
  int GetSyncMode();
  void SubmitEOS();
  bool SubmittedEOS();
  bool IsEOS();
  
protected:
  virtual void  Process();
  void UpdatePlayStatus();
  static void RenderUpdateCallBack(const void *ctx, const CRect &SrcRect, const CRect &DestRect);
  void UpdateRenderRect(const CRect &SrcRect, const CRect &DestRect);
  RK_U32 GetStereoMode();
  void UpdateRenderStereo(bool flag = false);
  void UpdateRenderFracHDMI();
  bool Support3D(int width, int height, float fps, uint32_t mode);
  void SetNative3DResolution(RK_U32 res3d);
  void SetNativeFracResolution(bool enable);

private:
  void ConfigureSetting();
  void SendConfigure(RK_U32 config, RK_PTR param = NULL);
  
private:
  CDVDStreamInfo  m_hints;
  RKCodecStreamInfo m_streamInfo;
  RKCodecDisplayInfo m_displayInfo;
  CRect m_displayResolution;
  CRect m_displayCrop;
  DllLibRKCodec *m_dll;
  bool m_bLoad;
  bool m_bReady;
  bool m_bOpen;
  bool m_bRender;
  bool m_bSubmittedEos;
  RK_U32 m_bSyncStatus;
  RK_S32 m_iSyncMode;
  double m_lfSyncThreshold;
  RK_S32 m_iSpeed;
  RK_S32 m_iNextSpeed;

  RK_U32 m_iStereoMode;
  
};




