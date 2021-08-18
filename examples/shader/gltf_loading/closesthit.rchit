#version 460
#extension GL_EXT_ray_tracing : enable
// #extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 2, set = 0) uniform sampler2D textureSamplers[];

layout(location = 0) rayPayloadInEXT vec3 payLoad;

hitAttributeEXT vec3 attribs;

// struct Vertex
// {
//     vec3 pos;       // 0
//     vec3 normal;    // 3
//     vec2 uv;        // 6
//     vec4 color;     // 8
//     vec4 tangent;   // 12
// };

// Vertex unpack(uint meshIndex, uint index)
// {
//     uint vertexSize = 14;
//     uint offset = index * vertexSize;
//     Vertex v;
//     v.pos       = vec3(vertices[meshIndex].v[offset +  0], vertices[meshIndex].v[offset +  1], vertices[meshIndex].v[offset + 2]);
//     v.normal    = vec3(vertices[meshIndex].v[offset +  3], vertices[meshIndex].v[offset +  4], vertices[meshIndex].v[offset + 5]);
//     v.uv        = vec2(vertices[meshIndex].v[offset +  6], vertices[meshIndex].v[offset +  7]);
//     v.tangent   = vec3(vertices[meshIndex].v[offset +  8], vertices[meshIndex].v[offset +  9], vertices[meshIndex].v[offset + 10]);
//     v.biTangent = vec3(vertices[meshIndex].v[offset + 11], vertices[meshIndex].v[offset + 12], vertices[meshIndex].v[offset + 13]);
//     return v;
// }

void main()
{
    // int meshIndex = instanceData[gl_InstanceID].meshIndex;

    // Vertex v0 = unpack(meshIndex, indices[meshIndex].i[3 * gl_PrimitiveID + 0]);
    // Vertex v1 = unpack(meshIndex, indices[meshIndex].i[3 * gl_PrimitiveID + 1]);
    // Vertex v2 = unpack(meshIndex, indices[meshIndex].i[3 * gl_PrimitiveID + 2]);

    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    // vec3 pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    // vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;
    // vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    // vec3 color = vec3(1.0);
    payLoad = texture(textureSamplers[0], uv).rgb;;
}
