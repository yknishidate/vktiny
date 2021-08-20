#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct Vertex
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec4 color;
    vec4 tangent;
};

struct MeshBuffers
{
    uint64_t vertices;
    uint64_t indices;
    int baseColorTexture;
};

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices {uvec3 i[]; };
layout(set = 0, binding = 2) uniform sampler2D textureSamplers[];
layout(set = 0, binding = 3) buffer _sceneDesc { MeshBuffers i[]; } sceneDesc;
layout(set = 0, binding = 4) uniform UniformBuffer
{
    mat4 invView;
    mat4 invProj;
    vec3 lightDirection;
} uniformBuffer;

layout(location = 0) rayPayloadInEXT vec3 payLoad;

hitAttributeEXT vec3 attribs;

void main()
{
    MeshBuffers meshResource = sceneDesc.i[gl_InstanceID];
    Vertices vertices = Vertices(meshResource.vertices);
    Indices indices = Indices(meshResource.indices);
    int baseColorTexture = meshResource.baseColorTexture;

    // Indices
    uvec3 index = indices.i[gl_PrimitiveID];

    // Vertex
    Vertex v0 = vertices.v[index.x];
    Vertex v1 = vertices.v[index.y];
    Vertex v2 = vertices.v[index.z];

    // Barycentric coordinates
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 pos = v0.pos * barycentrics.x +
               v1.pos * barycentrics.y +
               v2.pos * barycentrics.z;
    vec2 uv = v0.uv * barycentrics.x +
              v1.uv * barycentrics.y +
              v2.uv * barycentrics.z;
    vec3 normal = v0.normal * barycentrics.x +
                  v1.normal * barycentrics.y +
                  v2.normal * barycentrics.z;

    vec3 baseColor = texture(textureSamplers[baseColorTexture], uv).rgb;

    payLoad = vec3(normal.x, 1, 1);
    // payLoad = pos;
}
