#version 330 core
in vec2 uv;
out vec4 frag;
uniform sampler2D uTex;
void main() {
    float a = texture(uTex, uv).r;
    frag = vec4(1,1,1,a);
}
