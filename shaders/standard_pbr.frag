#version 450
#extension GL_EXT_scalar_block_layout : require

struct LightT
{
  int light_type;
  float angle;
  float blend;
  float falloff;

  vec3 position;
  vec3 direction;
  vec3 color;
  float intensity; // lux for directional light or cd for other lights
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

layout(std430, set=MATERIAL_SET_INDEX, binding = 0) uniform BasicMaterial
{
    vec4 base_color;
    float metallic;
    float roughness;
    float specular;
}pbr_mat;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 1) uniform sampler2D base_color_tex;
#endif

#ifdef HAS_METALLIC_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 2) uniform sampler2D metallic_tex;
#endif

#ifdef HAS_ROUGHNESS_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 3) uniform sampler2D roughness_tex;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 4) uniform sampler2D metallic_roughness_tex;
#endif

#ifdef HAS_SPECULAR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 5) uniform sampler2D specular_tex;
#endif

#ifdef HAS_NORMAL_MAP
layout(set=MATERIAL_SET_INDEX, binding = 6) uniform sampler2D normal_map;
#endif

layout(location=0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 pos;
layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

const float PI = 3.14159265359f;

struct PixelShadingParam
{
  float NdotL;
  float NdotV;
  float LdotH;
  float NdotH;

  // material
  vec4 base_color;
  float metallic;
  float roughness;
  float specular;

  // light
  vec3 intensity;
};

vec3 F_Schlick (const vec3 f0 , const vec3 f90 , float u)
{
  return f0 + (f90 - f0) * pow (1.0f - u, 5.f);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
  // do energy conservation
  float energyBias = 0.5f * roughness;
  float energyFactor = mix(1.0f, 1.0f / 1.51f, roughness);
  vec3 fd90 = vec3(energyBias + 2.0f * LdotH*LdotH*roughness);
  vec3 f0 = vec3(1.0f);
  float lightScatter = F_Schlick(f0, fd90, NdotL).r;
  float viewScatter = F_Schlick(f0, fd90, NdotV).r;
  return lightScatter * viewScatter * energyFactor;
}

float D_GGX(const float NdotH, const float roughness)
{
  float sq_r = roughness * roughness;
  float f = NdotH*NdotH*(sq_r - 1.0f) + 1.0f;
  return sq_r / (PI * f * f);
}

float V_SmithGGXCorrelated(const float NdotL, const float NdotV, const float roughness)
{
  float sq_r = roughness * roughness;
  float lambda_GGXV = NdotL * sqrt(NdotV * NdotV *(1-sq_r) + sq_r);
  float lambda_GGXL = NdotV * sqrt(NdotL * NdotL *(1-sq_r) + sq_r);
  return 0.5f/(lambda_GGXL + lambda_GGXV);
}

vec3 surfaceShading(const PixelShadingParam pixel)
{
  // diffuse
  vec3 diffuse_color = pixel.base_color.rgb * (1.0f - pixel.metallic);
  float Fd = Fr_DisneyDiffuse(pixel.NdotV, pixel.NdotL, pixel.LdotH, pixel.roughness); // diffuse
  
  // reflection: D, F, G
  vec3 f0 = mix(vec3(0.16f * pixel.specular * pixel.specular), pixel.base_color.rgb, pixel.metallic);
  vec3 f90 = clamp(50.0f * f0, 0.0f, 1.0f);
  vec3 F = F_Schlick(f0, f90, pixel.LdotH);
  float D = D_GGX(pixel.NdotH, pixel.roughness);
  float Vis = V_SmithGGXCorrelated(pixel.NdotL, pixel.NdotV, pixel.roughness);
  vec3 Fr = D * Vis * F;
  
  return pixel.intensity * ((vec3(1.0f) - F) * Fd * diffuse_color + Fr);
}

void main(void)
{
  PixelShadingParam pixel;
  
  #ifdef HAS_BASE_COLOR_TEXTURE
  pixel.base_color = texture(base_color_tex, uv);
  #else
  pixel.base_color = pbr_mat.base_color;
  #endif

  pixel.metallic = pbr_mat.metallic;
  pixel.roughness = pbr_mat.roughness;
  #ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
  vec2 mr = texture(metallic_roughness_tex, uv).rg;
  pixel.metallic = mr[0];
  pixel.roughness = mr[1];
  #endif
  #ifdef HAS_METALLIC_TEXTURE
  pixel.metallic = texture(metallic_tex, uv).r;
  #endif
  #ifdef HAS_ROUGHNESS_TEXTURE
  pixel.roughness = texture(roughness_tex, uv).r;
  #endif

  #ifdef HAS_SPECULAR_TEXTURE
  pixel.specular = texture(specular_tex, uv).r;
  #else
  pixel.specular = pbr_mat.specular;
  #endif

  vec4 result_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  vec3 V = -global_uniform.view[3].xyz - pos;
  pixel.NdotV = max(0.0f, dot(normal, V));
  for(int index=0; index<global_uniform.light_count; ++index)
  {
    if(global_uniform.lights[index].light_type == DIRECTIONAL)
    {
      pixel.NdotL = max(0.0f, dot(normal, -global_uniform.lights[index].direction));
      vec3 H = normalize(-global_uniform.lights[index].direction + V);
      pixel.NdotH = max(0.0f, dot(normal, H));
      pixel.LdotH = max(0.0f, dot(-global_uniform.lights[index].direction, H));
      pixel.intensity = global_uniform.lights[index].color;
    }
    result_color.xyz += surfaceShading(pixel);
  }

  frag_color = result_color;
}