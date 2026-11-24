#version 450

layout(location = 0) in vec3 inPosition;  // x, y, z
layout(location = 1) in vec4 inColor;     // r, g, b, a

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragWorldPos;

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
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    gl_Position   = ubo.proj * ubo.view * worldPos;
    fragColor     = inColor;
}