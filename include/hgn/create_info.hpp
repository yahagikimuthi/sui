#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include <string>
#include <vector>

#include "hgn/application_info.hpp"

namespace hgn {
class create_info final {
  public:
    explicit create_info(const application_info& app_info) noexcept : app_info_{app_info} {}

    auto layers(const std::span<const char*> layer_vec) noexcept -> create_info& {
        layers_ = std::vector<std::string>(layer_vec.begin(), layer_vec.end());
        return *this;
    }

    auto extensions(const std::span<const char*> extention_vec) noexcept -> create_info& {
        extensions_ = std::vector<std::string>(extention_vec.begin(), extention_vec.end());
        return *this;
    }

    [[nodiscard]] auto app_info() const noexcept -> const application_info& { return app_info_; }
    [[nodiscard]] auto layers() const noexcept -> std::span<const std::string> { return layers_; }
    [[nodiscard]] auto extensions() const noexcept -> std::span<const std::string> {
        return extensions_;
    }

  private:
    const application_info&  app_info_;
    std::vector<std::string> layers_;
    std::vector<std::string> extensions_;
};
}  // namespace hgn