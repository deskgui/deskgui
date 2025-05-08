/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#include "webview_windows_impl.h"

#include <rapidjson/document.h>

#include "js/drop.h"
#include "utils/strings.h"

using namespace deskgui;
using namespace deskgui::utils;

bool Webview::Impl::createWebviewInstance(const std::string& appName, HWND hWnd,
                                          const WebviewOptions& options) {
  using namespace Microsoft::WRL;

  HRESULT hr = OleInitialize(nullptr);
  if (FAILED(hr)) {
    return false;
  }

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
      = Microsoft::WRL::Make<CoreWebView2CustomSchemeRegistration>(Webview::kWOrigin.c_str());
  customSchemeRegistration->put_TreatAsSecure(true);
  customSchemeRegistration->put_HasAuthorityComponent(true);
  std::array<ICoreWebView2CustomSchemeRegistration*, 1> registrations
      = {customSchemeRegistration.Get()};
  environmentOptions->SetCustomSchemeRegistrations(registrations.size(), registrations.data());

  std::wstring temp;
  wil::GetEnvironmentVariableW(L"TEMP", temp);
  temp += L"\\" + std::wstring(appName.begin(), appName.end());

  std::atomic_flag flag = ATOMIC_FLAG_INIT;
  flag.test_and_set();

  CreateCoreWebView2EnvironmentWithOptions(
      nullptr, temp.c_str(), environmentOptions.Get(),
      Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, hWnd, &flag]([[maybe_unused]] HRESULT result,
                              ICoreWebView2Environment* environment) {
            return this->onCreateEnvironmentCompleted(environment, hWnd, flag);
          })
          .Get());

  MSG msg = {};
  while (flag.test_and_set() && GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (!webviewController || !webview) {
    return false;
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  webview->get_Settings(&settings);

  // Enable web messages and scripts
  settings->put_IsWebMessageEnabled(true);
  settings->put_IsScriptEnabled(true);

  // Disable default settings
  settings->put_AreDevToolsEnabled(false);
  settings->put_AreDefaultContextMenusEnabled(false);
  settings->put_IsZoomControlEnabled(false);
  settings->put_AreDefaultScriptDialogsEnabled(false);
  settings->put_AreHostObjectsAllowed(false);
  settings->put_IsStatusBarEnabled(false);

  if (auto settings3 = settings.try_query<ICoreWebView2Settings3>(); settings3) {
    settings3->put_AreBrowserAcceleratorKeysEnabled(false);
  }

  if (auto settings4 = settings.try_query<ICoreWebView2Settings4>(); settings4) {
    settings4->put_IsGeneralAutofillEnabled(false);
    settings4->put_IsPasswordAutosaveEnabled(false);
  }

  return true;
}

HRESULT Webview::Impl::onCreateEnvironmentCompleted(ICoreWebView2Environment* environment,
                                                    HWND hWnd, std::atomic_flag& flag) {
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

void Webview::Impl::onCreateCoreWebView2ControllerCompleted(ICoreWebView2Controller* controller) {
  if (controller) {
    webviewController = controller;
    webviewController->get_CoreWebView2(&webview);
  }
}

bool Webview::Impl::handleDragAndDrop(ICoreWebView2WebMessageReceivedEventArgs* event) {
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