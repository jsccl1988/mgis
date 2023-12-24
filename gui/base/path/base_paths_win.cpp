// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <shlobj.h>
#include <Shlwapi.h>

#include "gui/base/path/base_paths.h"

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace gui {
bool PathExists(const std::wstring& path) {
  return (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
}

bool DirectoryExists(const std::wstring& path) {
  DWORD fileattr = GetFileAttributes(path.c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

bool DirectoryAsDLLSearchPath(const std::wstring& path) {
  if (!DirectoryExists(path))
    return false;

  typedef BOOL(WINAPI *SetCurrentDirectoryFnPtr)(__in_opt LPCTSTR);
  SetCurrentDirectoryFnPtr pfnCurrentDirectory = NULL;

  HMODULE hKernel32 = ::LoadLibraryW(L"Kernel32.dll");
  if (hKernel32) {
#ifdef _UNICODE
    pfnCurrentDirectory =
      (SetCurrentDirectoryFnPtr)::GetProcAddress(hKernel32, "SetDllDirectoryW");
#else
    pfnCurrentDirectory =
      (SetCurrentDirectoryFnPtr)::GetProcAddress(hKernel32, "SetDllDirectoryA");
#endif  // _UNICODE

    if (pfnCurrentDirectory) {
      BOOL ok = pfnCurrentDirectory(path.c_str());
      if (!ok) {
        DWORD dw = GetLastError();
        return false;
      }
    }
  }

  return true;
}

bool GetCurrentDirectory(std::wstring* dir) {
  wchar_t system_buffer[MAX_PATH];
  system_buffer[0] = 0;
  DWORD len = ::GetCurrentDirectory(MAX_PATH, system_buffer);
  PathRemoveFileSpec(system_buffer);
  if (len == 0 || len > MAX_PATH)
    return false;
  // TODO(evanm): the old behavior of this function was to always strip the
  // trailing slash.  We duplicate this here, but it shouldn't be necessary
  // when everyone is using the appropriate FilePath APIs.
  std::wstring dir_str(system_buffer);
  *dir = dir_str;
  return true;
}

// Sets the current working directory for the process.
bool SetCurrentDirectory(const std::wstring& directory) {
  // ThreadRestrictions::AssertIOAllowed();
  BOOL ret = ::SetCurrentDirectory(directory.c_str());
  return ret != 0;
}

bool PathProvider(int key, std::wstring* result) {
  // We need to go compute the value. It would be nice to support paths with
  // names longer than MAX_PATH, but the system functions don't seem to be
  // designed for it either, with the exception of GetTempPath (but other
  // things will surely break if the temp path is too long, so we don't bother
  // handling it.
  wchar_t system_buffer[MAX_PATH];
  system_buffer[0] = 0;

  std::wstring cur;
  switch (key) {
    case gui::FILE_EXE:
      GetModuleFileName(NULL, system_buffer, MAX_PATH);
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_EXE:
      GetModuleFileName(NULL, system_buffer, MAX_PATH);
      system_buffer[MAX_PATH - 1] = L'\0';
      PathRemoveFileSpec(system_buffer);
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_TEMP:
      GetTempPathW(MAX_PATH, system_buffer);
      cur = std::wstring(system_buffer);
      break;
      break;
    case gui::FILE_MODULE: {
      // the resource containing module is assumed to be the one that
      // this code lives in, whether that's a dll or exe
      HMODULE this_module = reinterpret_cast<HMODULE>(&__ImageBase);
      GetModuleFileName(this_module, system_buffer, MAX_PATH);
      cur = std::wstring(system_buffer);
      break;
    }
    case gui::FILE_MODULE_DIR: {
      // the resource containing module is assumed to be the one that
      // this code lives in, whether that's a dll or exe
      HMODULE this_module = reinterpret_cast<HMODULE>(&__ImageBase);
      GetModuleFileName(this_module, system_buffer, MAX_PATH);
      system_buffer[MAX_PATH - 1] = L'\0';
      PathRemoveFileSpec(system_buffer);
      cur = std::wstring(system_buffer);
      break;
    }
    case gui::DIR_WINDOWS:
      GetWindowsDirectory(system_buffer, MAX_PATH);
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_SYSTEM:
      GetSystemDirectory(system_buffer, MAX_PATH);
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_PROGRAM_FILES:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_IE_INTERNET_CACHE:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_INTERNET_CACHE, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_COMMON_START_MENU:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_START_MENU:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
                                 system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_COMMON_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_LOCAL_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_USER_DESKTOP:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_COMMON_DESKTOP:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_USER_QUICK_LAUNCH:
      if (!PathProvider(gui::DIR_APP_DATA, &cur))
        return false;
      // According to various sources, appending
      // "Microsoft\Internet Explorer\Quick Launch" to %appdata% is the only
      // reliable way to get the quick launch folder across all versions of
      // Windows.
      // http://stackoverflow.com/questions/76080/how-do-you-reliably-get-the-quick-
      // http://www.microsoft.com/technet/scriptcenter/resources/qanda/sept05/hey0901.mspx
      cur += L"\\Microsoft";
      cur += L"\\Internet Explorer";
      cur += L"\\Quick Launch";
      break;
    case gui::DIR_TASKBAR_PINS:
      if (!PathProvider(gui::DIR_USER_QUICK_LAUNCH, &cur))
        return false;

      cur += L"\\User Pinned";
      cur += L"\\TaskBar";
      break;
    case gui::DIR_WINDOWS_FONTS:
      if (FAILED(SHGetFolderPath(
              NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = std::wstring(system_buffer);
      break;
    case gui::DIR_APP_DATA_HBGUI:
      if (!PathProvider(gui::DIR_APP_DATA, &cur))
        return false;
      cur += L"\\Baidu\\hbdemo";
      break;
    default:
      return false;
  }

  *result = cur;
  return true;
}

}  // namespace gui
