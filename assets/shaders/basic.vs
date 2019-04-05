#version 330 core
layout (location = 0) in vec2 vs_pos;
layout (location = 1) in vec3 vs_color;
layout (location = 2) in vec2 vs_uv;

out vec3 fs_color;
out vec2 fs_uv;

void main()
{
    gl_Position = vec4(vs_pos.x, vs_pos.y, 0, 1.0);
	fs_color = vs_color;
	fs_uv = vs_uv;
}