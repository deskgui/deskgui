/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "webview_platform_win32.h"

#include <Shlobj.h>
#include <rapidjson/document.h>

#include "js/drop.h"
#include "utils/strings.h"

using namespace deskgui;
using namespace deskgui::utils;

using Platform = Webview::Impl::Platform;

namespace detail {
  bool isProcessRunning(DWORD processId) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process == nullptr) {
      return false;
    }
    DWORD exitCode;
    bool running = GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE;
    CloseHandle(process);
    return running;
  }

  void cleanupOrphanWebviewFolders(const std::filesystem::path& basePath) {
    if (!std::filesystem::exists(basePath)) {
      return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
      if (!entry.is_directory()) {
        continue;
      }

      try {
        DWORD pid = std::stoul(entry.path().filename().string());
        if (!isProcessRunning(pid)) {
          std::filesystem::remove_all(entry.path());
        }
      } catch (...) {
        // Folder name is not a valid PID, skip it
      }
    }
  }
}  // namespace detail

bool Platform::createWebviewInstance(std::string_view appName, HWND hWnd,
                                     const WebviewOptions& options) {
  using namespace Microsoft::WRL;

  HRESULT hr = OleInitialize(nullptr);
  if (FAILED(hr)) {
    return false;
  }

  // Read async mode from options
  asyncMode_ = options.hasOption(WebviewOptions::kAsyncCreation)
               && options.getOption<bool>(WebviewOptions::kAsyncCreation);

  ComPtr environmentOptions = Make<CoreWebView2EnvironmentOptions>();

  std::wstring additionalArguments;

  if (options.hasOption(WebviewOptions::kRemoteDebuggingPort)) {
    const int port = options.getOption<int>(WebviewOptions::kRemoteDebuggingPort);
    additionalArguments += L"--remote-debugging-port=" + std::to_wstring(port);
    additionalArguments += L" ";
  }

  if (options.hasOption(WebviewOptions::kDisableGpu)) {
    if (const auto option = options.getOption<bool>(WebviewOptions::kDisableGpu); option) {
      additionalArguments += L"--disable-gpu";
      additionalArguments += L" ";
    }
  }

  if (options.hasOption(WebviewOptions::kAllowFileAccessFromFiles)) {
    if (const auto option = options.getOption<bool>(WebviewOptions::kAllowFileAccessFromFiles);
        option) {
      additionalArguments += L"--allow-file-access-from-files";
      additionalArguments += L" ";
    }
  }

  environmentOptions->put_AdditionalBrowserArguments(additionalArguments.c_str());

  // Register custom scheme
  auto customSchemeRegistration
      = Microsoft::WRL::Make<CoreWebView2CustomSchemeRegistration>(Webview::Impl::kWOrigin.c_str());
  customSchemeRegistration->put_TreatAsSecure(true);
  customSchemeRegistration->put_HasAuthorityComponent(true);
  std::array<ICoreWebView2CustomSchemeRegistration*, 1> registrations
      = {customSchemeRegistration.Get()};
  environmentOptions->SetCustomSchemeRegistrations(registrations.size(), registrations.data());

  // Configure SSO option (defaults to false)
  const bool allowSSO
      = options.hasOption(WebviewOptions::kWebview2AllowSingleSignOnUsingOSPrimaryAccount)
        && options.getOption<bool>(WebviewOptions::kWebview2AllowSingleSignOnUsingOSPrimaryAccount);
  environmentOptions->put_AllowSingleSignOnUsingOSPrimaryAccount(allowSSO ? TRUE : FALSE);

  // Configure crash reporting (defaults to false)
  const bool customCrashReporting
      = options.hasOption(WebviewOptions::kWebview2IsCustomCrashReportingEnabled)
        && options.getOption<bool>(WebviewOptions::kWebview2IsCustomCrashReportingEnabled);
  environmentOptions->put_IsCustomCrashReportingEnabled(customCrashReporting ? TRUE : FALSE);

  // Configure exclusive data folder access (defaults to true)
  const bool exclusiveAccess
      = !options.hasOption(WebviewOptions::kWebview2ExclusiveDataFolderAccess)
        || options.getOption<bool>(WebviewOptions::kWebview2ExclusiveDataFolderAccess);
  if (!exclusiveAccess) {
    environmentOptions->put_ExclusiveUserDataFolderAccess(FALSE);
  }

  // Use LocalAppData instead of Temp - more reliable in packaged apps (MSIX/UWP hosts)
  std::filesystem::path userDataFolder;
  PWSTR localAppDataPath = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath))) {
    userDataFolder = localAppDataPath;
    CoTaskMemFree(localAppDataPath);
  } else {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    userDataFolder = tempPath;
  }
  userDataFolder /= std::wstring(appName.begin(), appName.end());

  // Configure process isolation and cleanup
  const bool isolateByProcess
      = options.hasOption(WebviewOptions::kWebview2IsolateUserDataByProcess)
        && options.getOption<bool>(WebviewOptions::kWebview2IsolateUserDataByProcess);

  if (isolateByProcess) {
    const bool cleanupOrphans
        = options.hasOption(WebviewOptions::kWebview2CleanupOrphanedUserDataOnCreate)
          && options.getOption<bool>(WebviewOptions::kWebview2CleanupOrphanedUserDataOnCreate);
    if (cleanupOrphans) {
      detail::cleanupOrphanWebviewFolders(userDataFolder);
    }
    userDataFolder /= std::to_wstring(GetCurrentProcessId());
  }

  // Store ephemeral option to pass to controller creation
  ephemeralSession_ = options.hasOption(WebviewOptions::kEphemeralSession)
                      && options.getOption<bool>(WebviewOptions::kEphemeralSession);

  std::atomic_flag flag = ATOMIC_FLAG_INIT;
  flag.test_and_set();

  CreateCoreWebView2EnvironmentWithOptions(
      nullptr, userDataFolder.wstring().c_str(), environmentOptions.Get(),
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, hWnd, &flag]([[maybe_unused]] HRESULT result,
                              ICoreWebView2Environment* environment) {
            return this->onCreateEnvironmentCompleted(environment, hWnd, flag);
          })
          .Get());

  // Async mode: return immediately, initialization completes via callback
  if (asyncMode_) {
    return true;
  }

  // Sync mode: block until environment is ready
  MSG msg = {};
  while (flag.test_and_set() && GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return webviewController && webview;
}

