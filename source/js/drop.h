/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "js/events.h"

#ifdef WIN32
#include "utils/strings.h"
#endif

namespace deskgui::js {

  inline std::string createDropEvent(const std::vector<std::filesystem::path>& paths, double x,
                                     double y) {
    std::stringstream ss;
    ss << "(() => {";

    // Create array of paths
    ss << "  const paths = [";
    for (size_t i = 0; i < paths.size(); ++i) {
      if (i > 0) ss << ",";
#ifdef WIN32
      ss << "'" << utils::escapeBackslashes(paths[i].string()) << "'";
#else
      ss << "'" << paths[i].string() << "'";
#endif
    }
    ss << "];";

    // Get element under mouse position
    ss << "  const element = document.elementFromPoint(" << x << ", " << y << ");";

    // Create custom event with paths data
    ss << "  const customEvent = new CustomEvent('" << kDropEventName << "', {";
    ss << "    bubbles: true,";
    ss << "    cancelable: true,";
    ss << "    detail: { paths }";
    ss << "  });";

    // Dispatch event on element under mouse position
    ss << "  if (element) {";
    ss << "    element.dispatchEvent(customEvent);";
    ss << "  } else {";
    ss << "    window.dispatchEvent(customEvent);";
    ss << "  }";

    ss << "})();";

    return ss.str();
  }

  static const auto kWindowsDropListener = R"(
      document.addEventListener('drop', function(e) {
        window.chrome.webview.postMessageWithAdditionalObjects(
          JSON.stringify({
            type: 'deskgui-files-dropped',
            x: e.clientX,
            y: e.clientY
          }),
          e.dataTransfer.files
        );
      }, true);
    )";

}  // namespace deskgui::js
