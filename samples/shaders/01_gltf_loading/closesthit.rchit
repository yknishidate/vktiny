#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Vertex
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec4 color;
    vec4 joint0;
    vec4 weight0;
    vec4 tangent;
};

layout(binding = 2, set = 0)
buffer Vertices
{
    float v[];
} vertices;

layout(binding = 3, set = 0)
buffer Indices
{
    uint i[];
} indices;

Vertex unpack(uint index)
{
    uint vertexSize = 24;
    uint offset = index * vertexSize;

    Vertex v;
    v.pos    = vec3(vertices.v[offset + 0], vertices.v[offset + 1], vertices.v[offset + 2]);
    v.normal = vec3(vertices.v[offset + 3], vertices.v[offset + 4], vertices.v[offset + 5]);
    v.uv     = vec2(vertices.v[offset + 6], vertices.v[offset + 7]);
    v.color  = vec4(vertices.v[offset + 8], vertices.v[offset + 9], vertices.v[offset + 10], 1.0);

	return v;
}

layout(location = 0) rayPayloadInEXT vec3 payLoad;
hitAttributeEXT vec3 attribs;

void main()
{
	Vertex v0 = unpack(indices.i[3 * gl_PrimitiveID + 0]);
	Vertex v1 = unpack(indices.i[3 * gl_PrimitiveID + 1]);
	Vertex v2 = unpack(indices.i[3 * gl_PrimitiveID + 2]);

    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
	vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;

    payLoad = normal;
}
