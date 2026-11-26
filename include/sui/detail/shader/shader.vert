#version 450

layout(location = 0) in vec3 a_position;  // x, y, z
layout(location = 1) in vec4 a_color;     // r, g, b, a

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec3 v_posWS;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
}
ubo;

layout(push_constant, std430) uniform PushConstants {
    mat4 model;
}
push;

void main() {
    vec4 worldPos = push.model * vec4(a_position, 1.0);
    v_posWS       = worldPos.xyz;
    gl_Position   = ubo.proj * ubo.view * worldPos;
    v_color       = a_color;
}