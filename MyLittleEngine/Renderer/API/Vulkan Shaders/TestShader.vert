#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vNormal;
layout(location = 2) in vec4 vColor;
layout(location = 3) in vec2 vTexCoord;

layout(row_major, push_constant) uniform PushConstants
{
	mat4 projection;
	mat4 view;
} pushConstants;

layout(location = 0) out vec4 fColor;

void main()
{
	gl_Position = pushConstants.projection * pushConstants.view * vPosition;

	fColor = vColor;
}