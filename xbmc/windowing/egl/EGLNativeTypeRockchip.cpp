/*
 *      Copyright (C) 2011-2014 Team XBMC
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

#include <stdlib.h>

#include "system.h"
#include <EGL/egl.h>
#include "EGLNativeTypeRockchip.h"
#include "utils/log.h"
#include "guilib/gui3d.h"
#include "android/activity/XBMCApp.h"
#include "android/jni/Build.h"
#include "utils/StringUtils.h"
#include "utils/SysfsUtils.h"
#include "utils/RegExp.h"

CEGLNativeTypeRockchip::CEGLNativeTypeRockchip()
{
  display = NULL;
  has_api = false;
}

CEGLNativeTypeRockchip::~CEGLNativeTypeRockchip()
{
  if (display)
    delete display;
}

bool CEGLNativeTypeRockchip::CheckCompatibility()
{
  if (StringUtils::StartsWithNoCase(CJNIBuild::HARDWARE, "rk3") && SysfsUtils::Has("/system/lib/librkffplayer.so"))  // Rockchip
  {
    if (SysfsUtils::Has("/sys/class/display/display0.HDMI/mode") || SysfsUtils::Has("/sys/class/display/HDMI/mode"))
    {
      CLog::Log(LOGDEBUG, "RKEGL: Detected");
      return true;
    }
    else
      CLog::Log(LOGERROR, "RKEGL: no rw on /sys/class/display/display0.HDMI/mode");
  }
  return false;
}

bool CEGLNativeTypeRockchip::SysModeToResolution(std::string mode, RESOLUTION_INFO *res) const
{
  if (!res)
    return false;

  res->iWidth = 0;
  res->iHeight= 0;

  if(mode.empty())
    return false;

  std::string fromMode = mode;
  if (!isdigit(mode[0]))
    fromMode = StringUtils::Mid(mode, 2);
  StringUtils::Trim(fromMode);

  CRegExp split(true);
  split.RegComp("([0-9]+)x([0-9]+)([pi])-([0-9]+)");
  if (split.RegFind(fromMode) < 0)
    return false;

  int w = atoi(split.GetMatch(1).c_str());
  int h = atoi(split.GetMatch(2).c_str());
  std::string p = split.GetMatch(3);
  int r = atoi(split.GetMatch(4).c_str());
  CLog::Log(LOGDEBUG,"SysModeToResolution w:%d h:%d r:%d",w,h,r);
  res->iWidth = w;
  res->iHeight= h;
  res->iScreenWidth = w;
  res->iScreenHeight= h;
  res->fRefreshRate = r;
  res->dwFlags = p[0] == 'p' ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;

  res->iScreen       = 0;
  res->bFullScreen   = true;
  res->iSubtitles    = (int)(0.965 * res->iHeight);
  res->fPixelRatio   = 1.0f;
  res->strMode       = StringUtils::Format("%dx%d @ %.2f%s - Full Screen", res->iScreenWidth, res->iScreenHeight, res->fRefreshRate,
                                           res->dwFlags & D3DPRESENTFLAG_INTERLACED ? "i" : "");
  res->strId         = mode;

  return res->iWidth > 0 && res->iHeight> 0;
}

bool CEGLNativeTypeRockchip::GetNativeResolution(RESOLUTION_INFO *res) const
{
  CEGLNativeTypeAndroid::GetNativeResolution(&m_fb_res);

  std::string mode;
  RESOLUTION_INFO hdmi_res;
  if ((SysfsUtils::GetString("/sys/class/display/display0.HDMI/mode", mode) == 0 
    || SysfsUtils::GetString("/sys/class/display/HDMI/mode", mode) == 0) && SysModeToResolution(mode, &hdmi_res))
  {
    m_curHdmiResolution = mode;
    *res = hdmi_res;
    res->iWidth = m_fb_res.iWidth;
    res->iHeight = m_fb_res.iHeight;
    res->iSubtitles = (int)(0.965 * res->iHeight);
  }
  else
    *res = m_fb_res;
  CLog::Log(LOGDEBUG,"CEGLNativeTypeRockchip current res:%dx%d-%f hz",res->iScreenWidth,res->iScreenHeight,res->fRefreshRate);
  return true;
}

bool CEGLNativeTypeRockchip::SetNativeResolution(const RESOLUTION_INFO &res)
{
  CLog::Log(LOGDEBUG,"CEGLNativeTypeRockchip res:%dx%d-%f hz", res.iScreenWidth, res.iScreenHeight, res.fRefreshRate);
  switch((int)(res.fRefreshRate*10))
  {
    default:
    case 600:
      switch(res.iScreenWidth)
      {
        case 1280:
          SetDisplayResolution("1280x720p-60");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1920x1080i-60");
          else
            SetDisplayResolution("1920x1080p-60");
          break;
        case 3840:
          if (CJNIBase::GetSDKVersion() >= 21)
            SetDisplayResolution("3840x2160p-60(YCbCr420)");
          else
            SetDisplayResolution("3840x2160p-60");
          break;
        case 4096:
          if (CJNIBase::GetSDKVersion() >= 21)
            SetDisplayResolution("4096x2160p-60(YCbCr420)");
          else
            SetDisplayResolution("4096x2160p-60");
          break;
        default:
          SetDisplayResolution("1920x1080p-60");
          break;
      }
      break;
    case 500:
      switch(res.iScreenWidth)
      {
        case 1280:
          SetDisplayResolution("1280x720p-50");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1920x1080i-50");
          else
            SetDisplayResolution("1920x1080p-50");
          break;
        case 3840:
          if (CJNIBase::GetSDKVersion() >= 21)
            SetDisplayResolution("3840x2160p-50(YCbCr420)");
          else
            SetDisplayResolution("3840x2160p-50");
          break;
        case 4096:
          if (CJNIBase::GetSDKVersion() >= 21)
            SetDisplayResolution("4096x2160p-50(YCbCr420)");
          else
            SetDisplayResolution("4096x2160p-50");
          break;
        default:
          SetDisplayResolution("1920x1080p-60");
          break;
      }
      break;
    case 300:
      switch(res.iScreenWidth)
      {
        case 3840:
          SetDisplayResolution("3840x2160p-30");
          break;
        case 4096:
          SetDisplayResolution("4096x2160p-30");
          break;
        default:
          SetDisplayResolution("1920x1080p-30");
          break;
      }
      break;
    case 250:
      switch(res.iScreenWidth)
      {
        case 3840:
          SetDisplayResolution("3840x2160p-25");
          break;
        case 4096:
          SetDisplayResolution("4096x2160p-25");
          break;
        default:
          SetDisplayResolution("1920x1080p-25");
          break;
      }
      break;
    case 240:
      switch(res.iScreenWidth)
      {
        case 3840:
          SetDisplayResolution("3840x2160p-24");
          break;
        case 4096:
          SetDisplayResolution("4096x2160p-24");
          break;
        default:
          SetDisplayResolution("1920x1080p-24");
          break;
      }
      break;
  }

  return true;
}

void CEGLNativeTypeRockchip::SetNative3DResolution(const RESOLUTION_INFO &res)
{
  int out;
  switch(res.dwFlags)
  {
    case D3DPRESENTFLAG_MODE3DSBS: out = 8; break;
    case D3DPRESENTFLAG_MODE3DTB: out = 6; break;
    case D3DPRESENTFLAG_MODE3DMVC: out = 0; break;
    default: out = -1;
  }

  CLog::Log(LOGDEBUG, "CEGLNativeTypeRockchip SetNative3DResolution mode = %d", out);
  if (SysfsUtils::HasRW("/sys/class/display/display0.HDMI/3dmode"))
  {
    SysfsUtils::SetInt("/sys/class/display/display0.HDMI/3dmode", out);
  }
  else if (SysfsUtils::HasRW("/sys/class/display/HDMI/3dmode"))
  {
    SysfsUtils::SetInt("/sys/class/display/HDMI/3dmode", out);
  }
  else
  {
    if (!display)
      display = new CJNIDisplayOutputManager();
    display->set3DMode(CJNIDisplayOutputManager::MAIN_DISPLAY, CJNIDisplayOutputManager::DISPLAY_IFACE_HDMI, out);
  }
  
}


bool CEGLNativeTypeRockchip::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  CEGLNativeTypeAndroid::GetNativeResolution(&m_fb_res);
  std::string valstr;
  if (SysfsUtils::GetString("/sys/class/display/display0.HDMI/modes", valstr) < 0 
    && SysfsUtils::GetString("/sys/class/display/HDMI/modes", valstr) < 0)
    return false;
  std::vector<std::string> probe_str = StringUtils::Split(valstr, "\n");

  std::string val3dstr;
  if (SysfsUtils::GetString("/sys/class/display/display0.HDMI/3dmode", val3dstr) < 0 
    && SysfsUtils::GetString("/sys/class/display/HDMI/3dmode", val3dstr) < 0)
    val3dstr = "";
  std::vector<std::string> probe3d_str = StringUtils::Split(val3dstr, "\n");
  
  resolutions.clear();
  RESOLUTION_INFO res;
  
  for (size_t i = 0; i < probe_str.size(); i++)
  {
    if(SysModeToResolution(probe_str[i].c_str(), &res))
    {
      res.iWidth = m_fb_res.iWidth;
      res.iHeight = m_fb_res.iHeight;
      res.iSubtitles    = (int)(0.965 * res.iHeight);
      for (size_t j = 0; j < probe3d_str.size(); j++)
      {
        if (StringUtils::StartsWithNoCase(probe3d_str[j], probe_str[i].c_str()))
        {
          std::vector<std::string> tmp = StringUtils::Split(probe3d_str[j], ",");
          if (tmp.size() == 2 && StringUtils::IsInteger(tmp[1]))
          {
            int ires3d = atoi(tmp[1].c_str());
            if (ires3d & 1)
            {
              RESOLUTION_INFO res3d = res;
              res3d.dwFlags = D3DPRESENTFLAG_MODE3DMVC;
              resolutions.push_back(res3d);
            }
            if (ires3d & (1 << 6))
            {
              RESOLUTION_INFO res3d = res;
              res3d.dwFlags = D3DPRESENTFLAG_MODE3DSBS;
              resolutions.push_back(res3d);
            }
            if (ires3d & (1 << 8))
            {
              RESOLUTION_INFO res3d = res;
              res3d.dwFlags = D3DPRESENTFLAG_MODE3DTB;
              resolutions.push_back(res3d);
            }
          }
        }
      }
      CLog::Log(LOGDEBUG,"ProbeResolutions width:%d height:%d ScreenWidth:%d ScreenHeight:%d fps:%f", 
        res.iWidth, res.iHeight, res.iScreenWidth, res.iScreenHeight, res.fRefreshRate);
      resolutions.push_back(res);
    }
  }
  return resolutions.size() > 0;

}

bool CEGLNativeTypeRockchip::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  return GetNativeResolution(res);
}

bool CEGLNativeTypeRockchip::SetDisplayResolution(const char *resolution)
{
  CLog::Log(LOGDEBUG,"CEGLNativeTypeRockchip SetDisplayResolution %s",resolution);
  // current 3d mode 
  if (Get3DMode() >= 0)
    return false;
  
  if (m_curHdmiResolution == resolution)
    return true;

  // switch display resolution
  std::string out = resolution;
  if (SysfsUtils::HasRW("/sys/class/display/display0.HDMI/mode"))
  {
    out += '\n';
    if (SysfsUtils::SetString("/sys/class/display/display0.HDMI/mode", out.c_str()) < 0)
      return false;
  }
  else if (SysfsUtils::HasRW("/sys/class/display/HDMI/mode"))
  {
    out += '\n';
    if (SysfsUtils::SetString("/sys/class/display/HDMI/mode", out.c_str()) < 0)
      return false;
  }
  else
  {
    if (!display)
      display = new CJNIDisplayOutputManager();
    display->setMode(CJNIDisplayOutputManager::MAIN_DISPLAY, CJNIDisplayOutputManager::DISPLAY_IFACE_HDMI, out);
  }

  m_curHdmiResolution = resolution;

  return true;
}

int CEGLNativeTypeRockchip::Get3DMode()
{
  std::string valstr;
  if (SysfsUtils::GetString("/sys/class/display/display0.HDMI/3dmode", valstr) < 0 
    && SysfsUtils::GetString("/sys/class/display/HDMI/3dmode", valstr) < 0)
    return -1;
  
  std::vector<std::string> probe_str = StringUtils::Split(valstr, "\n");
  for (size_t i = 0; i < probe_str.size(); i++)
  {
    if (StringUtils::StartsWith(probe_str[i], "cur3dmode="))
    {
      int mode;
      sscanf(probe_str[i].c_str(),"cur3dmode=%d",&mode);
      return mode;
    }
  }
  return -1;
}



