/**
 * deskgui - A powerful and flexible C++ library to create web-based desktop applications.
 *
 * Copyright (c) 2023 deskgui
 * MIT License
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace deskgui {

  /**
   * Represents a resource, including its scheme, content and type.
   */
  struct Resource {
    std::string scheme;  // The URL scheme of resource (e.g., "static/assets/", "data/js/").
    std::vector<std::uint8_t> content;  // The resource content
    std::string mime;  // The resource mime (e.g., "text/html", "application/javascript", ...).
  };

  using Resources = std::vector<Resource>;

#ifdef COMPILED_RESOURCES_ENABLED
  /**
   * Retrieves the compiled resources with the specified name.
   *
   * @param name The name of the compiled resources.
   * @return The compiled resources matching the specified name.
   */
  Resources getCompiledResources(const std::string& name);
#endif
}  // namespace deskgui
