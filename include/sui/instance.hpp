#pragma once

#include <expected>
#include <hgn/application_info.hpp>
#include <hgn/instance.hpp>
#include <hgn/others.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/types.hpp>
#include <kgm/kgm.hpp>
#include <utility>
#include <vector>

#include "sui/error.hpp"

namespace sui {
class instance final {
  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<instance, error> {
        auto app_info = hgn::application_info{};
        hgn::application_info_setter{app_info}
            .application_name("Hello Triangle")
            .application_version(1, 0, 0)
            .engine_name("No Engine")
            .engine_name("Engine Version")
            .engine_version(1, 0, 0)
            .api_version(1, 3, 0);

        auto create_info = hgn::instance_create_info{};
        hgn::instance_create_info_setter{create_info}
            .app_info(app_info)
            .layers(
                enable_validation_layers ? std::vector<const char*>{"VK_LAYER_KHRONOS_validation"}
                                         : std::span<const char* const>{}
            )
            .extensions(get_required_extensions());

        auto native_instance = hgn::try_make_instance(create_info);
        if (not native_instance) return make_error(runtime_error, "Failed to create instance.\n");

        return instance{*native_instance};
    }

    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : value_{std::exchange(other.value_, hgn::null_handle)} {}
    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        value_ = std::exchange(other.value_, hgn::null_handle);
        return *this;
    }

    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto try_make_device() noexcept
        -> std::expected<std::vector<hgn::physical_device>, error> {
        if (value_ == hgn::null_handle) return make_error(logic_error, "Invalid instance.");
        const auto device = hgn::try_enumerate_physical_devices(value_);
        if (not device) return make_error(runtime_error, "Failed to find GPUs with Vulkan support");
        return *device;
    }

  private:
    explicit instance(hgn::instance value) noexcept : value_{value} {}

    [[nodiscard]] static auto get_required_extensions() noexcept -> std::vector<const char*> {
        auto extensions = kgm::get_required_instance_extensions();

        if (enable_validation_layers) {
            extensions.push_back(hgn::ext_debug_utils_extension_name);
        }

        return extensions;
    }

    void destroy() noexcept {
        if (value_ == hgn::null_handle) return;
        hgn::destroy_instance(value_);
    }

    hgn::instance value_{hgn::null_handle};

#ifndef NDEBUG
    static constexpr auto enable_validation_layers = true;
#else
    static constexpr auto enable_validation_layers = false;
#endif
};
}  // namespace sui