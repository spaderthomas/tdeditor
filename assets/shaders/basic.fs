#version 330 core
out vec4 color;

in vec3 fs_color;
in vec2 fs_uv;

void main()
{
    color = vec4(fs_color, 1.0);
} 