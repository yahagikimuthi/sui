#pragma once

#include <string_view>

namespace sui {
using namespace std::string_view_literals;

inline constexpr auto vertex_source = R"(
#version 450

// 頂点バッファのレイアウトに対応するインデックス
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// フラグメントシェーダーへ渡す色
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
)"sv;

inline constexpr auto frag_source = R"(
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)"sv;
}  // namespace sui