HRESULT Platform::onCreateEnvironmentCompleted(ICoreWebView2Environment* environment, HWND hWnd,
                                               std::atomic_flag& flag) {
  // Check if environment creation failed
  if (!environment) {
    flag.clear();
    return E_FAIL;
  }

  // Query for ICoreWebView2Environment10 to access CreateCoreWebView2ControllerOptions
  wil::com_ptr<ICoreWebView2Environment10> environment10;
  if (ephemeralSession_ && SUCCEEDED(environment->QueryInterface(IID_PPV_ARGS(&environment10)))) {
    wil::com_ptr<ICoreWebView2ControllerOptions> controllerOptions;
    if (SUCCEEDED(environment10->CreateCoreWebView2ControllerOptions(&controllerOptions))) {
      controllerOptions->put_IsInPrivateModeEnabled(TRUE);

      environment10->CreateCoreWebView2ControllerWithOptions(
          hWnd, controllerOptions.get(),
          Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
              [this, &flag]([[maybe_unused]] HRESULT result, ICoreWebView2Controller* controller) {
                this->onCreateCoreWebView2ControllerCompleted(controller);
                flag.clear();
                return S_OK;
              })
              .Get());
      return S_OK;
    }
  }

  // Fallback to standard controller creation (no private mode)
  environment->CreateCoreWebView2Controller(
      hWnd,
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [this, &flag]([[maybe_unused]] HRESULT result, ICoreWebView2Controller* controller) {
            this->onCreateCoreWebView2ControllerCompleted(controller);
            flag.clear();
            return S_OK;
          })
          .Get());
  return S_OK;
}

void Platform::onCreateCoreWebView2ControllerCompleted(ICoreWebView2Controller* controller) {
  if (controller) {
    webviewController = controller;
    webviewController->get_CoreWebView2(&webview);
  }

  // For async mode: complete initialization and notify when controller is ready
  if (asyncMode_ && webviewImpl_ && webview && webviewController) {
    webviewImpl_->initialize(options_);
  }
}

bool Platform::handleDragAndDrop(ICoreWebView2WebMessageReceivedEventArgs* event) {
  wil::com_ptr<ICoreWebView2WebMessageReceivedEventArgs2> args2
      = wil::com_ptr<ICoreWebView2WebMessageReceivedEventArgs>(event)
            .query<ICoreWebView2WebMessageReceivedEventArgs2>();
  if (!args2) {
    return false;
  }

  // Get the message to extract coordinates
  wil::unique_cotaskmem_string message;
  event->TryGetWebMessageAsString(&message);

  if (!message) {
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(ws2s(message.get()).c_str());

  if (!doc.HasParseError() && doc.IsObject() && doc.HasMember("type")
      && doc["type"].GetString() == std::string("deskgui-files-dropped") && doc.HasMember("x")
      && doc["x"].IsNumber() && doc.HasMember("y") && doc["y"].IsNumber()) {
    double x = doc["x"].GetDouble();
    double y = doc["y"].GetDouble();

    wil::com_ptr<ICoreWebView2ObjectCollectionView> objectsCollection;
    args2->get_AdditionalObjects(&objectsCollection);
    unsigned int length;

    objectsCollection->get_Count(&length);
    std::vector<std::filesystem::path> paths;

    for (unsigned int i = 0; i < length; i++) {
      wil::com_ptr<IUnknown> object;

      objectsCollection->GetValueAtIndex(i, &object);
      // Note that objects can be null.
      if (object) {
        wil::com_ptr<ICoreWebView2File> file = object.query<ICoreWebView2File>();
        if (file) {
          wil::unique_cotaskmem_string path;
          file->get_Path(&path);
          paths.emplace_back(ws2s(path.get()));
        }
      }
    }

    webview->ExecuteScript(s2ws(js::createDropEvent(paths, x, y)).c_str(), nullptr);
    return true;
  }

  return false;
};