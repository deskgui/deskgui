/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace deskgui {

  // Represents a resource, including its scheme, resource, and type.
  struct Resource {
    std::string scheme;        // The URL scheme of resource (e.g., "static/assets/", "data/js/").
    const std::uint8_t* data;  // The resource content
    std::size_t size;    // Size of the resource content
    std::string mime;  // The resource mime (e.g., "text/html", "application/javascript", ...).
  };

  using Resources = std::vector<Resource>;

#ifdef COMPILED_RESOURCES_ENABLED
  Resources getCompiledResources(const std::string& name);
#endif
}  // namespace deskgui
