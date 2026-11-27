#version 450

layout(location = 0) in vec3 v_posWS;
layout(location = 1) in vec3 v_normalWS;
layout(location = 2) in vec4 v_color;

layout(location = 0) out vec4 outColor;

vec4 calcLightedColor(vec4 baseColor, vec3 normalWS) {
    vec3 lightColor = vec3(0.7, 0.75, 0.85);
    vec3 lightDir   = normalize(vec3(0.5, 1.0, 0.3));

    float diff    = max(dot(normalWS, lightDir), 0.0);
    vec3  diffuse = diff * lightColor;
    vec3  ambient = 0.15 * lightColor;

    vec3 litRgb = (ambient + diffuse) * baseColor.rgb;
    return vec4(litRgb, baseColor.a);
}

vec4 calcLinedColor(vec4 baseColor, vec3 posWS) {
    vec2  coordPos = posWS.xz;
    vec2  grid     = abs(fract(coordPos - 0.5) - 0.5) / fwidth(coordPos);
    float line     = min(grid.x, grid.y);
    float mask     = 1.0 - min(line, 1.0);

    vec4 lineColor = vec4(0.75, 0.78, 0.85, 1.0);
    return mix(baseColor, lineColor, mask);
}

void main() {
    vec4 lightedColor = calcLightedColor(v_color, normalize(v_normalWS));
    outColor = abs(v_posWS.y) < 0.0001 ? calcLinedColor(lightedColor, v_posWS) : lightedColor;
}