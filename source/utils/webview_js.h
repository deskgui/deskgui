/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */
 
#pragma once

#include "file.h"

#include <string>
#include <vector>
#include <sstream>

namespace utils {

inline std::string createDropEventJS(const std::vector<std::string>& paths, double x, double y) {
    std::stringstream ss;
    ss << "(() => {";
    ss << "  const dataTransfer = new DataTransfer();";
    
    // Create File objects directly for each path
    for (const auto& path : paths) {
        const auto& data = readFileData(path);
        const std::string& mimeType = getMimeType(path);
        const std::string& fileName = std::filesystem::path(path).filename().string();
        
        ss << "  const blob" << &path << " = new Blob([new Uint8Array([";
        for (size_t j = 0; j < data.size(); ++j) {
            if (j > 0) ss << ",";
            ss << static_cast<int>(data[j]);
        }
        ss << "])], { type: '" << mimeType << "' });";
        
        ss << "  const file" << &path << " = new File([blob" << &path << "], '" << fileName << "', {";
        ss << "    type: '" << mimeType << "',";
        ss << "    lastModified: new Date().getTime()";
        ss << "  });";
        
        ss << "  dataTransfer.items.add(file" << &path << ");";
    }
    
    ss << "  const dropEvent = new DragEvent('drop', {";
    ss << "    bubbles: true,";
    ss << "    cancelable: true,";
    ss << "    dataTransfer: dataTransfer,";
    ss << "    clientX: " << x << ",";
    ss << "    clientY: " << y;
    ss << "  });";
    
    ss << "  document.elementFromPoint(" << x << ", " << y << ")?.dispatchEvent(dropEvent);";
    ss << "})();";
    
    return ss.str();
}

} // namespace utils 