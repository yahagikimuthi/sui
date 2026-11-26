#version 450

layout(location = 0) in vec3 v_posWS;
layout(location = 1) in vec3 v_normalWS;
layout(location = 2) in vec4 v_color;

layout(location = 0) out vec4 outColor;

float calcLineMask(vec3 posWS) {
    vec2  coordPos = posWS.xz;
    vec2  grid     = abs(fract(coordPos - 0.5) - 0.5) / fwidth(coordPos);
    float line     = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

vec4 calcLitColor(vec3 normalWS) {
    vec3 l = normalize(vec3(0.5, 1.0, 0.3));

    float diff    = max(dot(normalWS, l), 0.0);
    float ambient = 0.15;

    vec3 baseColor = vec3(0.7, 0.75, 0.85);
    vec3 litColor  = baseColor * (ambient + diff + 0.85);
    return vec4(litColor, 1.0);
}

void main() {
    vec4 litColor = calcLitColor(normalize(v_normalWS));

    vec4 lineColor = vec4(0.75, 0.78, 0.85, 1.0);
    outColor       = mix(litColor, lineColor, calcLineMask(v_posWS));
}