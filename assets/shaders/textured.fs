#version 330 core
out vec4 FragColor;

in vec3 fs_color;
in vec2 fs_uv;

uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, fs_uv);
} 