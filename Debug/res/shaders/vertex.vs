#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 transform;
uniform mat4 frame;

void main()
{
	texCoord = (frame * vec4(aTexCoord, 0.0, 1.0)).xy;
    gl_Position = transform * vec4(aPos, 1.0);
}