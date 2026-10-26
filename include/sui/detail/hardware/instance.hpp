#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <hgn/app_instance_info.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical_device.hpp>
#include <kgm/kgm.hpp>

#include "sui/detail/others.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class instance final {
    class native_instance final {
      public:
        explicit native_instance(hgn::instance& native) noexcept : native_{native} {}

        native_instance(const native_instance&) noexcept                    = delete;
        auto operator=(const native_instance&) noexcept -> native_instance& = delete;

        native_instance(native_instance&& other) noexcept
            : native_{std::exchange(other.native_, std::nullopt)} {}

        auto operator=(native_instance&& other) noexcept -> native_instance& {
            if (this == &other) return *this;

            destroy();

            native_ = std::exchange(other.native_, std::nullopt);
            return *this;
        }

        ~native_instance() noexcept { destroy(); }

        [[nodiscard]] auto get() noexcept -> hgn::instance& { return *native_; }

        [[nodiscard]] operator hgn::instance&() noexcept { return get(); }

      private:
        void destroy() noexcept {
            if (native_) hgn::destroy_instance(get());
        }

        std::optional<hgn::instance&> native_;
    };

  public:
    [[nodiscard]] static auto try_make() noexcept -> std::expected<instance, error> {
        const auto app_info = hgn::application_info{}
                                  .application_name("sui")
                                  .application_version(1, 0, 0)
                                  .engine_name("No Engine")
                                  .engine_version(1, 0, 0)
                                  .engine_version(1, 3, 0)
                                  .api_version(1, 3, 0);

        auto info =
            hgn::instance_create_info{}.app_info(app_info).extensions(required_extensions());

#ifndef NDEBUG
        info.layer("VK_LAYER_KHRONOS_validation");
#endif
        auto native = hgn::try_make_instance(info);
        if (not native) return make_error(runtime_error, "Failed to make hagane instance\n");

        return instance{*native};
    }

    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;
    instance(instance&&) noexcept                         = default;
    auto operator=(instance&&) noexcept -> instance&      = default;
    ~instance() noexcept                                  = default;

    [[nodiscard]] auto try_enumerate_physical_devices() noexcept
        -> std::optional<std::vector<std::reference_wrapper<hgn::physical_device>>> {
        auto res = hgn::try_enumerate_physical_devices(native_);
        return to_optional(res);
    }

    [[nodiscard]] operator hgn::instance&() noexcept { return native_; }

  private:
    explicit instance(hgn::instance& native) noexcept : native_{native} {}

    [[nodiscard]] static auto required_extensions() noexcept -> std::vector<std::string_view> {
        auto extensions = kgm::get_required_instance_extensions();

#ifndef NDEBUG
        extensions.emplace_back(hgn::ext_debug_utils_extension_name);
#endif
        return extensions;
    }

    native_instance native_;
};
}  // namespace sui::detail