/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

namespace deskgui {
  class WebviewOptions {
  public:
    using Option = std::variant<bool, int>;

    inline void setOption(const std::string& key, const Option& value) { options[key] = value; }

    inline bool hasOption(const std::string& key) const {
      return options.find(key) != options.end();
    }

    template <typename T> T getOption(const std::string& key) const {
      auto it = options.find(key);
      if (it != options.end()) {
        try {
          return std::get<T>(it->second);
        } catch (const std::bad_variant_access& e) {
          throw std::runtime_error("Option value is not of the requested type");
        }
      }
      // Return default value if option not found
      return T();
    }

  private:
    std::unordered_map<std::string, Option> options;

  public:
    // Windows
    static constexpr auto kRemoteDebuggingPort = "remote-debugging-port";
    static constexpr auto kDisableGpu = "disable-gpu";
    static constexpr auto kAllowFileAccessFromFiles = "allow-file-access-from-files";
  };

}  // namespace deskgui
