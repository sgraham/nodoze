// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <windows.h>

LPWSTR FindStartOfSubCommand(LPWSTR orig_command) {
  int num_args;
  LPWSTR* args = CommandLineToArgvW(orig_command, &num_args);
  if (num_args < 2) {
    wprintf(L"nodoze: Could not get subprocess command line.\n");
    exit(1);
  }
  LPWSTR ret = orig_command + wcslen(args[0]);
  if (orig_command[0] == L'"')
    ret += 2;  // Opening and closing quote.
  LocalFree(args);
  while (*ret == L' ')
    ++ret;
  return ret;
}

DWORD WINAPI KeyPresserThread(void*) {
  for (;;) {
    keybd_event(VK_F24,
                static_cast<BYTE>(MapVirtualKey(VK_F24, MAPVK_VK_TO_VSC)),
                0,
                0);
    Sleep(59000);
  }
}

int main() {
  if (!SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED)) {
    wprintf(L"nodoze: Could not SetThreadExecutionState\n");
    exit(1);
  }

  LPWSTR command = FindStartOfSubCommand(GetCommandLine());

  STARTUPINFO startup_info = {sizeof(STARTUPINFO)};
  PROCESS_INFORMATION process_info;
  GetStartupInfo(&startup_info);

  if (!CreateProcess(nullptr,
                     command,
                     nullptr,
                     nullptr,
                     TRUE,
                     0,
                     nullptr,
                     nullptr,
                     &startup_info,
                     &process_info)) {
    wprintf(
        L"nodoze: Could not spawn subprocess, command line:\n"
        L"%ws\n"
        L"You may need to prefix the command with \"cmd /c \".\n",
        command);
    exit(1);
  }
  CloseHandle(process_info.hThread);
  WaitForSingleObject(process_info.hProcess, INFINITE);
  DWORD exit_code;
  GetExitCodeProcess(process_info.hProcess, &exit_code);
  CloseHandle(process_info.hProcess);

  return exit_code;
}
