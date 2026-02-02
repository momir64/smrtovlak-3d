#version 330 core
layout(location=0) in vec4 aPosUV;
out vec2 uv;
void main() {
    uv = aPosUV.zw;
    vec2 p = aPosUV.xy * 2.0 - 1.0;
    gl_Position = vec4(p, 0, 1);
}
