#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

namespace sui {
[[nodiscard]] inline auto read_file(const std::filesystem::path& path) noexcept
    -> std::optional<std::vector<char>> {
    auto file = std::ifstream{path, std::ios::ate | std::ios::binary};

    if (not file.is_open()) return std::nullopt;

    const auto size   = static_cast<std::int64_t>(file.tellg());
    auto       buffer = std::vector<char>(static_cast<std::size_t>(size));

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}
}  // namespace sui