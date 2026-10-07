#pragma once

#include <expected>
#include <hgn/application_info.hpp>
#include <hgn/device.hpp>
#include <hgn/instance.hpp>
#include <hgn/others.hpp>
#include <hgn/types.hpp>
#include <kgm/kgm.hpp>
#include <optional>
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
            .engine_version(1, 0, 0)
            .api_version(1, 3, 0);

        static const auto validation_layers =
            std::vector<const char*>{"VK_LAYER_KHRONOS_validation"};

        auto create_info          = hgn::instance_create_info{};
        auto extensions           = get_required_extensions();
        auto instance_info        = hgn::instance_create_info{};
        auto instance_info_setter = hgn::instance_create_info_setter{instance_info};
        instance_info_setter.app_info(app_info).extensions(extensions);

#ifndef NDEBUG
        instance_info_setter.layers(validation_layers);
#endif

        auto native_instance = hgn::try_make_instance(create_info);
        if (not native_instance) return make_error(runtime_error, "Failed to create instance.\n");

        return instance{*native_instance};
    }

    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}
    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        return *this;
    }

    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto try_make_devices() noexcept
        -> std::expected<std::vector<hgn::physical_device*>, error> {
        if (not native_) return make_error(logic_error, "Invalid instance.");
        const auto device = hgn::try_enumerate_physical_devices(*native_);
        if (not device) return make_error(runtime_error, "Failed to find GPUs with Vulkan support");
        return *device;
    }

    [[nodiscard]] auto native() noexcept -> hgn::instance& {
        assert(native_);
        return *native_;
    }

  private:
    explicit instance(hgn::instance& native) noexcept : native_{native} {}

    [[nodiscard]] static auto get_required_extensions() noexcept -> std::vector<const char*> {
        auto extensions = kgm::get_required_instance_extensions();

#ifndef NDEBUG
        extensions.push_back(hgn::ext_debug_utils_extension_name);
#endif
        return extensions;
    }

    void destroy() noexcept {
        if (native_) hgn::destroy_instance(*native_);
    }

    std::optional<hgn::instance&> native_{std::nullopt};
};
}  // namespace sui