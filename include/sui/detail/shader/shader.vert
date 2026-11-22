#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
}
ubo;

layout(location = 0) in vec3 inPosition;  // x, y, z
layout(location = 1) in vec4 inColor;     // r, g, b, a

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragWorldPos;

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    gl_Position   = ubo.proj * ubo.view * worldPos;
    fragColor     = inColor;
}