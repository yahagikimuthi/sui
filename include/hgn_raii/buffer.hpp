#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/declaration.hpp>
#include <hgn/device_memory.hpp>
#include <hgn/enums.hpp>

namespace hgn::raii {
class device_memory final {
  public:
    [[nodiscard]] static auto try_allocate(
        const hgn::memory_allocate_info& info, hgn::device& dev
    ) noexcept -> std::expected<device_memory, hgn::result> {
        auto res = hgn::try_allocate_memory(dev, info);
        if (not res) return std::unexpected{res.error()};

        return device_memory{*res, dev};
    }

    device_memory(const device_memory&) noexcept                    = delete;
    auto operator=(const device_memory&) noexcept -> device_memory& = delete;

    device_memory(device_memory&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(device_memory&& other) noexcept -> device_memory& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~device_memory() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::device_memory& { return *native_; }

    [[nodiscard]] operator hgn::device_memory&() noexcept { return get(); }

  private:
    explicit device_memory(hgn::device_memory& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::free_memory(dev_, get());
    }

    std::optional<hgn::device_memory&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_allocate_memory(
    const hgn::memory_allocate_info& info, hgn::device& dev
) noexcept -> std::expected<device_memory, hgn::result> {
    return device_memory::try_allocate(info, dev);
}

class buffer final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::buffer_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<buffer, hgn::result> {
        auto res = hgn::try_make_buffer(dev, info);
        if (not res) return std::unexpected{res.error()};

        return buffer{*res, dev};
    }

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~buffer() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::buffer& { return *native_; }

    [[nodiscard]] operator hgn::buffer&() noexcept { return get(); }

  private:
    explicit buffer(hgn::buffer& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_buffer(dev_, get());
    }

    std::optional<hgn::buffer&>         native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_buffer(
    const hgn::buffer_create_info& info, hgn::device& dev
) noexcept -> std::expected<buffer, hgn::result> {
    return buffer::try_make(info, dev);
}
}  // namespace hgn::raii