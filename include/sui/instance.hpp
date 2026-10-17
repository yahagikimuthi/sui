#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <hgn/app_instance_info.hpp>
#include <hgn/instance.hpp>
#include <kgm/kgm.hpp>

#include "hgn/physical_device.hpp"
#include "sui/error.hpp"

namespace sui {
class instance final {
  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<instance, error> {
        auto app_info = hgn::application_info{};
        app_info.application_name("Sui")
            .application_version(1, 0, 0)
            .engine_name("No Engine")
            .engine_version(1, 0, 0)
            .engine_version(1, 3, 0)
            .api_version(1, 3, 0);

        auto info = hgn::instance_create_info{};
        info.app_info(app_info).extensions(required_extensions());

#ifndef NDEBUG
        info.layers({"VK_LAYER_KHRONOS_validation"});
#endif
        auto ins = hgn::try_make_instance(info);
        if (not ins) return make_error(runtime_error, "Failed to make hagane instance\n");

        return instance{ins->get()};
    }

    instance(const instance&) noexcept = delete;

    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);

        return *this;
    }

    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto try_enumerate_physical_devices() noexcept
        -> std::optional<std::vector<std::reference_wrapper<hgn::physical_device>>> {
        if (not native_) return std::nullopt;
        auto res = hgn::try_enumerate_physical_devices(*native_);
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto native() noexcept -> hgn::instance& { return *native_; }

  private:
    explicit instance(hgn::instance& native) : native_{native} {}

    [[nodiscard]] static auto required_extensions() noexcept -> std::vector<std::string_view> {
        auto extensions = kgm::get_required_instance_extensions();

#ifndef NDEBUG
        extensions.emplace_back(hgn::ext_debug_utils_extension_name);
#endif
        return extensions;
    }

    void destroy() noexcept {
        if (native_) hgn::destroy_instance(*native_);
    }

    std::optional<hgn::instance&> native_;
};
}  // namespace sui