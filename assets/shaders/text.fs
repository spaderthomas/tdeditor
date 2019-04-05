#version 330 core
in vec3 fs_color;
in vec2 fs_uv;

out vec4 out_color;

uniform sampler2D text;

void main() {
	 float sample = texture(text, fs_uv).r;
	 out_color = vec4(fs_color.x, fs_color.y, fs_color.z, 1.0 * sample);
}  