/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#ifdef _WIN32

#  include <windows.h>

#  include <regex>
#  include <string>

namespace deskgui::utils {

  inline std::wstring s2ws(const std::string &str) {
    int size_needed
        = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0],
                        size_needed);
    return wstr;
  }

  inline std::string ws2s(const std::wstring &wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()),
                                          nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0],
                        size_needed, nullptr, nullptr);
    return str;
  }

  inline std::string escapeBackslashes(const std::string &str) {
    std::string escapedStr = str;
    std::regex backslashRegex(R"(\\)");
    escapedStr = std::regex_replace(escapedStr, backslashRegex, R"(\\)");
    return escapedStr;
  }

}  // namespace deskgui::utils
#else
static_assert(false, "Wide string conversions are only available on Windows platforms.");
#endif
