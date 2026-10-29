#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn2/application.hpp"
#include "hgn2/enums.hpp"

namespace hgn {
class instance final {
  public:
    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;
    instance(instance&& other) noexcept;
    auto operator=(instance&& other) noexcept -> instance&;
    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkInstance_T& { return *raw_; }

    class builder final {
      public:
        explicit builder() noexcept = default;

        auto app(const application& app) noexcept -> builder&;

        auto layer(const std::string_view layer) noexcept -> builder&;

        auto extensions(const std::span<const std::string_view> exts) noexcept -> builder&;

        [[nodiscard]] auto build() noexcept -> std::expected<instance, result>;

      private:
        std::optional<const application&> app_;
        std::string                       layer_;
        std::vector<std::string>          extensions_;
    };

  private:
    friend class instance::builder;

    explicit instance(VkInstance_T& raw) noexcept : raw_{raw} {}

    void destroy() noexcept;

    std::optional<VkInstance_T&> raw_;
};
}  // namespace hgn

#define HGN_DETAIL_INSTANCE_IMPL_HPP_
#include "hgn2/detail/instance.impl.hpp"
#undef HGN_DETAIL_INSTANCE_IMPL_HPP_