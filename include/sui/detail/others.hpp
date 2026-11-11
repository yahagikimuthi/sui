#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

namespace sui::detail {
class debugger final {
  public:
    static void set_fn(std::function<void(std::string_view)>&& fn) noexcept { fn_ = std::move(fn); }

    static void set_default() noexcept { is_default_ = true; }

    static void reset() noexcept { fn_ = std::function<void(std::string_view)>{}; }

    static void notify(const std::string_view message) noexcept {
        if (fn_)
            fn_(message);
        else if (is_default_)
            std::cerr << message;
    }

  private:
    static inline auto           fn_         = std::function<void(std::string_view)>{};  // NOLINT
    static constinit inline auto is_default_ = false;
};

[[nodiscard]] inline auto fail(const std::string_view message) noexcept -> bool {
    debugger::notify(message);
    return false;
}

[[nodiscard]] inline auto read_file(const std::filesystem::path& path
) noexcept -> std::optional<std::vector<char>> {
    auto file = std::ifstream{path, std::ios::ate | std::ios::binary};

    if (not file.is_open()) return std::nullopt;

    const auto size   = static_cast<std::int64_t>(file.tellg());
    auto       buffer = std::vector<char>(static_cast<std::size_t>(size));

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

struct uniform_buffer_object final {
    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 view  = glm::mat4(1.f);
    glm::mat4 proj  = glm::mat4(1.f);
};
}  // namespace sui::detail