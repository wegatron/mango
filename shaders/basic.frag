#version 450

layout(set=2, binding = 0) uniform BasicMaterial
{
    vec4 base_color;
}pbr_mat;

layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

void main(void)
{
    frag_color = pbr_mat.base_color;
}