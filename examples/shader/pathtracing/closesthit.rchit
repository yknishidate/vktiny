#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#include "globals.glsl"
#include "random.glsl"

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
layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 2) uniform sampler2D textureSamplers[];
layout(set = 0, binding = 3) buffer _sceneDesc { MeshBuffers i[]; } sceneDesc;
layout(set = 0, binding = 4) uniform UniformBuffer
{
    mat4 invView;
    mat4 invProj;
    mat4 transform;
    vec3 lightDirection;
} uniformBuffer;

layout(location = 0) rayPayloadInEXT HitPayload payLoad;
layout(location = 1) rayPayloadInEXT ShadowPayload shadowPayLoad;

hitAttributeEXT vec3 attribs;

vec3 calcNormal(vec3 pos0, vec3 pos1, vec3 pos2)
{
    vec3 e0 = pos1 - pos0;
    vec3 e1 = pos2 - pos0;
    vec3 normal = cross(e0, e1);
    return normalize(normal);
}

vec3 createShadowRay()
{
    vec2 randVal = vec2(rand(payLoad.seed), rand(payLoad.seed));
    vec3 lightPos = 50.0 * uniformBuffer.lightDirection;
    lightPos += sampleSphere(randVal);
    vec3 lightDir = normalize(lightPos);
    return lightDir;
}

bool inShadow(vec3 pos, vec3 dir)
{
    shadowPayLoad.hit = true;
    traceRayEXT(
        topLevelAS,
        gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
        0xff, // cullMask
        0,    // sbtRecordOffset
        0,    // sbtRecordStride
        1,    // missIndex
        pos,
        0.01,
        dir,
        1000.0,
        1     // payloadLocation
    );
    return shadowPayLoad.hit; 
}

vec3 removeGamma(in vec3 color)
{
    return pow(color, vec3(2.2));
}

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
    vec3 normal = calcNormal(v0.pos, v1.pos, v2.pos);
    pos = (uniformBuffer.transform * vec4(pos, 1.0)).xyz;
    normal.y *= -1;

    vec3 baseColor = texture(textureSamplers[baseColorTexture], uv).rgb;
    baseColor = removeGamma(baseColor);

    vec2 randVal = vec2(rand(payLoad.seed), rand(payLoad.seed));
    payLoad.nextDir = sampleDirection(randVal, normal);

    vec3 shadowDir = createShadowRay();
    if(!inShadow(pos, shadowDir)){
        vec3 lightIntensity = vec3(5.0);
        payLoad.color += payLoad.weight * baseColor * lightIntensity * abs(dot(normal, shadowDir));
    }
    payLoad.hit = true;
    payLoad.pos = pos;
    payLoad.normal = normal;
    payLoad.weight *= baseColor * abs(dot(normal, payLoad.nextDir));
}
