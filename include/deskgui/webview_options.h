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
  /**
   * @brief Class representing the options for a webview.
   *
   * Note: In Windows, all webviews should share the same WebviewOptions.
   * https://github.com/MicrosoftEdge/WebView2Feedback/issues/2323#issuecomment-1088248488
   *
   */
  class WebviewOptions {
  public:
    using Option = std::variant<bool, int, std::string>;

    /**
     * @brief Set an option with the specified key and value.
     *
     * @param key The key of the option.
     * @param value The value of the option.
     */
    inline void setOption(const std::string& key, const Option& value) { options[key] = value; }

    /**
     * @brief Check if an option with the specified key exists.
     *
     * @param key The key of the option.
     * @return true if the option exists, false otherwise.
     */
    inline bool hasOption(const std::string& key) const {
      return options.find(key) != options.end();
    }

    /**
     * @brief Get the value of an option with the specified key.
     *
     * @tparam T The type of the option value.
     * @param key The key of the option.
     * @return The value of the option.
     * @throws std::runtime_error if the option value is not of the requested type.
     */
    template <typename T> T getOption(const std::string& key) const {
      if (auto it = options.find(key); it != options.end()) {
        try {
          return std::get<T>(it->second);
        } catch ([[maybe_unused]] const std::bad_variant_access& e) {
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
    static constexpr auto kActivateNativeDragAndDrop = "activate-native-drag-and-drop";

    // WebView2-specific options (Windows only)
    /// The path to the user data folder.
    /// Defaults to the system temp folder. Windows only.
    static constexpr auto kWebview2UserDataFolder = "webview2-user-data-folder";
    
    // WebView2-specific options (Windows only)
    /// When false, allows multiple app instances to share the same user data folder.
    /// Defaults to true (exclusive access). Windows only.
    static constexpr auto kWebview2ExclusiveDataFolderAccess = "webview2-exclusive-data-folder-access";

    /// When true, allows single sign-on using the OS primary account.
    /// Defaults to false. Windows only.
    static constexpr auto kWebview2AllowSingleSignOnUsingOSPrimaryAccount = "webview2-allow-sso";

    /// When true, enables custom crash reporting in WebView2.
    /// Defaults to false. Windows only.
    static constexpr auto kWebview2IsCustomCrashReportingEnabled = "webview2-custom-crash-reporting";

    // Cross-platform options
    /// When true, enables ephemeral/private browsing mode. Data is not persisted.
    /// - Windows: Uses WebView2 InPrivate mode via ICoreWebView2ControllerOptions
    /// - macOS: Uses WKWebsiteDataStore.nonPersistentDataStore
    /// - Linux: Uses webkit_web_context_new_ephemeral
    static constexpr auto kEphemeralSession = "ephemeral-session";

    /// When true, creates the webview asynchronously without blocking the UI thread.
    /// Use isReady() or onReady() to check/wait for initialization.
    /// - Windows: Returns immediately, WebView2 initializes in background
    /// - macOS/Linux: No effect (initialization is always synchronous)
    /// Defaults to false (synchronous/blocking).
    static constexpr auto kAsyncCreation = "async-creation";

    /// Scheme name used to serve embedded resources via loadResources/serveResource.
    /// Defaults to "webview", giving an origin of "webview://localhost/". Override
    /// when the default scheme is incompatible with content loaded inside the
    /// webview (e.g. third-party widgets that read window.location and reject
    /// non-standard schemes). The value must be a valid URI scheme — letters,
    /// digits, '+', '-' or '.' — and cannot collide with a scheme already
    /// registered by the underlying engine (Windows WebView2 rejects standard
    /// schemes such as "http"/"https").
    static constexpr auto kCustomSchemeProtocol = "custom-scheme-protocol";

    /// Host used in the resource origin alongside kCustomSchemeProtocol.
    /// Defaults to "localhost", giving an origin of "<protocol>://localhost/".
    static constexpr auto kCustomSchemeHost = "custom-scheme-host";
  };

}  // namespace deskgui
