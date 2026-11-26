#version 450

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec3 v_posWS;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 coordPosWS = v_posWS.xz;

    vec2  grid     = abs(fract(coordPosWS - 0.5) - 0.5) / fwidth(coordPosWS);
    float line     = min(grid.x, grid.y);
    float lineMask = 1.0 - min(line, 1.0);

    vec4 lineColor = vec4(0.75, 0.78, 0.85, 1.0);
    outColor       = mix(v_color, lineColor, lineMask);
}