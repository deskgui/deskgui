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

#include "file.h"

#ifdef WIN32
#include "strings.h"
#endif

namespace deskgui::utils {

  inline std::string createDropEventJS(const std::vector<std::filesystem::path>& paths, double x,
                                       double y) {
    std::stringstream ss;
    ss << "(() => {";

    // Create array of paths
    ss << "  const paths = [";
    for (size_t i = 0; i < paths.size(); ++i) {
      if (i > 0) ss << ",";
#ifdef WIN32
      ss << "'" << escapeBackslashes(paths[i].string()) << "'";
#else
      ss << "'" << paths[i].string() << "'";
#endif
    }
    ss << "];";

    // Get element under mouse position
    ss << "  const element = document.elementFromPoint(" << x << ", " << y << ");";

    // Create custom event with paths data
    ss << "  const customEvent = new CustomEvent('nativedrop', {";
    ss << "    bubbles: true,";
    ss << "    cancelable: true,";
    ss << "    detail: {";
    ss << "      paths: paths,";
    ss << "      x: " << x << ",";
    ss << "      y: " << y;
    ss << "    }";
    ss << "  });";

    // Dispatch event on element under mouse position
    ss << "  if (element) {";
    ss << "    element.dispatchEvent(customEvent);";
    ss << "  }";

    ss << "})();";

    return ss.str();
  }

}  // namespace deskgui::utils