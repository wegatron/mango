#version 450

#define HAS_BASE_COLOR_TEXTURE 1

layout(set=1, binding = 0) uniform BasicMaterial
{
    vec4 base_color;
}pbr_mat;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set=1, binding = 1) uniform sampler2D base_color_tex;
#endif

layout(location=0) in vec2 uv;
layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

void main(void)
{
    #ifdef HAS_BASE_COLOR_TEXTURE
    frag_color = texture(base_color_texture, uv);
    #else
    frag_color = pbr_mat.base_color;
    #endif
}