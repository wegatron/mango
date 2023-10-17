#version 450

// vertex data binding = 0
layout(location=0) in vec4 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(set=3, binding = 0) uniform MeshUniform
{
    mat4 model;
} mesh_uniform;

void main(void)
{
    gl_Position = mesh_uniform.model * vpos;
}