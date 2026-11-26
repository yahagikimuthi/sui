#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_color;

layout(location = 0) out vec3 v_posWS;
layout(location = 1) out vec3 v_normalWS;
layout(location = 2) out vec4 v_color;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
}
cam;

layout(push_constant, std430) uniform PushConstants {
    mat4 model;
}
push;

void main() {
    vec4 posWS  = push.model * vec4(a_position, 1.0);
    v_posWS     = posWS.xyz;
    gl_Position = cam.proj * cam.view * posWS;

    // 以下の式は法線ベクトルと接線ベクトルの内積に基づいて計算した結果、
    // 法線ベクトルに書けるべきmodel行列が、push.modelの逆行列の転置である
    // 数学的事実に基づいている。しかし、計算量もあるため、実装は留保
    // mat3 normalMat = transpose(inverse(mat3(push.model)));
    // v_normalWS     = normalMat * a_normal;

    // model行列には次の役割がある
    // 左上の3x3部分は拡大縮小
    // 右側の列は平行移動
    // 下側は射影や同時座標系(通常0,0,0,1)
    // ベクトルであることが重要なため、右側の場所情報は不要なため、mat3にキャストする
    v_normalWS = mat3(push.model) * a_normal;

    v_color = a_color;
}