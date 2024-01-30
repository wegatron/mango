#version 450
#extension GL_EXT_scalar_block_layout : require

// vertex data binding = 0
layout(location=0) in vec4 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(location=0) out vec2 out_uv;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_pos;

struct LightT
{
  int light_type;
  float angle;
  float blend;
  float falloff;//16

  vec3 position;
  vec3 direction;
  vec3 color; // 64
  float intensity; // 64
};

layout(std430, set=GLOBAL_SET_INDEX, binding = 0) uniform GlobalUniform
{
    // camera
    float ev100; // 16
    mat4 view; // 80
    mat4 proj; // 144    

    // lights
    LightT lights[MAX_LIGHTS_COUNT]; // 144 + 64 * MAX_LIGHTS_COUNT
    int light_count;  // 144 + 64 * MAX_LIGHTS_COUNT + 16
} global_uniform;

layout(set=PER_OBJECT_SET_INDEX, binding = 0) uniform MeshUniform
{
    mat4 model;
} mesh_uniform;

void main(void)
{
    out_uv = uv;
    out_normal = mat3x3(mesh_uniform.model) * normal;
    vec4 gpos = mesh_uniform.model * vpos;
    out_pos = gpos.xyz;
    gl_Position = global_uniform.proj * global_uniform.view * gpos;
}